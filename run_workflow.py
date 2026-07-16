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

if config.CHUNK_DATA or config.BASE_EMBED_CHUNKS or config.FINETUNE_MODEL:
    base_embedding = embedding_model(config.MODEL_NAME)


print("-- Step 1: Preparing Data --")
if config.PREPARE_DATA:
    subprocess.run(
        f"{config.INTERPRETER} dataset.py prepare",
        shell=True, check=True
    )
else:
    print("Skipped")


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


    torch.save(chunks, config.CHUNKS_PATH)
    print(f"Saved chunks for {len(chunks)} files to disk ({chunk_counter} chunks)")

else:
    chunks = torch.load(config.CHUNKS_PATH,weights_only=False)
    for file in chunks:
        if not os.path.exists(file):
            print("Fatal: Path of cached chunk file does not exist")
            exit()
    print("Loaded Cached Chunks from Disk")


print("-- Step 3: Base Embed Chunks --")
base_embedded_chunks = {}
if config.BASE_EMBED_CHUNKS:
    file_counter = 0
    for file in chunks:
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
    if config.TRAIN_DIRTY_CLASSIFIER or config.CLASSIFY_CHUNKS or config.FINETUNE_MODEL:
        base_embedded_chunks = torch.load(config.BASE_EMBEDDINGS_PATH,weights_only=False)
        for file in base_embedded_chunks:
            if not os.path.exists(file):
                print("Fatal: Path of cached embedding file does not exist")
                exit()
        print("Loaded Cached Embeddings from Disk")

l = list(base_embedded_chunks.items())
random.shuffle(l)
base_embedded_chunks = dict(l)


print("-- Step 4: Train dirty Classifier --")
dirty_classifier = get_classifier(config.DIRTY_CLASSIFIER_PATH)
if config.TRAIN_DIRTY_CLASSIFIER:
    crypto_embeddings = []
    non_crypto_embeddings = []

    for file in base_embedded_chunks:
        if "data/training/crypto" in file:
            for chunk in base_embedded_chunks[file]:
                crypto_embeddings.append(chunk["embedding"])
        elif "data/training/non_crypto" in file:
            for chunk in base_embedded_chunks[file]:
                non_crypto_embeddings.append(chunk["embedding"])
    dirty_classifier.train(crypto_embeddings, non_crypto_embeddings)
else:
    dirty_classifier.load()

print("-- Step 5: Classify Chunks --")
if config.CLASSIFY_CHUNKS:
    detected_crypto_chunks = []
    embedding_counter = 0
    different_files = []
    files_sum = 0
    for file in base_embedded_chunks:
        if "data/training/crypto" in file:
            files_sum += 1
            embeddings = []
            chunk_tokens = []
            for chunk in base_embedded_chunks[file]:
                embeddings.append(chunk["embedding"])
                chunk_tokens.append(chunk["chunk_tokens"])

            probabilities = dirty_classifier.predict_proba(embeddings)
            for index, probability in enumerate(probabilities):
                embedding_counter += 1
                if probability[1] > config.CLASSIFIER_THRESHOLD:
                    different_files.append(file)
                    detected_crypto_chunks.append({
                        "file": file,
                        "chunk_tokens": chunk_tokens[index],
                    })
    different_files = list(set(different_files))
    print(f"Using {len(different_files)}/{files_sum} crypto files")
    print(f"Predicted {len(detected_crypto_chunks)} crypto chunks out of {embedding_counter} chunks in crypto files")
    torch.save(detected_crypto_chunks, config.CHUNK_CLASSIFICATION_PATH)
    print(f"Saved predictions to disk")
else:
    detected_crypto_chunks = torch.load(config.CHUNK_CLASSIFICATION_PATH,weights_only=False)
    if len(detected_crypto_chunks) == 0:
        print("Fatal: Empty Crypto Chunk Classification File")
        exit()
    print("Loaded Cached Chunk Classifications from Disk")


