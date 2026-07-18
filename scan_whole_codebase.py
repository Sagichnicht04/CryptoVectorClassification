import config
import subprocess
from  embedding import embedding_model
from graph_representation import standardize_graph_representation, get_lang_from_path
import os
import torch
from classifier import get_classifier
import random
import json
from evaluate import evaluation
import time

codebase = "./systemd"

# Either neural_network_binary_classifier or random_forest_classifier
CLASSIFIER = "neural_network_binary_classifier"

# Either graph or text
REPRESENTATION = "text"

FINETUNED_EMBED_CHUNKS = False
CHUNK_DATA = False
EVALUATE_CLASSIFIER = True

CHUNKS_PATH = f"cache/{os.path.basename(codebase)}-{REPRESENTATION}_chunks.pt"
FINE_TUNED_MODEL_DIR = f"cache/{REPRESENTATION}_{CLASSIFIER}_fine_tuned/"
FINE_TUNED_EMBEDDINGS_PATH = f"cache/{os.path.basename(codebase)}-{REPRESENTATION}_fine_tuned_embeddings.pt"
FINE_TUNED_CLASSIFIER_PATH = f"cache/fine_tuned_{REPRESENTATION}_{CLASSIFIER}.{"pkl" if REPRESENTATION == "graph" else "pt"}"
EVALUATION_RESULT_PATH = f"Evaluations/{os.path.basename(codebase)}-{REPRESENTATION}_{CLASSIFIER}_evaluation/"

fine_tuned_embedding = embedding_model(FINE_TUNED_MODEL_DIR)


print("-- Step 2: Chunk Data --")
chunks = {}
if CHUNK_DATA:
    all_files = []

    for root, _, files in os.walk(codebase):
        for filename in files:
            path = os.path.join(root, filename)
            lang = get_lang_from_path(path)
            if lang:
                all_files.append((path, lang))
                
    
    chunk_counter = 0
    file_counter = 0
    for file, lang in all_files:
        with open(file, "r", encoding="utf-8") as f:
            content = f.read()

        if REPRESENTATION == "graph":
            representation = standardize_graph_representation(lang, content)
        else:
            representation = content

        # Tokenize the entire representation
        tokens = fine_tuned_embedding.TOKENIZER(
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


    torch.save(chunks, CHUNKS_PATH)
    print(f"Saved chunks for {len(chunks)} files to disk ({chunk_counter} chunks)")

else:
    chunks = torch.load(CHUNKS_PATH,weights_only=False)
    for file in chunks:
        if not os.path.exists(file):
            print("Fatal: Path of cached chunk file does not exist")
            exit()
    print("Loaded Cached Chunks from Disk")



print("-- Step: Finetune embed Dataset --")
fine_tuned_embedded_chunks = {}
if FINETUNED_EMBED_CHUNKS:
    file_counter = 0
    for file in chunks:
        embedding_counter = 0
        file_counter += 1
        fine_tuned_embedded_chunks[file] = []
        for chunk in chunks[file]:
            chunk_tokens, embedding = fine_tuned_embedding.get_embedding(chunk)
            fine_tuned_embedded_chunks[file].append({
                "chunk_tokens":chunk_tokens,
                "embedding": embedding
            })
            embedding_counter += 1
            print(f"\rEmbedding file {file} ({file_counter}/{len(chunks)}) | Chunk {embedding_counter}/{len(chunks[file])} | ", end="", flush=True,
            )

    print()
    torch.save(fine_tuned_embedded_chunks, FINE_TUNED_EMBEDDINGS_PATH)
    print(f"Saved embeddings for {len(fine_tuned_embedded_chunks)} files to disk")
else:
    fine_tuned_embedded_chunks = torch.load(FINE_TUNED_EMBEDDINGS_PATH,weights_only=False)
    for file in fine_tuned_embedded_chunks:
        if not os.path.exists(file):
            print("Fatal: Path of cached embedding file does not exist")
            exit()
    print("Loaded Cached Embeddings from Disk")

print("-- Step 8: Load Fine Tuned Classifier --")
fine_tuned_classifier = get_classifier(FINE_TUNED_CLASSIFIER_PATH)
fine_tuned_classifier.load()



print("-- Step 9: Evaluate Classifier --")
if EVALUATE_CLASSIFIER:
    crypto_embeddings = {}
    non_crypto_embeddings = {}
    discarded_crypto_embeddings = {}

    for file in fine_tuned_embedded_chunks:
        embeddings = []
        chunk_tokens = []
        for chunk in fine_tuned_embedded_chunks[file]:
            embeddings.append(chunk["embedding"])
            chunk_tokens.append(chunk["chunk_tokens"])

        probabilities = fine_tuned_classifier.predict_proba(embeddings)
                
    
        if "data/evaluation/novel_crypto" in file:
            crypto_embeddings[file] = [] 
            for index, probability in enumerate(probabilities):
                crypto_embeddings[file].append({
                    "probability": probability[1],
                    "clear_text": fine_tuned_embedding.decode(chunk_tokens[index]["input_ids"])
                })
                

        if "data/evaluation/non_crypto/non_crypto_dataset" in file:
            non_crypto_embeddings[file] = [] 
            for index, probability in enumerate(probabilities):
                non_crypto_embeddings[file].append({
                    "probability": probability[1],
                    "clear_text": fine_tuned_embedding.decode(chunk_tokens[index]["input_ids"])
                })
                
        if "data/evaluation/non_crypto/discarded_dataset" in file:
            discarded_crypto_embeddings[file] = [] 
            for index, probability in enumerate(probabilities):
                discarded_crypto_embeddings[file].append({
                    "probability": probability[1],
                    "clear_text": fine_tuned_embedding.decode(chunk_tokens[index]["input_ids"])
                })

    os.makedirs(config.EVALUATION_RESULT_PATH, exist_ok=True)

    with open(f"{config.EVALUATION_RESULT_PATH}crypto_results.json", "w") as f:
        json.dump(crypto_embeddings, f)
    with open(f"{config.EVALUATION_RESULT_PATH}non_crypto_results.json", "w") as f:
        json.dump(non_crypto_embeddings, f)
    with open(f"{config.EVALUATION_RESULT_PATH}api_crypto_results.json", "w") as f:
        json.dump(discarded_crypto_embeddings, f)

    print(f"Predicted {len(detected_crypto_chunks)} crypto chunks out of  chunks in crypto files")
    torch.save(detected_crypto_chunks, config.CHUNK_CLASSIFICATION_PATH)
    print(f"Saved predictions to disk")


    evaluator = evaluation()
    results = evaluator.evaluate(crypto_embeddings,non_crypto_embeddings,discarded_crypto_embeddings)
    print(results)

    with open(f"{config.EVALUATION_RESULT_PATH}evaluation.json", "w") as f:
        json.dump(results, f)
else:
    pass