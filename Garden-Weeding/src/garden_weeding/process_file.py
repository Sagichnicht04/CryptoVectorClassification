import os
import numpy as np
import json
import torch
import torch.amp
from transformers import AutoTokenizer, AutoModel, BitsAndBytesConfig
from peft import LoraConfig, get_peft_model, prepare_model_for_kbit_training, PeftModel
import torch.nn as nn
import torch.optim as optim
from huggingface_hub import snapshot_download
import logging


os.environ["HF_HOME"] = os.path.join(os.getcwd(), "hf_cache")
os.environ["TRANSFORMERS_CACHE"] = os.path.join(os.getcwd(), "hf_cache", "transformers")
os.environ["HF_HUB_OFFLINE"] = "1"

class file_processor:
    def __init__(self, args):
        self.ARGS = args
        self.USE_GPU = args.force_gpu or (not args.force_cpu and torch.cuda.is_available())
        if not self.ARGS.verbose:
            logging.getLogger("transformers").setLevel(logging.ERROR)


        model_dir = snapshot_download(args.embedding_model_name)
        
        adapter_config_path = os.path.join(model_dir, "adapter_config.json")
        if os.path.exists(adapter_config_path):
            # It's a PEFT model!
            with open(adapter_config_path, "r") as f:
                peft_config_data = json.load(f)
            base_model_dir = peft_config_data.get("base_model_name_or_path")

            # Load tokenizer from model_dir
            self.TOKENIZER = AutoTokenizer.from_pretrained(model_dir)

            if args.use_quantization:
                quantization_config = BitsAndBytesConfig(
                    load_in_4bit=True,
                    bnb_4bit_use_double_quant=True,
                    bnb_4bit_quant_type="nf4",
                    bnb_4bit_compute_dtype=torch.bfloat16
                )
            else:
                quantization_config = None

            load_kwargs = {
                "use_safetensors": True,
                "quantization_config": quantization_config,
            }
            if self.USE_GPU:
                load_kwargs["device_map"] = "auto"
            else:
                # Explicit CPU load: keep everything on CPU in fp32 to avoid any
                # bitsandbytes / accelerate GPU-only code paths.
                load_kwargs["torch_dtype"] = torch.float32

            base_model = AutoModel.from_pretrained(base_model_dir, **load_kwargs)
            if not self.USE_GPU:
                base_model = base_model.to("cpu")
            base_model.gradient_checkpointing_enable()

            self.MODEL = PeftModel.from_pretrained(base_model, model_dir, is_trainable=True)
        else:
            # Base model loading
            self.TOKENIZER = AutoTokenizer.from_pretrained(model_dir)

            if args.use_quantization:
                quantization_config = BitsAndBytesConfig(
                    load_in_4bit=True,
                    bnb_4bit_use_double_quant=True,
                    bnb_4bit_quant_type="nf4",
                    bnb_4bit_compute_dtype=torch.bfloat16
                )
            else:
                quantization_config = None

            load_kwargs = {
                "use_safetensors": True,
                "quantization_config": quantization_config,
            }
            if self.USE_GPU:
                load_kwargs["device_map"] = "auto"
            else:
                load_kwargs["torch_dtype"] = torch.float32

            self.MODEL = AutoModel.from_pretrained(model_dir, **load_kwargs)
            if not self.USE_GPU:
                self.MODEL = self.MODEL.to("cpu")

            # Prepare model for PEFT LoRA training
            if quantization_config is not None:
                self.MODEL = prepare_model_for_kbit_training(self.MODEL)
                peft_config = LoraConfig(
                    r=16,
                    lora_alpha=32,
                    target_modules=["q_proj", "v_proj", "k_proj", "o_proj", "gate_proj", "up_proj", "down_proj"],
                    lora_dropout=0.05,
                    bias="none",
                    task_type=None
                )
                self.MODEL = get_peft_model(self.MODEL, peft_config)
            else:
                self.MODEL.gradient_checkpointing_enable()

        if self.USE_GPU:
            self.DEVICE = getattr(self.MODEL, "device", torch.device("cuda"))
        else:
            self.DEVICE = torch.device("cpu")

    def tokenize(self, text):
        return self.TOKENIZER(
            text,
            return_tensors="pt",
            padding=False,
            truncation=False
        )
    
    def decode(self, input_ids):
        if torch.is_tensor(input_ids):
            input_ids = input_ids.squeeze().tolist()
        text = self.TOKENIZER.decode(
            input_ids,
            skip_special_tokens=True
        )
        return text

    def _pool_embeddings(self, outputs, attention_mask):
        if hasattr(outputs, 'pooler_output') and outputs.pooler_output is not None:
            return outputs.pooler_output
        else:
            last_hidden = outputs.last_hidden_state
            mask = attention_mask.unsqueeze(-1).expand(last_hidden.size()).float()
            masked_hidden = last_hidden * mask
            sum_hidden = torch.sum(masked_hidden, dim=1)
            sum_mask = torch.clamp(mask.sum(dim=1), min=1e-9)
            return sum_hidden / sum_mask

    def get_embedding(self, chunk):
        # Truncate chunk if it somehow exceeds TOKEN_SIZE (safety check)
        if chunk.size(0) > self.ARGS.token_size:
            chunk = chunk[:self.ARGS.token_size]

        # Use the actual length of the chunk without padding to config.TOKEN_SIZE.
        # This completely avoids extreme memory usage and OOM with large TOKEN_SIZE (e.g. 16k or 32k context lengths).
        chunk_tokens = {
            'input_ids': chunk.unsqueeze(0).to(self.DEVICE),
            'attention_mask': torch.ones((1, chunk.size(0)), dtype=torch.long, device=self.DEVICE)
        }

        with torch.no_grad():
            if hasattr(self.MODEL, "encoder") and getattr(self.MODEL.config, "is_encoder_decoder", False):
                outputs = self.MODEL.encoder(**chunk_tokens)
            else:
                outputs = self.MODEL(**chunk_tokens)
        
            embedding = self._pool_embeddings(outputs, chunk_tokens['attention_mask'])

            chunk_tokens_cpu = {
                'input_ids': chunk_tokens['input_ids'].cpu(),
                'attention_mask': chunk_tokens['attention_mask'].cpu()
            }
            return chunk_tokens_cpu, embedding.to(torch.float32).cpu().numpy()

    def get_embeddings_for_file(self, content):
        # Tokenize the entire representation
        tokens = self.TOKENIZER(
            content,
            return_tensors="pt",
            padding=False,
            truncation=False
        )

        input_ids = tokens['input_ids'].squeeze()

        
        # Define chunk size and overlap
        chunk_size = self.ARGS.token_size
        overlap = self.ARGS.chunk_overlap_size
        stride = chunk_size - overlap

        # Create overlapping chunks
        chunks_of_file = [
            input_ids[i:i + chunk_size]
            for i in range(0, input_ids.size(0), stride)
        ]

        embedded_chunks = {"chunk_tokens":[], "embedding": []}

        for chunk in chunks_of_file:
            chunk_tokens, embedding = self.get_embedding(chunk)
            embedded_chunks["chunk_tokens"].append(chunk_tokens)
            embedded_chunks["embedding"].append(embedding)

        return embedded_chunks