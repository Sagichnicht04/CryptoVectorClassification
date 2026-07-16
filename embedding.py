import os
import numpy as np
import json
import torch
import torch.amp
from transformers import AutoTokenizer, AutoModel
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
        self.TOKENIZER = AutoTokenizer.from_pretrained(
            model_dir,
            local_files_only=True
            )

        self.MODEL = AutoModel.from_pretrained(
            model_dir,
            use_safetensors=True,
            #torch_dtype=torch.float16,
            torch_dtype="auto",
            local_files_only=True
        ).cuda()

        # Enable gradient checkpointing to save VRAM
        self.MODEL.gradient_checkpointing_enable()

        self.DEVICE = torch.device("cuda")

        self.OPTIMIZER = optim.Adam(
            self.MODEL.parameters(),
            lr=1e-5
        )


    def tokenize(self, text):
        return self.TOKENIZER(
            text,
            return_tensors="pt",
            padding=False,
            truncation=False
        )
    
    def decode(self, input_ids):
        text = self.TOKENIZER.decode(
            input_ids,
            skip_special_tokens=True
        )
        return text

    def get_embedding(self, chunk):

        padded_chunk = torch.full((config.TOKEN_SIZE,), self.TOKENIZER.pad_token_id, dtype=torch.long)
        padded_chunk[:chunk.size(0)] = chunk
        
        chunk_tokens = {
            'input_ids': padded_chunk.unsqueeze(0).to(self.DEVICE),
            'attention_mask': (padded_chunk != self.TOKENIZER.pad_token_id).unsqueeze(0).to(self.DEVICE)
        }

        with torch.no_grad():
            outputs = self.MODEL(**chunk_tokens)
        
        
            if hasattr(outputs, 'pooler_output') and outputs.pooler_output is not None:
                embedding = outputs.pooler_output
            else:
                last_hidden = outputs.last_hidden_state
                attention_mask = chunk_tokens['attention_mask']
                mask = attention_mask.unsqueeze(-1).expand(last_hidden.size()).float()
                masked_hidden = last_hidden * mask
                sum_hidden = torch.sum(masked_hidden, dim=1)
                sum_mask = torch.clamp(mask.sum(dim=1), min=1e-9)
                embedding = sum_hidden / sum_mask


            return chunk_tokens, embedding.to(torch.float32).cpu().numpy()


    def finetune(self, dataset, batch_size=4):
        triplet_loss = nn.TripletMarginLoss(margin=0.5)

        anchor_ids_all = torch.cat(dataset["anchor_ids"], dim=0)
        anchor_mask_all = torch.cat(dataset["anchor_mask"], dim=0)

        positive_ids_all = torch.cat(dataset["positive_ids"], dim=0)
        positive_mask_all = torch.cat(dataset["positive_mask"], dim=0)

        negative_ids_all = torch.cat(dataset["negative_ids"], dim=0)
        negative_mask_all = torch.cat(dataset["negative_mask"], dim=0)

        for i in range(0, len(anchor_ids_all), batch_size):
            # Slice current batch
            anchor_ids = anchor_ids_all[i:i + batch_size].to(self.DEVICE)
            anchor_mask = anchor_mask_all[i:i + batch_size].to(self.DEVICE)

            positive_ids = positive_ids_all[i:i + batch_size].to(self.DEVICE)
            positive_mask = positive_mask_all[i:i + batch_size].to(self.DEVICE)

            # Slicing safely with negative ids in case sizes differ by 1
            negative_ids = negative_ids_all[i:i + batch_size].to(self.DEVICE)
            negative_mask = negative_mask_all[i:i + batch_size].to(self.DEVICE)

            self.OPTIMIZER.zero_grad()

            # Wrap in automatic mixed precision (autocast) context to cut memory in half
            with torch.amp.autocast("cuda"):
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

                # Use CLS token embedding (index 0)
                a_vec = anchor_emb.last_hidden_state[:, 0, :]
                p_vec = positive_emb.last_hidden_state[:, 0, :]
                n_vec = negative_emb.last_hidden_state[:, 0, :]

                loss = triplet_loss(
                    a_vec,
                    p_vec,
                    n_vec
                )

            loss.backward()
            self.OPTIMIZER.step()

            step = (i // batch_size) + 1
            total_steps = (len(anchor_ids_all) + batch_size - 1) // batch_size
            progress_percent = (step / total_steps) * 100
            print(f"\rFinetuning Batch {step}/{total_steps} ({progress_percent:.1f}%) | Triplet Loss: {loss.item():.4f}", end="", flush=True)
        print() 

    def save_model(self):
        self.MODEL.save_pretrained(config.FINE_TUNED_MODEL_DIR)
        self.TOKENIZER.save_pretrained(config.FINE_TUNED_MODEL_DIR)
