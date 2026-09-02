import argparse
from classifier import RandomForestClassifier
from pathlib import Path
from cache import update_cache
from cache import get_embedded_files
from cache import load_uncached_hashes
import time

home = Path.home()
file_path = Path(__file__).parent


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--no-cache", action="store_true")
    parser.add_argument("--only-cache", action="store_true")
    parser.add_argument("--reset-cache", action="store_true")
    parser.add_argument("--cache-dir", required=False, default=f"{home}/.cache/garden_weeding/")
    parser.add_argument("--embedding-model-name", required=False, default=f"Alibaba-NLP/gte-Qwen2-1.5B-instruct")
    parser.add_argument("--token-size", required=False, default=4096, type=int)
    parser.add_argument("--chunk-overlap-size", required=False, default=512, type=int)

    parser.add_argument("--classifier-file", required=False, default=f"{file_path}/classifier.pkl")


    parser.add_argument("--use-quantization", action="store_true")
    parser.add_argument("--force-gpu", action="store_true")
    parser.add_argument("--force-cpu", action="store_true")
    parser.add_argument("--include-non-c-files", action="store_true")
    parser.add_argument("--exclusion-list", required=False, default="./.exclude")
    parser.add_argument("--target", required=False, default="./")
    parser.add_argument("--threshold", required=False, default=0.22, type=float)
    parser.add_argument("--strict-threshold", required=False, action="store_true")
    parser.add_argument("--rough-threshold", required=False, action="store_true")

    parser.add_argument("--train", action="store_true")
    parser.add_argument("--positives", required=False, default="./positives")
    parser.add_argument("--negatives", required=False, default="./negatives")

    args = parser.parse_args()

    if args.force_gpu and args.force_cpu:
        print("Fatal: Can't force gpu and cpu for embedding at the same time.")
        exit(1)
    elif args.strict_threshold and args.rough_threshold:
        print("Fatal: Can't use strict and rough threshold at the same time.")
        exit(1)

    threshold = args.threshold
    if args.strict_threshold:
        threshold = 0.22
    elif args.rough_threshold:
        threshold = 0.78

    print(args)

    uncached_hashes = load_uncached_hashes(args)
    print(uncached_hashes)

    embedded_files = get_embedded_files(args)


    if len(uncached_hashes) > 0:
        from process_file import file_processor
        processor = file_processor(args)

        embedding_times = [3 if processor.USE_GPU else 120]
        for uncached_hash in uncached_hashes:
            print(f"\rEstimated time left: {int(sum(embedding_times) / len(embedding_times)) * len(uncached_hashes)} seconds. Embedding file {uncached_hashes[uncached_hash]}",end="")
            start = time.time()
            with open(uncached_hashes[uncached_hash], "r", encoding='utf-8', errors="replace") as f:
                embedded_files[uncached_hash] = processor.get_embeddings_for_file(f.read())
                embedded_files[uncached_hash]["path"] = uncached_hashes[uncached_hash]
            embedding_times.append(time.time() - start)
        update_cache(args, embedded_files)


    if args.verbose:
        print("Verbose mode enabled")
    print("\n"*10)
    if not args.train:
        classifier = RandomForestClassifier(args)
        for embedded_file in embedded_files:
            embedded_files[embedded_file]["probabilities"] = classifier.predict_proba(embedded_files[embedded_file]["embedding"])
            is_crypto = False
            print(embedded_files[embedded_file]["probabilities"])
            for probability in embedded_files[embedded_file]["probabilities"]:
                if probability >= threshold:
                    is_crypto = True
            embedded_files[embedded_file]["is_crypto"] = is_crypto
            if is_crypto:
                print(f"{embedded_files[embedded_file]["path"]} is crypto {max(embedded_files[embedded_file]["probabilities"])}")
            else:
                print(f"{embedded_files[embedded_file]["path"]} is not crypto {max(embedded_files[embedded_file]["probabilities"])}")

if __name__ == "__main__":
    main()