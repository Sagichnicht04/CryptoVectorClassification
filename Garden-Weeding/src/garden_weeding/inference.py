import subprocess
from embedding import embedding_model
from classifier import get_classifier
import os
import torch
import random
import json
from evaluate import evaluation
from cache import get_uncached_files

def inference(args):
    print(get_uncached_files(args))

def ignore():

    _effective_gpu = bool(getattr(config, "USE_GPU", True)) and torch.cuda.is_available()
    print(f"-- Runtime device: {'GPU (CUDA)' if _effective_gpu else 'CPU'}"
        f" (config.USE_GPU={getattr(config, 'USE_GPU', True)}, cuda.available={torch.cuda.is_available()}) --")

    base_embedding = embedding_model(config.MODEL_NAME)
    detected_crypto_chunks = []


    updated_files = []
    for root, _, files in os.walk(config.DATA_DIR):
        for filename in files:
            path = os.path.join(root, filename)
            updated_files.append(path)

    def get_lang_from_path(path):
        if path.endswith('.c'):
            return 'c'
        if path.endswith(('.cpp', '.cc', '.cxx')):
            return 'cpp'
        return None

    print("-- Step 2: Chunk Data --")
    chunks = {}
    if config.CHUNK_DATA:
        all_files = []

        chunk_counter = 0
        file_counter = 0
        for root, _, files in os.walk(config.DATA_DIR):
            for filename in files:
                path = os.path.join(root, filename)
                lang = get_lang_from_path(path)
                if lang:
                    new_path = "/".join(path.split("/")[::-1][0:4][::-1])
                    all_files.append((new_path, lang))
                    
                    with open(path, "r", encoding='utf-8', errors="replace") as f:
                        content = f.read()

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
                    chunks[new_path] = chunks_of_file
                    chunk_counter += len(chunks_of_file)
                    file_counter += 1
                    print(f"\rProcessed: {file_counter*100/len(files)}%", end="", flush=True)


        torch.save(chunks, config.CHUNKS_PATH)
        print(f"Saved chunks for {len(chunks)} files to disk ({chunk_counter} chunks)")

    else:
        chunks = torch.load(config.CHUNKS_PATH,weights_only=False)
        print("Loaded Cached Chunks from Disk")


    print("-- Step 3: Base Embed Chunks --")
    base_embedded_chunks = {}

    try:
        base_embedded_chunks = torch.load(config.BASE_EMBEDDINGS_PATH,weights_only=False)
        print("Loaded Cached Embeddings from Disk")
    except:
        print("No Cached Embeddings Found")

    if config.BASE_EMBED_CHUNKS:
        file_counter = 0
        for file in chunks:
            if file in base_embedded_chunks:
                continue
            embedding_counter = 0
            file_counter += 1
            base_embedded_chunks[file] = []
            for chunk in chunks[file]:
                chunk_tokens, embedding = base_embedding.get_embedding(chunk)
                base_embedded_chunks[file].append({
                    "chunk_tokens":chunk_tokens,
                    "embedding": embedding
                })
                embedding_counter += 1
                print(f"\rEmbedding file {file} ({file_counter}/{len(chunks)}) | Chunk {embedding_counter}/{len(chunks[file])} | ", end="", flush=True,
                )

        print()
        torch.save(base_embedded_chunks, config.BASE_EMBEDDINGS_PATH)
        print(f"Saved embeddings for {len(base_embedded_chunks)} files to disk")
    else:
        base_embedded_chunks = torch.load(config.BASE_EMBEDDINGS_PATH,weights_only=False)
        print("Loaded Cached Embeddings from Disk")

    l = list(base_embedded_chunks.items())
    random.shuffle(l)
    base_embedded_chunks = dict(l)


    print("-- Step 4: Load dirty Classifier --")
    dirty_classifier = get_classifier("random_forest_classifier", config.DIRTY_CLASSIFIER_PATH,
                                        {
                                            "n_estimators": 200,
                                            "max_depth": 12,
                                            "min_samples_split": 5,
                                            "bootstrap": False,
                                            "seed": 0
                                        })
    if not dirty_classifier.load():
        print(f"Error: Classifier not found {dirty_classifier.path}")
        exit()

    fine_tuned_embedding = base_embedding
    fine_tuned_embedded_chunks = base_embedded_chunks
    fine_tuned_classifier = dirty_classifier


    print("-- Step 9: Evaluate Classifier --")
    if config.EVALUATE_CLASSIFIER:
        crypto_embeddings = {}
        non_crypto_embeddings = {}
        discarded_crypto_embeddings = {}
        files_probas = {}

        for file in fine_tuned_embedded_chunks:

            lang = get_lang_from_path(file)
            if lang:
                embeddings = []
                chunk_tokens = []
                for chunk in fine_tuned_embedded_chunks[file]:
                    embeddings.append(chunk["embedding"])
                    chunk_tokens.append(chunk["chunk_tokens"])

                probabilities = fine_tuned_classifier.predict_proba(embeddings)
                proba = 0
                discarded_crypto_embeddings[file] = [] 
                for index, probability in enumerate(probabilities):
                    discarded_crypto_embeddings[file].append({
                        "probability": probability[1],
                        "clear_text": fine_tuned_embedding.decode(chunk_tokens[index]["input_ids"])
                    })
                    if probability[1] > proba:
                        proba = probability[1]
                files_probas[file] = proba
                    

        os.makedirs(config.EVALUATION_RESULT_PATH, exist_ok=True)

        with open(f"{config.EVALUATION_RESULT_PATH}crypto_results.json", "w") as f:
            json.dump(crypto_embeddings, f)
        with open(f"{config.EVALUATION_RESULT_PATH}non_crypto_results.json", "w") as f:
            json.dump(non_crypto_embeddings, f)
        with open(f"{config.EVALUATION_RESULT_PATH}api_crypto_results.json", "w") as f:
            json.dump(discarded_crypto_embeddings, f)
        with open(f"{config.EVALUATION_RESULT_PATH}probas.json", "w") as f:
                json.dump(files_probas, f)


        evaluator = evaluation()

        id = config.EVALUATION_RESULT_PATH

        os.makedirs(f"{id}", exist_ok=True)


        results, html = evaluator.evaluate(crypto_embeddings,non_crypto_embeddings,discarded_crypto_embeddings, "text", "random_forest_classifier")
        with open(f"{id}/evaluation.json", "w") as f:
            json.dump(results, f)

        with open(f"{id}/evaluation.html", "w", encoding="utf-8") as f:
            f.write(html)

        print(f"Evaluated {id} - F1-Score: {results["best_f1_metrics"]["f1"]}")
