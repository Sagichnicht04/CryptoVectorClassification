import argparse
from inference import inference
from pathlib import Path
home = Path.home()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--no-cache", action="store_true")
    parser.add_argument("--only-cache", action="store_true")
    parser.add_argument("--cache-dir", required=False, default=f"{home}/.cache/garden_weeding/")
    parser.add_argument("--embedding-model-name", required=False, default=f"Alibaba-NLP/gte-Qwen2-1.5B-instruct")
    parser.add_argument("--token-size", required=False, default="4096")
    parser.add_argument("--chunk-overlap-size", required=False, default="512")

    parser.add_argument("--use-quantization", action="store_true")
    parser.add_argument("--force-gpu", action="store_true")
    parser.add_argument("--force-cpu", action="store_true")
    parser.add_argument("--include-non-c-files", action="store_true")
    parser.add_argument("--exclusion-list", required=False, default="./.exclude")
    parser.add_argument("--target", required=False, default="./")
    parser.add_argument("--threshold", required=False, default="0.22")
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

    print(args)

    if args.verbose:
        print("Verbose mode enabled")
    if not args.train:
        inference(args)

if __name__ == "__main__":
    main()