import os
from pathlib import Path
import torch
import json
import hashlib

def init_cache(args):
    cache_dir = Path(args.cache_dir)
    embedded_files_hashes = Path.joinpath(cache_dir, f"{get_cache_identifier(args)}_embedded_files_hashes.json")
    embedded_files = Path.joinpath(cache_dir, f"{get_cache_identifier(args)}_embedded_files.pkl")

    if not cache_dir.exists():
        folder = Path(args.cache_dir)
        folder.mkdir(parents=True, exist_ok=True)

    if not embedded_files_hashes.exists() or args.reset_cache:
        with open(embedded_files_hashes, "w") as f:
            f.write("[]")

    if not embedded_files.exists() or args.reset_cache:
        torch.save({}, embedded_files)

def get_md5_hash(file_path):
    hash_md5 = hashlib.md5() 
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()

def get_lang_from_path(path):
    if path.endswith('.c'):
        return 'c'
    if path.endswith(('.cpp', '.cc', '.cxx')):
        return 'cpp'
    return None

def get_cache_identifier(args):
    return hashlib.md5(f"{args.token_size}_{args.chunk_overlap_size}_{args.embedding_model_name}".encode()).hexdigest()

def get_embedded_files_hashes(args):
    cache_dir = Path(args.cache_dir)
    embedded_files_hashes = Path.joinpath(cache_dir, f"{get_cache_identifier(args)}_embedded_files_hashes.json")
    with open(embedded_files_hashes, "r") as f:
        return json.load(f)

def get_embedded_files(args):
    cache_dir = Path(args.cache_dir)
    embedded_files = Path.joinpath(cache_dir, f"{get_cache_identifier(args)}_embedded_files.pkl")
    return torch.load(embedded_files, weights_only=False)

def update_cache(args, new_cache):
    cache_dir = Path(args.cache_dir)
    embedded_files = Path.joinpath(cache_dir, f"{get_cache_identifier(args)}_embedded_files.pkl")
    torch.save(new_cache, embedded_files)
    
    embedded_files_hashes = Path.joinpath(cache_dir, f"{get_cache_identifier(args)}_embedded_files_hashes.json")
    with open(embedded_files_hashes, "w") as f:
        json.dump(list(new_cache.keys()),f)

def map_training_data(args):
    results = {"positives":[], "negatives":[]}

    for root, _, files in os.walk(args.positives):
        for filename in files:
            path = os.path.join(root, filename)
            hash = get_md5_hash(path)
            results["positives"].append(hash)

    for root, _, files in os.walk(args.negatives):
        for filename in files:
            path = os.path.join(root, filename)
            hash = get_md5_hash(path)
            results["negatives"].append(hash)
    return results
        
def load_uncached_hashes(args):
    init_cache(args)

    embedded_files_hashes = get_embedded_files_hashes(args) if not args.no_cache else []
    uncached_hashes = {}
    targets = [args.target]
    if args.train:
        targets = [args.positives, args.negatives]
    for target in targets:
        for root, _, files in os.walk(target):
            for filename in files:
                path = os.path.join(root, filename)
                lang = get_lang_from_path(path)
                if lang or args.include_non_c_files:
                    hash = get_md5_hash(path)
                    if hash not in embedded_files_hashes:
                        if not args.only_cache:
                            uncached_hashes[hash] = path

    return uncached_hashes