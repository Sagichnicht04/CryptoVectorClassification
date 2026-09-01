import argparse


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--no-cache", action="store_true")
    parser.add_argument("--only-cache", action="store_true")
    parser.add_argument("--force-gpu", action="store_true")
    parser.add_argument("--force-cpu", action="store_true")
    parser.add_argument("--exclusion-list", required=False, default="./.exclude")
    parser.add_argument("--target", required=False, default="./")

    parser.add_argument("--train", action="store_true")
    parser.add_argument("--positives", required=False, default="./positives")
    parser.add_argument("--negatives", required=False, default="./negatives")

    args = parser.parse_args()



    config = {
        "exclusion-list": args.exclusion_list,
        "verbose": args.verbose,
        "no-cache": args.no_cache,
        "force-gpu": args.force_gpu,
        "force-cpu": args.force_cpu,
        "target": args.target,

    }
    print(args)

    if args.verbose:
        print("Verbose mode enabled")

if __name__ == "__main__":
    main()