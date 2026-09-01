import os
import numpy as np
import json
import torch
import torch.amp
from transformers import AutoTokenizer, AutoModel, BitsAndBytesConfig
from peft import LoraConfig, get_peft_model, prepare_model_for_kbit_training, PeftModel
import config
import torch.nn as nn
import torch.optim as optim

os.environ["HF_HOME"] = os.path.join(os.getcwd(), "hf_cache")
os.environ["TRANSFORMERS_CACHE"] = os.path.join(os.getcwd(), "hf_cache", "transformers")
os.environ["HF_HUB_OFFLINE"] = "1"

def get_lang_from_path(path):
    if path.endswith('.py'): return 'python'
    if path.endswith(('.c', '.h')): return 'c'
    if path.endswith(('.cpp', '.hpp', '.hh', '.cc', '.cxx')): return 'cpp'
    if path.endswith('.java'): return 'java'
    return None


class embedding_model:

    def __init__(self, model_dir):
        # Honour the global USE_GPU flag AND the actual runtime availability of CUDA.
        # 4-bit bitsandbytes quantization and `device_map="auto"` require a CUDA GPU;
        # when running on CPU we load the model in fp32 without any quantization.
        use_gpu = bool(getattr(config, "USE_GPU", True)) and torch.cuda.is_available()

        adapter_config_path = os.path.join(model_dir, "adapter_config.json")
        if os.path.exists(adapter_config_path):
            # It's a PEFT model!
            with open(adapter_config_path, "r") as f:
                peft_config_data = json.load(f)
            base_model_dir = peft_config_data.get("base_model_name_or_path")

            # Load tokenizer from model_dir
            self.TOKENIZER = AutoTokenizer.from_pretrained(model_dir)

            # Load base model in 4-bit NF4 (GPU only). On CPU we skip quantization.
            if use_gpu and ("7B" in base_model_dir or "15B" in base_model_dir or "1.5B" in base_model_dir):
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
            if use_gpu:
                load_kwargs["device_map"] = "auto"
            else:
                # Explicit CPU load: keep everything on CPU in fp32 to avoid any
                # bitsandbytes / accelerate GPU-only code paths.
                load_kwargs["torch_dtype"] = torch.float32

            base_model = AutoModel.from_pretrained(base_model_dir, **load_kwargs)
            if not use_gpu:
                base_model = base_model.to("cpu")
            base_model.gradient_checkpointing_enable()

            self.MODEL = PeftModel.from_pretrained(base_model, model_dir, is_trainable=True)
        else:
            # Base model loading
            self.TOKENIZER = AutoTokenizer.from_pretrained(model_dir)

            # Load in 4-bit for QLoRA to fit 12 GB of VRAM perfectly and stably (GPU only).
            if use_gpu and ("7B" in model_dir or "15B" in model_dir or "1.5B" in model_dir):
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
            if use_gpu:
                load_kwargs["device_map"] = "auto"
            else:
                load_kwargs["torch_dtype"] = torch.float32

            self.MODEL = AutoModel.from_pretrained(model_dir, **load_kwargs)
            if not use_gpu:
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

        # Force the effective device: CPU when USE_GPU is disabled / no CUDA available,
        # otherwise fall back to whatever HF dispatched the model onto.
        if use_gpu:
            self.DEVICE = getattr(self.MODEL, "device", torch.device("cuda"))
        else:
            self.DEVICE = torch.device("cpu")

        # Optimize only parameters that require gradients (the LoRA adapter weights)
        try:
            self.OPTIMIZER = optim.Adam(
                filter(lambda p: p.requires_grad, self.MODEL.parameters()),
                lr=1e-5
            )
        except:
            print("ERROR: Could not initialize Optimizer")


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
        if chunk.size(0) > config.TOKEN_SIZE:
            chunk = chunk[:config.TOKEN_SIZE]

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


    def finetune(self, dataset, batch_size=1, accumulation_steps=8):
        triplet_loss = nn.TripletMarginLoss(margin=0.5)

        anchor_ids_list = dataset["anchor_ids"]
        anchor_mask_list = dataset["anchor_mask"]

        positive_ids_list = dataset["positive_ids"]
        positive_mask_list = dataset["positive_mask"]

        negative_ids_list = dataset["negative_ids"]
        negative_mask_list = dataset["negative_mask"]

        pad_id = self.TOKENIZER.pad_token_id if self.TOKENIZER.pad_token_id is not None else 0

        def pad_and_batch(tensors, pad_value, max_len=config.TOKEN_SIZE):
            processed = []
            for t in tensors:
                if not torch.is_tensor(t):
                    t = torch.tensor(t)
                t = t.squeeze()
                if t.dim() == 0:
                    t = t.unsqueeze(0)
                if t.size(0) > max_len:
                    t = t[:max_len]
                processed.append(t)
            if not processed:
                return torch.zeros((0, max_len), dtype=torch.long)
            return nn.utils.rnn.pad_sequence(processed, batch_first=True, padding_value=pad_value)

        total_samples = len(anchor_ids_list)
        total_steps = (total_samples + batch_size - 1) // batch_size

        self.MODEL.train()
        device_type = "cuda" if "cuda" in str(self.DEVICE) else "cpu"
        amp_dtype = torch.bfloat16 if device_type == "cuda" else torch.float32

        # Reset gradients before starting
        self.OPTIMIZER.zero_grad()

        with torch.enable_grad():
            for step_idx, i in enumerate(range(0, total_samples, batch_size)):
                # Slice current batch
                anchor_ids = pad_and_batch(anchor_ids_list[i:i + batch_size], pad_id, max_len=config.TOKEN_SIZE).to(self.DEVICE)
                anchor_mask = pad_and_batch(anchor_mask_list[i:i + batch_size], 0, max_len=config.TOKEN_SIZE).to(self.DEVICE)

                positive_ids = pad_and_batch(positive_ids_list[i:i + batch_size], pad_id, max_len=config.TOKEN_SIZE).to(self.DEVICE)
                positive_mask = pad_and_batch(positive_mask_list[i:i + batch_size], 0, max_len=config.TOKEN_SIZE).to(self.DEVICE)

                # Slicing safely with negative ids in case sizes differ by 1
                negative_ids = pad_and_batch(negative_ids_list[i:i + batch_size], pad_id, max_len=config.TOKEN_SIZE).to(self.DEVICE)
                negative_mask = pad_and_batch(negative_mask_list[i:i + batch_size], 0, max_len=config.TOKEN_SIZE).to(self.DEVICE)

                # Wrap in automatic mixed precision (autocast) context to cut memory in half
                with torch.amp.autocast(device_type=device_type, dtype=amp_dtype):
                    anchor_emb = self.MODEL(
                        input_ids=anchor_ids,
                        attention_mask=anchor_mask
                    )

                    positive_emb = self.MODEL(
                        input_ids=positive_ids,
                        attention_mask=positive_mask
                    )

                    negative_emb = self.MODEL(
                        input_ids=negative_ids,
                        attention_mask=negative_mask
                    )

                    # Use consistent sequence pooling logic matching get_embedding
                    a_vec = self._pool_embeddings(anchor_emb, anchor_mask)
                    p_vec = self._pool_embeddings(positive_emb, positive_mask)
                    n_vec = self._pool_embeddings(negative_emb, negative_mask)

                    loss = triplet_loss(
                        a_vec,
                        p_vec,
                        n_vec
                    )
                    
                    # Scale loss to account for gradient accumulation steps
                    if accumulation_steps > 1:
                        loss = loss / accumulation_steps

                loss.backward()

                # Step optimizer only after accumulating gradients over multiple steps
                step = step_idx + 1
                if (step % accumulation_steps == 0) or (i + batch_size >= total_samples):
                    self.OPTIMIZER.step()
                    self.OPTIMIZER.zero_grad()

                progress_percent = (step / total_steps) * 100
                # Scale loss printing back up to represent actual triplet loss value
                print(f"\rFinetuning Batch {step}/{total_steps} ({progress_percent:.1f}%) | Triplet Loss: {loss.item() * (accumulation_steps if accumulation_steps > 1 else 1):.4f}", end="", flush=True)
        print() 
        self.MODEL.eval()

    def save_model(self):
        self.MODEL.save_pretrained(config.FINE_TUNED_MODEL_DIR)
        self.TOKENIZER.save_pretrained(config.FINE_TUNED_MODEL_DIR)
