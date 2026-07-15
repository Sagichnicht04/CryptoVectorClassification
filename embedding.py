import os
import numpy as np
import json
import torch
from transformers import AutoTokenizer, AutoModel
import config

os.environ["HF_HOME"] = os.path.join(os.getcwd(), "hf_cache")
os.environ["TRANSFORMERS_CACHE"] = os.path.join(os.getcwd(), "hf_cache", "transformers")

def get_lang_from_path(path):
    if path.endswith('.py'): return 'python'
    if path.endswith(('.c', '.h')): return 'c'
    if path.endswith(('.cpp', '.hpp', '.hh', '.cc', '.cxx')): return 'cpp'
    if path.endswith('.java'): return 'java'
    return None


class embedding_model:

    def __init__(self, model_dir):
        self.TOKENIZER = AutoTokenizer.from_pretrained(model_dir)

        self.MODEL = AutoModel.from_pretrained(
            model_dir,
            use_safetensors=True,
            #torch_dtype=torch.float16,
            torch_dtype="auto",
        ).cuda()

        self.DEVICE = torch.device("cuda")


    def tokenize(self, text):
        return self.TOKENIZER(
            text,
            return_tensors="pt",
            padding=False,
            truncation=False
        )

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


            return embedding.to(torch.float32).cpu().numpy()


        




def embed_dataset(model, tokenizer):
    """
    Embeds a dataset of source files based on the settings in config.py.

    Saves:
      - FAISS index  (config.INDEX_PATH)         – crypto embeddings only
      - file map     (config.MAP_PATH)            – paths for indexed files
      - labeled npz  (config.RF_EMBEDDINGS_PATH) – crypto=1 + non-crypto=0
    """
    print("--- Embedding Dataset ---")

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Using device: {device}")
    model.to(device)
    model.eval()

    if tokenizer.pad_token is None:
        tokenizer.pad_token = tokenizer.eos_token

    lang_ext_map = {
        "python": [".py"],
        "c": [".c", ".h"],
        "cpp": [".cpp", ".hpp", ".hh", ".cc", ".cxx"],
        "java": [".java"]
    }
    extensions = tuple([ext for lang in config.LANGUAGES for ext in lang_ext_map.get(lang, [])])

    all_files = []
    for root, _, files in os.walk("data/training/crypto"):
        for file in files:
            if file.endswith(extensions):
                all_files.append(os.path.join(root, file))
    
    import random
    all_files.sort()
    random.seed(42)
    random.shuffle(all_files)
    all_files = all_files[:config.NUM_TRAIN_FILES]
    print(f"Sampling {len(all_files)} files to process for languages: {config.LANGUAGES}")

    D = model.config.hidden_size
    # FAISS index is no longer used with chunking
    # index = faiss.IndexFlatIP(D)

    successful_file_paths = []
    problematic_files = []
    all_embeddings = []
    all_labels = []

    with torch.no_grad():
        # --- 1. Crypto dataset files (label=1) ---
        for i, file_path in enumerate(all_files):
            print(f"Processing file {i+1}/{len(all_files)}: {file_path}")
            try:
                with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                if not content.strip():
                    print("  - SKIPPED (empty file)")
                    continue
                lang = get_lang_from_path(file_path)
                emb = _embed_code(content, lang, tokenizer, model, device)
                if not emb:
                    problematic_files.append((file_path, f"Could not generate {config.INPUT_TYPE} representation."))
                    print("  - FAILED (Representation generation error)")
                    continue
                
                # With chunking, emb is a list of embeddings
                all_embeddings.append(emb)
                all_labels.append(1)
                successful_file_paths.append(file_path)
                print(f"  - SUCCESS (generated {len(emb)} chunks)")
            except Exception as e:
                problematic_files.append((file_path, str(e)))
                print(f"  - FAILED (Exception: {e})")


        # --- 3. Non-crypto snippets (label=0) ---
        print("\n--- Embedding non-crypto training snippets ---")
        neg_count = 0
        for lang, code in _load_snippets_from_dir("data/training/non_crypto"):
            emb = _embed_code(code, lang, tokenizer, model, device)
            if emb:
                all_embeddings.append(emb)
                all_labels.append(0)
                neg_count += 1
        print(f"  Non-crypto snippets embedded: {neg_count}")

    # --- Save paths for RF embeddings ---
    print("\n--------------------")
    print("Processing complete.")
    print(f"Saving file map to '{config.MAP_PATH}'...")
    with open(config.MAP_PATH, "w") as f:
        json.dump(successful_file_paths, f)
    print("Save complete.")

    # --- Save labeled embeddings for RF ---
    if all_embeddings:
        # Save as an object array since the inner lists can have different lengths
        np.savez(config.RF_EMBEDDINGS_PATH, embeddings=np.array(all_embeddings, dtype=object), labels=np.array(all_labels))
        n_c = int(sum(all_labels))
        n_nc = len(all_labels) - n_c
        print(f"Labeled embeddings saved to '{config.RF_EMBEDDINGS_PATH}' ({n_c} crypto, {n_nc} non-crypto).")

    if problematic_files:
        print(f"\nFailed to process: {len(problematic_files)} files.")
        with open("problematic_files_log.txt", "w") as f:
            for path, reason in problematic_files:
                f.write(f"{path} - {reason}\n")
        print("Log saved to 'problematic_files_log.txt'")

if __name__ == "__main__":
    pass
