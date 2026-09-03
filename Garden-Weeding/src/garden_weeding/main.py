import argparse
from classifier import RandomForestClassifier
from pathlib import Path
from cache import update_cache
from cache import get_embedded_files
from cache import load_uncached_hashes
from cache import map_training_data
import time

home = Path.home()
file_path = Path(__file__).parent


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
        help="path to a file listing paths to exclude (default: %(default)s)",
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
    else:
        mapped = map_training_data(args)
        crypto_embeddings = []
        non_crypto_embeddings = []
        for hash in mapped["positives"]:
            crypto_embeddings.append(embedded_files[hash]["embedding"])
        for hash in mapped["negatives"]:
            non_crypto_embeddings.append(embedded_files[hash]["embedding"])


        classifier = RandomForestClassifier(args)
        classifier.train(crypto_embeddings, non_crypto_embeddings)

if __name__ == "__main__":
    main()