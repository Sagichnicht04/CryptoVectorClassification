import os
from pathlib import Path
import torch
import json
import hashlib
import re

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
    all_files = torch.load(embedded_files, weights_only=False)

    exclusion_patterns = load_exclusion_patterns(args)
    if not exclusion_patterns:
        return all_files

    targets = [args.target]
    if args.train:
        targets = [args.positives, args.negatives]

    filtered = {}
    for file_hash, entry in all_files.items():
        path = entry.get("path", "") if isinstance(entry, dict) else ""
        excluded = False
        for target in targets:
            if is_excluded(path, target, exclusion_patterns):
                excluded = True
                break
        if not excluded:
            filtered[file_hash] = entry
    return filtered

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
    exclusion_patterns = load_exclusion_patterns(args)
    uncached_hashes = {}
    targets = [args.target]
    if args.train:
        targets = [args.positives, args.negatives]
    for target in targets:
        for root, _, files in os.walk(target):
            for filename in files:
                path = os.path.join(root, filename)
                if is_excluded(path, target, exclusion_patterns):
                    continue
                lang = get_lang_from_path(path)
                if lang or args.include_non_c_files:
                    hash = get_md5_hash(path)
                    if hash not in embedded_files_hashes:
                        if not args.only_cache:
                            uncached_hashes[hash] = path

    return uncached_hashes








def load_exclusion_patterns(args):
    """Load and compile regex patterns from the exclusion list file.

    The file format is one pattern per line:
      - Blank lines and lines starting with '#' are ignored.
      - Each pattern is a Python regular expression matched against file paths
        relative to the scan target (using forward slashes).
      - Gitignore-style globs work too: '*' and '**/' are converted to regex
        equivalents before compilation.

    Returns a list of ``(compiled_re, match_basename)`` tuples, or an empty
    list when the file does not exist.
    """
    exclusion_file = Path(args.exclusion_list)
    if not exclusion_file.is_file():
        return []

    patterns = []
    with open(exclusion_file, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            patterns.append(_compile_pattern(line))
    return patterns


def _compile_pattern(pattern):
    """Compile a single exclusion pattern string into a regex.

    Returns a ``(compiled_re, match_basename)`` tuple.

    Supports:
      - Plain regex (passed through as-is when it contains regex-specific chars
        like ^, $, +, |, character classes, or grouping).  These are always
        matched against the full relative path (``match_basename=False``).
      - Gitignore-style globs: '**/' matches any number of directories,
        '*' matches anything except '/', '?' matches a single non-'/' char.

    Glob patterns are anchored (wrapped with ``^`` and ``$``) so they match
    the full string, not a substring.  Following gitignore conventions:
      - Patterns containing '/' are matched against the full relative path.
      - Patterns without '/' are matched against the basename only.
    """
    # Detect whether the pattern looks like a gitignore glob or a raw regex.
    # If it contains unescaped regex-only metacharacters (^, $, +, |, grouping,
    # character classes, lookaheads) treat it as raw regex.
    _regex_only = re.compile(r'(?<!\\)[\^$+|(){}\[\]]')
    if _regex_only.search(pattern):
        return (re.compile(pattern), False)

    # Remember whether the original pattern contains a path separator (before
    # we start translating).  Patterns like "*.c" have no slash and should
    # match against the filename only; patterns like "vendor/*" contain a
    # slash and should match against the full relative path.
    match_basename = "/" not in pattern

    # Convert gitignore-style glob to regex.
    regex = ""
    i = 0
    while i < len(pattern):
        c = pattern[i]
        if c == '*' and i + 1 < len(pattern) and pattern[i + 1] == '*':
            # '**' - match everything (including '/')
            if i + 2 < len(pattern) and pattern[i + 2] == '/':
                regex += "(.+/)?"
                i += 3
            else:
                regex += ".*"
                i += 2
        elif c == '*':
            regex += "[^/]*"
            i += 1
        elif c == '?':
            regex += "[^/]"
            i += 1
        elif c == '.':
            regex += r"\."
            i += 1
        else:
            regex += c
            i += 1

    # Anchor so the pattern matches the whole string, not a substring.
    regex = f"^{regex}$"

    return (re.compile(regex), match_basename)


def is_excluded(file_path, base_dir, patterns):
    """Check whether *file_path* matches any exclusion pattern.

    *patterns* is a list of ``(compiled_re, match_basename)`` tuples as
    returned by :func:`_compile_pattern`.

    Matching is performed against the path relative to *base_dir* (using
    forward slashes).  Glob patterns without a '/' in the original text are
    tested against the basename only (like gitignore); glob patterns that
    contained a '/' are tested against the full relative path.  Raw regex
    patterns are always tested against the full relative path.

    Returns True when the file should be skipped.
    """
    if not patterns:
        return False
    try:
        rel = os.path.relpath(file_path, base_dir).replace(os.sep, "/")
    except ValueError:
        # On Windows, relpath raises ValueError for paths on different drives.
        rel = file_path.replace(os.sep, "/")

    basename = os.path.basename(file_path)

    for compiled_re, match_basename in patterns:
        target = basename if match_basename else rel
        if compiled_re.search(target):
            return True
    return False