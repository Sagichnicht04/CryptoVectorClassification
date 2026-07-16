import os
import numpy as np
import json
import torch
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


    def finetune(self, dataset):
        triplet_loss = nn.TripletMarginLoss(margin=0.5)


        anchor_ids = dataset["anchor_ids"].to(self.DEVICE)
        anchor_mask = dataset["anchor_mask"].to(self.DEVICE)

        positive_ids = dataset["positive_ids"].to(self.DEVICE)
        positive_mask = dataset["positive_mask"].to(self.DEVICE)

        negative_ids = dataset["negative_ids"].to(self.DEVICE)
        negative_mask = dataset["negative_mask"].to(self.DEVICE)

        self.OPTIMIZER.zero_grad()

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

        loss = triplet_loss(
            anchor_emb,
            positive_emb,
            negative_emb
        )

        loss.backward()
        self.OPTIMIZER.step()

