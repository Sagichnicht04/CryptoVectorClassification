import os
from pathlib import Path
import torch
import json
import hashlib



def init_cache(args):
    cache_dir = Path(args.cache_dir)
    embedded_files_hashes = Path.joinpath(cache_dir, "embedded_files_hashes.json")
    embedded_files = Path.joinpath(cache_dir, "embedded_files.pkl")

    if not cache_dir.exists():
        folder = Path(args.cache_dir)
        folder.mkdir(parents=True, exist_ok=True)

    if not embedded_files_hashes.exists():
        with open(embedded_files_hashes, "w") as f:
            f.write("[]")

    if not embedded_files.exists():
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

def get_embedded_files_hashes(args):
    cache_dir = Path(args.cache_dir)
    embedded_files_hashes = Path.joinpath(cache_dir, "embedded_files_hashes.json")
    with open(embedded_files_hashes, "r") as f:
        return json.load(f)

def get_uncached_files(args):
    init_cache(args)
    embedded_files_hashes = get_embedded_files_hashes(args)
    all_files = {}

    for root, _, files in os.walk(args.target):
        for filename in files:
            path = os.path.join(root, filename)
            lang = get_lang_from_path(path)
            if lang or args.include_non_c_files:
                hash = get_md5_hash(path)
                if hash not in embedded_files_hashes:
                    all_files[hash] = path
    return all_files