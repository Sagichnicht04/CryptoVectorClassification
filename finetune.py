import os
import json
from torch.utils.data import Dataset, DataLoader
from transformers import AutoTokenizer, AutoModel, BitsAndBytesConfig
from torch.optim import AdamW
import torch
import config
from peft import get_peft_model, LoraConfig
from evaluate import get_embedding
import random

os.environ['TOKENIZERS_PARALLELISM'] = 'false'

class TripletDataset(Dataset):
    def __init__(self, tokenizer, crypto_chunks, non_crypto_chunks):
        self.tokenizer = tokenizer
        self.crypto_chunks = crypto_chunks
        self.non_crypto_chunks = non_crypto_chunks
        
        print(f"Loaded {len(self.crypto_chunks)} crypto chunks and {len(self.non_crypto_chunks)} non-crypto chunks.")

    def __len__(self):
        return len(self.crypto_chunks)

    def __getitem__(self, idx):
        anchor = self.crypto_chunks[idx]
        
        positive = random.choice(
            [f for f in self.crypto_chunks if f != anchor]
        )
        
        negative = self.non_crypto_chunks[torch.randint(len(self.non_crypto_chunks), (1,))]

        def tokenize_and_pad(text):
            tokens = self.tokenizer(
                text,
                return_tensors="pt",
                padding='max_length',
                truncation=True,
                max_length=config.TOKEN_SIZE
            )
            return tokens['input_ids'].squeeze(), tokens['attention_mask'].squeeze()

        anchor_ids, anchor_mask = tokenize_and_pad(anchor)
        positive_ids, positive_mask = tokenize_and_pad(positive)
        negative_ids, negative_mask = tokenize_and_pad(negative)

        return {
            "anchor_ids": anchor_ids,
            "anchor_mask": anchor_mask,
            "positive_ids": positive_ids,
            "positive_mask": positive_mask,
            "negative_ids": negative_ids,
            "negative_mask": negative_mask,
        }

def finetune():
    """
    Fine-tunes a model based on the settings in config.py.
    """
    if not config.DO_FINETUNE:
        print("--- Skipping Fine-tuning as per config ---")
        return

    print("--- Fine-tuning Model ---")
    
    print(f"Loading pre-trained model and tokenizer: {config.MODEL_NAME}")
    
    quantization_config = BitsAndBytesConfig(
        load_in_4bit=True,
        bnb_4bit_compute_dtype=torch.float16
    )

    tokenizer = AutoTokenizer.from_pretrained(config.MODEL_NAME)
    if tokenizer.pad_token is None:
        tokenizer.pad_token = tokenizer.eos_token
    model = AutoModel.from_pretrained(
        config.MODEL_NAME,
        quantization_config=quantization_config,
        use_safetensors=True,
        torch_dtype="auto"
    )
    
    peft_config = LoraConfig(
        r=16,
        lora_alpha=32,
        lora_dropout=0.1,
        bias="none",
        task_type="FEATURE_EXTRACTION"
    )
    model = get_peft_model(model, peft_config)
    model.print_trainable_parameters()

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Using device: {device}")
    model.to(device)

    print(f"Loading dataset: {config.FINETUNING_DATASET}")
    train_dataset = TripletDataset(tokenizer, config.FINETUNING_DATASET)
    print(f"Loaded {len(train_dataset)} training triplets.")
    
    def collate_fn(batch):
        anchor_ids = torch.stack([item['anchor_ids'] for item in batch])
        anchor_mask = torch.stack([item['anchor_mask'] for item in batch])
        positive_ids = torch.stack([item['positive_ids'] for item in batch])
        positive_mask = torch.stack([item['positive_mask'] for item in batch])
        negative_ids = torch.stack([item['negative_ids'] for item in batch])
        negative_mask = torch.stack([item['negative_mask'] for item in batch])
        return {
            'anchor_ids': anchor_ids, 'anchor_mask': anchor_mask,
            'positive_ids': positive_ids, 'positive_mask': positive_mask,
            'negative_ids': negative_ids, 'negative_mask': negative_mask,
        }

    train_dataloader = DataLoader(train_dataset, batch_size=8, shuffle=True, collate_fn=collate_fn)

    optimizer = AdamW(model.parameters(), lr=5e-5) # Increased learning rate for PEFT
    loss_fct = torch.nn.TripletMarginLoss(margin=1.0)
    
    print("Starting fine-tuning loop...")
    model.train()
    
    num_epochs = 1 # Increased epochs for better training with PEFT
    for epoch in range(num_epochs):
        print(f"--- Epoch {epoch + 1}/{num_epochs} ---")
        total_loss = 0
        for i, batch in enumerate(train_dataloader):
            optimizer.zero_grad()
            
            anchor_ids = batch['anchor_ids'].to(device)
            anchor_mask = batch['anchor_mask'].to(device)
            positive_ids = batch['positive_ids'].to(device)
            positive_mask = batch['positive_mask'].to(device)
            negative_ids = batch['negative_ids'].to(device)
            negative_mask = batch['negative_mask'].to(device)
            
            anchor_embedding = get_embedding(model(input_ids=anchor_ids, attention_mask=anchor_mask), {"attention_mask": anchor_mask, "input_ids": anchor_ids})
            positive_embedding = get_embedding(model(input_ids=positive_ids, attention_mask=positive_mask), {"attention_mask": positive_mask, "input_ids": positive_ids})
            negative_embedding = get_embedding(model(input_ids=negative_ids, attention_mask=negative_mask), {"attention_mask": negative_mask, "input_ids": negative_ids})
            
            loss = loss_fct(anchor_embedding, positive_embedding, negative_embedding)
            
            loss.backward()
            optimizer.step()
            
            total_loss += loss.item()
            if i % 2 == 0:
                print(f"  Batch {i}/{len(train_dataloader)}, Loss: {loss.item()}")
        
        avg_loss = total_loss / len(train_dataloader)
        print(f"  Average Loss for Epoch {epoch + 1}: {avg_loss}")

    print(f"\nFine-tuning complete. Saving model to '{config.FINETUNED_MODEL_DIR}'")
    if not os.path.exists(config.FINETUNED_MODEL_DIR):
        os.makedirs(config.FINETUNED_MODEL_DIR)
        
    model.save_pretrained(config.FINETUNED_MODEL_DIR)
    tokenizer.save_pretrained(config.FINETUNED_MODEL_DIR)
    print("Fine-tuned model saved successfully.")

if __name__ == "__main__":
    finetune()