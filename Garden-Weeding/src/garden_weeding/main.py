import logging
import sys
from .classifier import RandomForestClassifier
from pathlib import Path
from .cache import update_cache
from .cache import get_embedded_files
from .cache import load_uncached_hashes
from .cache import map_training_data
from .argparser import get_args
from .json_report import _write_json_report, _print_report
import time

home = Path.home()
file_path = Path(__file__).parent

log = logging.getLogger("garden_weeding")


def main():
    args = get_args(home, file_path)

    # -- Configure logging ----------------------------------------------------
    level = logging.DEBUG if args.verbose else logging.INFO
    logging.basicConfig(
        level=level,
        format="%(message)s",
        stream=sys.stderr,
    )

    if args.force_gpu and args.force_cpu:
        log.error("Fatal: Can't force gpu and cpu for embedding at the same time.")
        exit(1)
    elif args.strict_threshold and args.rough_threshold:
        log.error("Fatal: Can't use strict and rough threshold at the same time.")
        exit(1)

    threshold = args.threshold
    if args.strict_threshold:
        threshold = 0.22
    elif args.rough_threshold:
        threshold = 0.78

    log.debug("Configuration: %s", args)

    classifier_path = f"{file_path}/../classifier/{args.embedding_model_name.replace("/","--")}.pkl"

    # -- Discover files -------------------------------------------------------
    uncached_hashes = load_uncached_hashes(args)
    embedded_files = get_embedded_files(args)

    cached_count = len(embedded_files)
    uncached_count = len(uncached_hashes)
    log.info("Found %d cached file(s), %d new file(s) to embed.", cached_count, uncached_count)
    log.debug("Uncached files: %s", list(uncached_hashes.values()))

    # -- Embed new files ------------------------------------------------------
    if uncached_count > 0:
        from .process_file import file_processor
        processor = file_processor(args)

        log.debug("Using device: %s", "GPU" if processor.USE_GPU else "CPU")

        embedding_times = [3 if processor.USE_GPU else 120]
        new_embeddings = {}
        for i, uncached_hash in enumerate(uncached_hashes, 1):
            avg_time = sum(embedding_times) / len(embedding_times)
            remaining = int(avg_time * (uncached_count - i + 1))
            path = uncached_hashes[uncached_hash]
            print(
                f"\r  Embedding file {i}/{uncached_count} "
                f"(~{remaining}s remaining): {path}",
                end="", flush=True, file=sys.stderr,
            )
            start = time.time()
            with open(path, "r", encoding='utf-8', errors="replace") as f:
                entry = processor.get_embeddings_for_file(f.read())
                entry["path"] = path
            # Available for classification this run...
            embedded_files[uncached_hash] = entry
            # ...and persisted to the cache (always merged, never completely replaced).
            new_embeddings[uncached_hash] = entry
            embedding_times.append(time.time() - start)

        print(file=sys.stderr)
        update_cache(args, new_embeddings)
        log.info("Embedding complete. Cache updated.")
    # -- Classify or train ----------------------------------------------------
    if not args.train:
        classifier = RandomForestClassifier(args, classifier_path)
        crypto_files = []
        non_crypto_files = []

        for embedded_file in embedded_files:
            entry = embedded_files[embedded_file]
            entry["probabilities"] = classifier.predict_proba(entry["embedding"])
            max_prob = max(entry["probabilities"]) if entry["probabilities"] else 0.0
            is_crypto = any(p >= threshold for p in entry["probabilities"])
            entry["is_crypto"] = is_crypto

            if is_crypto:
                crypto_files.append(entry)
            else:
                non_crypto_files.append(entry)

            log.debug(
                "  %s -> max_prob=%.4f, chunks=%d, crypto=%s, probabilities=%s",
                entry["path"], max_prob, len(entry["probabilities"]),
                is_crypto, entry["probabilities"],
            )

        # -- Print report to stdout -------------------------------------------
        _print_report(crypto_files, non_crypto_files, threshold, args)

        # -- Write machine-readable JSON results ------------------------------
        _write_json_report(crypto_files, non_crypto_files, threshold, args)

    else:
        mapped = map_training_data(args)
        crypto_embeddings = []
        non_crypto_embeddings = []
        excluded = 0
        for hash in mapped["positives"]:
            if hash in embedded_files:
                crypto_embeddings.append(embedded_files[hash]["embedding"])
            else: 
                excluded += 1
        for hash in mapped["negatives"]:
            if hash in embedded_files:
                non_crypto_embeddings.append(embedded_files[hash]["embedding"])
            else:
                excluded += 1
        log.debug(f"Ignored {excluded} files")

        log.info(
            "Training classifier on %d crypto and %d non-crypto file(s)...",
            len(mapped["positives"]), len(mapped["negatives"]),
        )
        classifier = RandomForestClassifier(args, classifier_path)
        classifier.train(crypto_embeddings, non_crypto_embeddings)
        log.info(f"Training complete. Classifier saved to {classifier_path}")



if __name__ == "__main__":
    main()
