import os
import shutil
import random
import argparse
import config

random.seed(config.RANDOM_SEED)


def reset_evaluation_directories():
    """
    Moves all files from the evaluation directories back to their
    original training directories, and clears the modified_crypto dir.
    """
    print("--- Resetting Evaluation Directories ---")
    
    eval_dirs = {
        "novel_crypto": "data/training/crypto",
        "non_crypto": "data/training/non_crypto",
    }

    for eval_dir, train_dir in eval_dirs.items():
        source_dir = f"data/evaluation/{eval_dir}"
        if not os.path.exists(source_dir): continue

        for root, _, files in os.walk(source_dir):
            for filename in files:
                source_path = os.path.join(root, filename)
                rel_path = os.path.relpath(source_path, source_dir)
                dest_path = os.path.join(train_dir, rel_path)
                os.makedirs(os.path.dirname(dest_path), exist_ok=True)
                shutil.move(source_path, dest_path)
        print(f"Moved files from {source_dir} to {train_dir}")
        try:
            shutil.rmtree(source_dir)
        except Exception:
            pass


def get_all_files_recursive(directory):
    all_files = []
    if os.path.exists(directory):
        for root, _, files in os.walk(directory):
            for f in files:
                all_files.append(os.path.join(root, f))
    return all_files

def prepare_evaluation_data():
    """
    Randomly selects files from the training set to create a new
    evaluation set.
    """
        
    reset_evaluation_directories()

    # Ensure evaluation directories exist
    os.makedirs("data/evaluation/novel_crypto", exist_ok=True)
    os.makedirs("data/evaluation/non_crypto", exist_ok=True)
    
    print("\n--- Preparing New Evaluation Set ---")


    # Select novel crypto files for evaluation
    crypto_files = get_all_files_recursive("data/training/crypto")
    eval_crypto_files = random.sample(crypto_files, min(config.NUM_EVAL_FILES, len(crypto_files)))
    for source_path in eval_crypto_files:
        rel_path = os.path.relpath(source_path, "data/training/crypto")
        dest_path = os.path.join("data/evaluation/novel_crypto", rel_path)
        os.makedirs(os.path.dirname(dest_path), exist_ok=True)
        shutil.move(source_path, dest_path)
    print(f"Moved {len(eval_crypto_files)} files to novel_crypto for evaluation.")

    # Select non-crypto files for evaluation (sub-categorized explicitly to avoid design flaws)
    unrelated_files = get_all_files_recursive("data/training/non_crypto/non_crypto_dataset")
    discarded_files = get_all_files_recursive("data/training/non_crypto/discarded_dataset")
    
    eval_unrelated_files = random.sample(unrelated_files, min(config.NUM_EVAL_FILES, len(unrelated_files)))
    eval_discarded_files = random.sample(discarded_files, min(config.NUM_EVAL_FILES, len(discarded_files)))
    
    for source_path in eval_unrelated_files:
        rel_path = os.path.relpath(source_path, "data/training/non_crypto")
        dest_path = os.path.join("data/evaluation/non_crypto", rel_path)
        os.makedirs(os.path.dirname(dest_path), exist_ok=True)
        shutil.move(source_path, dest_path)
        
    for source_path in eval_discarded_files:
        rel_path = os.path.relpath(source_path, "data/training/non_crypto")
        dest_path = os.path.join("data/evaluation/non_crypto", rel_path)
        os.makedirs(os.path.dirname(dest_path), exist_ok=True)
        shutil.move(source_path, dest_path)
        
    print(f"Moved {len(eval_unrelated_files)} unrelated and {len(eval_discarded_files)} discarded files to non_crypto for evaluation.")
    

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('action')  
    args = parser.parse_args()
    if args.action == "clear":
        reset_evaluation_directories()
    elif args.action == "prepare":
        prepare_evaluation_data()
