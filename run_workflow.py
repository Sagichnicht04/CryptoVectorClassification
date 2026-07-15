import config
import subprocess
from  embedding import embedding_model
from graph_representation import standardize_graph_representation, get_lang_from_path
import os
import torch


print("-- Step 1: Preparing Data --")
if config.PREPARE_DATA:
    subprocess.run(
        f"{config.INTERPRETER} dataset.py prepare",
        shell=True, check=True
    )
else:
    print("Skipped")

base_embedding = embedding_model(config.MODEL_NAME)

print("-- Step 2: Chunk Data --")
chunks = {}
if config.CHUNK_DATA:
    all_files = []

    for root, _, files in os.walk("./data"):
        for filename in files:
            path = os.path.join(root, filename)
            lang = get_lang_from_path(path)
            if lang:
                all_files.append((path, lang))
                
    
    chunk_counter = 0
    file_counter = 0
    for file, lang in all_files:
        with open(file, "r") as f:
            content = f.read()

        if config.REPRESENTATION == "graph":
            representation = standardize_graph_representation(lang, content)
        else:
            representation = content

        # Tokenize the entire representation
        tokens = base_embedding.TOKENIZER(
            representation,
            return_tensors="pt",
            padding=False,
            truncation=False
        )

        input_ids = tokens['input_ids'].squeeze()

        
        # Define chunk size and overlap
        chunk_size = config.TOKEN_SIZE
        overlap = config.OVERLAP
        stride = chunk_size - overlap

        # Create overlapping chunks
        chunks_of_file = [
            input_ids[i:i + chunk_size]
            for i in range(0, input_ids.size(0), stride)
        ]
        chunks[file] = chunks_of_file
        chunk_counter += len(chunks_of_file)
        file_counter += 1
        print(f"\rProcessed: {file_counter*100/len(all_files)}%", end="", flush=True)


    torch.save(chunks, "chunks.pt")
    print(f"Saved chunks for {len(chunks)} files to disk ({chunk_counter} chunks)")

else:
    chunks = torch.load("chunks.pt")
    for file in chunks.keys():
        if not os.path.exists(file):
            print("Fatal: Path of cached chunk file does not exist")
            exit()
    print("Loaded Cached Chunks from Disk")