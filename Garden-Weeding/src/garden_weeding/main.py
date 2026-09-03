import argparse
import logging
import sys
from .classifier import RandomForestClassifier
from pathlib import Path
from .cache import update_cache
from .cache import get_embedded_files
from .cache import load_uncached_hashes
from .cache import map_training_data
import time

home = Path.home()
file_path = Path(__file__).parent

log = logging.getLogger("garden_weeding")


def main():
    parser = argparse.ArgumentParser(
        prog="garden-weeding",
        description=(
            "Detect cryptographic implementations in source code files.\n\n"
            "Garden-Weeding scans a directory of source files, embeds their "
            "contents using an embedding model, and classifies each file as "
            "cryptographic or non-cryptographic using a trained Random Forest "
            "classifier. Results are printed per file with a confidence score."
        ),
        epilog=(
            "examples:\n"
            "  %(prog)s --target ./src\n"
            "      Scan all C/C++ files in ./src with default settings.\n\n"
            "  %(prog)s --target ./src --strict-threshold --force-gpu\n"
            "      Scan with a strict threshold (0.22) on GPU.\n\n"
            "  %(prog)s --target ./src --rough-threshold --include-non-c-files\n"
            "      Scan all files (not just C/C++) with a rough threshold (0.78).\n\n"
            "  %(prog)s --train --positives ./crypto --negatives ./non-crypto\n"
            "      Train a new classifier from labelled data.\n\n"
            "  %(prog)s --target ./src --no-cache --use-quantization\n"
            "      Re-embed everything from scratch with 4-bit quantization."
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )

    # -- General options ------------------------------------------------------
    general = parser.add_argument_group("general options")
    general.add_argument(
        "--verbose", action="store_true",
        help="enable verbose output and detailed transformer logging",
    )

    # -- Cache control --------------------------------------------------------
    cache = parser.add_argument_group(
        "cache control",
        "Embeddings are cached on disk so unchanged files are not re-embedded.",
    )
    cache.add_argument(
        "--no-cache", action="store_true",
        help="ignore existing cache and re-embed all files",
    )
    cache.add_argument(
        "--only-cache", action="store_true",
        help="only classify already-cached files; skip embedding new ones",
    )
    cache.add_argument(
        "--reset-cache", action="store_true",
        help="delete existing cache files before starting",
    )
    cache.add_argument(
        "--cache-dir", default=f"{home}/.cache/garden_weeding/",
        help="directory for storing embedding cache (default: %(default)s)",
    )

    # -- Embedding model configuration ----------------------------------------
    model = parser.add_argument_group("embedding model configuration")
    model.add_argument(
        "--embedding-model-name",
        default="Alibaba-NLP/gte-Qwen2-1.5B-instruct",
        help="HuggingFace model name or path used for embedding (default: %(default)s)",
    )
    model.add_argument(
        "--token-size", default=4096, type=int,
        help="maximum token chunk size per embedding pass (default: %(default)s)",
    )
    model.add_argument(
        "--chunk-overlap-size", default=512, type=int,
        help="token overlap between consecutive chunks (default: %(default)s)",
    )

    # -- Classifier configuration ---------------------------------------------
    clf = parser.add_argument_group("classifier configuration")
    clf.add_argument(
        "--classifier-file", default=f"{file_path}/classifier.pkl",
        help="path to the pickled Random Forest classifier (default: %(default)s)",
    )

    # -- Hardware control -----------------------------------------------------
    hw = parser.add_argument_group(
        "hardware control",
        "By default the tool auto-detects GPU availability. "
        "Use these flags to override.",
    )
    hw.add_argument(
        "--use-quantization", action="store_true",
        help="enable 4-bit quantization (BitsAndBytes, nf4) to reduce VRAM usage",
    )
    hw.add_argument(
        "--force-gpu", action="store_true",
        help="force GPU usage for embedding (mutually exclusive with --force-cpu)",
    )
    hw.add_argument(
        "--force-cpu", action="store_true",
        help="force CPU usage for embedding (mutually exclusive with --force-gpu)",
    )

    # -- File selection -------------------------------------------------------
    files = parser.add_argument_group("file selection")
    files.add_argument(
        "--include-non-c-files", action="store_true",
        help="process all files, not just .c/.cpp/.cc/.cxx",
    )
    files.add_argument(
        "--exclusion-list", default="./.exclude",
        help=(
            "path to a file with exclusion patterns, one per line. "
            "Supports gitignore-style globs (e.g. '*.txt', '**/vendor/*') "
            "and Python regexes. Lines starting with '#' are comments "
            "(default: %(default)s)"
        ),
    )
    files.add_argument(
        "--target", default="./",
        help="directory to scan for files to classify (default: %(default)s)",
    )

    # -- Threshold control ----------------------------------------------------
    thresh = parser.add_argument_group(
        "threshold control",
        "A file is flagged as cryptographic when any chunk exceeds the threshold. "
        "Lower values catch more crypto (but more false positives).",
    )
    thresh.add_argument(
        "--threshold", default=0.22, type=float,
        help="crypto detection probability threshold (default: %(default)s)",
    )
    thresh.add_argument(
        "--strict-threshold", action="store_true",
        help="use a strict threshold of 0.22 (mutually exclusive with --rough-threshold)",
    )
    thresh.add_argument(
        "--rough-threshold", action="store_true",
        help="use a rough threshold of 0.78 (mutually exclusive with --strict-threshold)",
    )

    # -- Training mode --------------------------------------------------------
    training = parser.add_argument_group(
        "training mode",
        "Train a new Random Forest classifier from labelled crypto / non-crypto files.",
    )
    training.add_argument(
        "--train", action="store_true",
        help="switch to training mode instead of classifying",
    )
    training.add_argument(
        "--positives", default="./positives",
        help="directory containing crypto-positive training samples (default: %(default)s)",
    )
    training.add_argument(
        "--negatives", default="./negatives",
        help="directory containing crypto-negative training samples (default: %(default)s)",
    )

    args = parser.parse_args()

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
                embedded_files[uncached_hash] = processor.get_embeddings_for_file(f.read())
                embedded_files[uncached_hash]["path"] = path
            embedding_times.append(time.time() - start)

        print(file=sys.stderr)  # newline after the progress line
        update_cache(args, embedded_files)
        log.info("Embedding complete. Cache updated.")

    # -- Classify or train ----------------------------------------------------
    if not args.train:
        classifier = RandomForestClassifier(args)
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

    else:
        mapped = map_training_data(args)
        crypto_embeddings = []
        non_crypto_embeddings = []
        for hash in mapped["positives"]:
            crypto_embeddings.append(embedded_files[hash]["embedding"])
        for hash in mapped["negatives"]:
            non_crypto_embeddings.append(embedded_files[hash]["embedding"])

        log.info(
            "Training classifier on %d crypto and %d non-crypto file(s)...",
            len(mapped["positives"]), len(mapped["negatives"]),
        )
        classifier = RandomForestClassifier(args)
        classifier.train(crypto_embeddings, non_crypto_embeddings)
        log.info("Training complete. Classifier saved to %s", args.classifier_file)


def _print_report(crypto_files, non_crypto_files, threshold, args):
    """Print a structured classification report to stdout."""
    total = len(crypto_files) + len(non_crypto_files)

    print("=" * 72)
    print("  GARDEN-WEEDING CLASSIFICATION REPORT")
    print("=" * 72)
    print()
    print(f"  Target:       {args.target}")
    print(f"  Threshold:    {threshold}")
    print(f"  Files scanned: {total}")
    print(f"  Crypto:       {len(crypto_files)}")
    print(f"  Non-crypto:   {len(non_crypto_files)}")
    print()

    if crypto_files:
        print("-" * 72)
        print("  CRYPTOGRAPHIC FILES")
        print("-" * 72)
        for entry in sorted(crypto_files, key=lambda e: -max(e["probabilities"])):
            max_prob = max(entry["probabilities"])
            print(f"    [CRYPTO]  {entry['path']}")
            print(f"              confidence: {max_prob:.4f}  chunks: {len(entry['probabilities'])}")
        print()

    if non_crypto_files:
        print("-" * 72)
        print("  NON-CRYPTOGRAPHIC FILES")
        print("-" * 72)
        for entry in sorted(non_crypto_files, key=lambda e: -max(e["probabilities"]) if e["probabilities"] else 0):
            max_prob = max(entry["probabilities"]) if entry["probabilities"] else 0.0
            print(f"    [NON-CRYPTO]      {entry['path']}")
            log.debug("              max_score: %.4f  chunks: %d", max_prob, len(entry["probabilities"]))
        print()

    print("=" * 72)

    if not crypto_files:
        print("  No cryptographic implementations detected.")
    else:
        print(f"  {len(crypto_files)} file(s) flagged as cryptographic.")
    print("=" * 72)


if __name__ == "__main__":
    main()
