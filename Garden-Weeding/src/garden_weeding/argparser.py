import argparse

def get_args(home, file_path):
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
    general.add_argument(
        "--output-file", default="garden-weeding-results.json",
        help=(
            "path to write machine-readable JSON classification results "
            "for further analysis. Set to an empty string to disable "
            "(default: %(default)s)"
        ),
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
        "--exclusion-list", default="",
        help=(
            "path to a file with exclusion patterns, one per line. "
            "Supports gitignore-style globs (e.g. '*.txt', '**/vendor/*') "
            "and Python regexes. Lines starting with '#' are comments "
            "(default: %(default)s)"
        ),
    )
    files.add_argument(
        "--file-size-limit", default=200000, type=int,
        help=(
            "maximum number of bytes a file may have to be included in "
            "the analysis. Files exceeding this limit are skipped. "
            "Set to 0 to disable the limit (default: %(default)s)"
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
    return args