print("-- Step 6: Finetuning Model --")
if config.FINETUNE_MODEL:
    dataset = {
        "anchor_ids" : [],
        "anchor_mask" : [],
        "positive_ids" : [],
        "positive_mask" : [],
        "negative_ids" : [],
        "negative_mask" : [],
    }

    for i, crypto_chunk in enumerate(detected_crypto_chunks):
        dataset["anchor_ids"].append(crypto_chunk["chunk_tokens"]["input_ids"])
        dataset["anchor_mask"].append(crypto_chunk["chunk_tokens"]["attention_mask"])

        positive_idx = random.randrange(len(detected_crypto_chunks) - 1)
        if positive_idx >= i:
            positive_idx += 1

        positive_chunk = detected_crypto_chunks[positive_idx]

        dataset["positive_ids"].append(
            positive_chunk["chunk_tokens"]["input_ids"]
        )
        dataset["positive_mask"].append(
            positive_chunk["chunk_tokens"]["attention_mask"]
        )

    for file in base_embedded_chunks:
        if "data/training/non_crypto" in file:
            for non_crypto_chunk in base_embedded_chunks[file]:
                if len(dataset["anchor_ids"]) > len(dataset["negative_ids"]):
                    dataset["negative_ids"].append(non_crypto_chunk["chunk_tokens"]["input_ids"])
                    dataset["negative_mask"].append(non_crypto_chunk["chunk_tokens"]["attention_mask"])

    base_embedding.finetune(dataset)
    print(f"Fine Tuned model with {len(dataset["anchor_ids"])} anchors, {len(dataset["positive_ids"])} positives and {len(dataset["negative_ids"])} negatives")
    base_embedding.save_model()
else:
    print("Skipped")

if config.FINETUNED_EMBED_CHUNKS or config.EVALUATE_CLASSIFIER:
    fine_tuned_embedding = embedding_model(config.FINE_TUNED_MODEL_DIR)

print("-- Step 7: Fine Tuned Embed Chunks --")
fine_tuned_embedded_chunks = {}
if config.FINETUNED_EMBED_CHUNKS:
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
    torch.save(fine_tuned_embedded_chunks, config.FINE_TUNED_EMBEDDINGS_PATH)
    print(f"Saved embeddings for {len(fine_tuned_embedded_chunks)} files to disk")
else:
    fine_tuned_embedded_chunks = torch.load(config.FINE_TUNED_EMBEDDINGS_PATH,weights_only=False)
    for file in fine_tuned_embedded_chunks:
        if not os.path.exists(file):
            print("Fatal: Path of cached embedding file does not exist")
            exit()
    print("Loaded Cached Embeddings from Disk")

l = list(fine_tuned_embedded_chunks.items())
random.shuffle(l)
fine_tuned_embedded_chunks = dict(l)  

print("-- Step 8: Train Fine Tuned Classifier --")
fine_tuned_classifier = get_classifier(config.FINE_TUNED_CLASSIFIER_PATH)
if config.TRAIN_FINETUNED_CLASSIFIER:
    crypto_embeddings = []
    non_crypto_embeddings = []

    for file in fine_tuned_embedded_chunks:
        if "data/training/crypto" in file:
            for chunk in fine_tuned_embedded_chunks[file]:
                crypto_embeddings.append(chunk["embedding"])
        elif "data/training/non_crypto" in file:
            for chunk in fine_tuned_embedded_chunks[file]:
                non_crypto_embeddings.append(chunk["embedding"])
    fine_tuned_classifier.train(crypto_embeddings, non_crypto_embeddings)
else:
    fine_tuned_classifier.load()

if config.SKIP_FINETUNE:
    fine_tuned_classifier = dirty_classifier
    print("Skipped Finetune: Using Dirty Classifier as per config")


print("-- Step 9: Evaluate Classifier --")
if config.EVALUATE_CLASSIFIER:
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