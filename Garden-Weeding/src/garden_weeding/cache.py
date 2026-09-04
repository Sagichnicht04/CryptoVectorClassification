import os
from pathlib import Path
import torch
import json
import hashlib
import re
import logging

log = logging.getLogger("garden_weeding")

def _hashes_path(args):
    """On-disk path of the JSON file listing every cached content hash."""
    cache_dir = Path(args.cache_dir)
    return Path.joinpath(
        cache_dir, f"{get_cache_identifier(args)}_embedded_files_hashes.json"
    )


def _pickle_path(args):
    """On-disk path of the torch pickle holding the embeddings themselves."""
    cache_dir = Path(args.cache_dir)
    return Path.joinpath(
        cache_dir, f"{get_cache_identifier(args)}_embedded_files.pkl"
    )


def init_cache(args):
    cache_dir = Path(args.cache_dir)
    embedded_files_hashes = _hashes_path(args)
    embedded_files = _pickle_path(args)

    if not cache_dir.exists():
        cache_dir.mkdir(parents=True, exist_ok=True)

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

def exceeds_file_size_limit(file_path, limit):
    """Return True if the file at *file_path* is larger than *limit* bytes.

    A *limit* of 0 (or negative) disables the check and always returns False.
    The size is obtained via a single ``stat`` call, so the file contents are
    never read into memory.
    """
    if limit <= 0:
        return False
    try:
        return os.path.getsize(file_path) > limit
    except OSError:
        return False


def get_cache_identifier(args):
    return hashlib.md5(f"{args.token_size}_{args.chunk_overlap_size}_{args.embedding_model_name}".encode()).hexdigest()

def get_embedded_files_hashes(args):
    with open(_hashes_path(args), "r") as f:
        return json.load(f)

def get_embedded_files(args):
    all_files = torch.load(_pickle_path(args), weights_only=False)

    exclusion_patterns = load_exclusion_patterns(args)
    file_size_limit = args.file_size_limit

    targets = [args.target]
    if args.train:
        targets = [args.positives, args.negatives]

    # Resolve target directories to absolute paths for reliable prefix checks.
    resolved_targets = [os.path.realpath(t) for t in targets]

    filtered = {}
    for file_hash, entry in all_files.items():
        path = entry.get("path", "") if isinstance(entry, dict) else ""

        # Skip entries that are not under any active target directory.
        if path:
            real_path = os.path.realpath(path)
            in_target = any(
                real_path == t or real_path.startswith(t + os.sep)
                for t in resolved_targets
            )
            if not in_target:
                continue

        # Skip entries that match an exclusion pattern.
        if exclusion_patterns and path:
            excluded = False
            for target in targets:
                if is_excluded(path, target, exclusion_patterns):
                    excluded = True
                    break
            if excluded:
                continue

        # Skip entries whose file exceeds the byte-size limit.
        if path and exceeds_file_size_limit(path, file_size_limit):
            log.warning("Skipping %s (exceeds %d byte limit)", path, file_size_limit)
            continue

        filtered[file_hash] = entry
    return filtered

def update_cache(args, new_embeddings):
    """Merge freshly-computed embeddings into the on-disk cache.

    *new_embeddings* is a ``{hash: entry}`` mapping for files that were just
    embedded (each *entry* must carry a ``"path"`` key).  Rather than replacing
    the cache wholesale -- which would destroy embeddings for files outside the
    current ``--target`` -- we load the full cache from disk, apply the new
    entries on top, and persist the union.

    Reconciliation guarantees enforced here:

      * If an incoming *hash* already exists in the cache, its stored path is
        refreshed to the new location (requirement 2).
      * Any other cached entry that used to live at one of the incoming paths
        is evicted, because that path now hashes to different content
        (requirement 3).
    """
    # Load the *complete* cache so we never drop entries outside the target.
    full_cache = torch.load(_pickle_path(args), weights_only=False)

    incoming_paths = {
        os.path.realpath(entry["path"])
        for entry in new_embeddings.values()
        if isinstance(entry, dict) and entry.get("path")
    }

    # Evict stale entries whose path now maps to a different hash (req 3).
    incoming_hashes = set(new_embeddings.keys())
    for stale_hash in [
        h for h, e in full_cache.items()
        if h not in incoming_hashes
        and isinstance(e, dict)
        and e.get("path")
        and os.path.realpath(e["path"]) in incoming_paths
    ]:
        log.debug("Evicting stale cache entry %s (path re-hashed)", stale_hash)
        del full_cache[stale_hash]

    # Apply the new / refreshed embeddings (req 2 & 4).
    for file_hash, entry in new_embeddings.items():
        full_cache[file_hash] = entry

    _write_cache(args, full_cache)


def _write_cache(args, cache):
    """Persist *cache* (both the pickle and the hash index) to disk."""
    torch.save(cache, _pickle_path(args))
    with open(_hashes_path(args), "w") as f:
        json.dump(list(cache.keys()), f)

def map_training_data(args):
    log.info("Hashing file-contents. This can take a moment.")

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
        
def _iter_target_files(args):
    """Yield ``(target, path, file_hash)`` for every file eligible for caching.

    Applies, in order, the same gating used everywhere else:
      * language / ``--include-non-c-files`` extension filtering,
      * the exclusion list,
      * the ``--file-size-limit`` byte-size cap.
    """
    exclusion_patterns = load_exclusion_patterns(args)
    file_size_limit = args.file_size_limit
    log.info("Hashing file-contents. This can take a moment.")

    targets = [args.target]
    if args.train:
        targets = [args.positives, args.negatives]

    for target in targets:
        for root, _, files in os.walk(target):
            for filename in files:
                path = os.path.join(root, filename)
                lang = get_lang_from_path(path)
                if not (lang or args.include_non_c_files):
                    continue
                if is_excluded(path, target, exclusion_patterns):
                    continue
                if exceeds_file_size_limit(path, file_size_limit):
                    log.warning(
                        "Skipping %s (exceeds %d byte limit)",
                        path, file_size_limit,
                    )
                    continue
                yield target, path, get_md5_hash(path)

def reconcile_cache(args):
    """Reconcile the on-disk cache against the current state of the target(s).

    Walking every eligible file, it classifies each one and mutates the
    persisted cache so that the four invariants hold:

      1. A file whose *content hash* is already cached is never re-embedded.
      2. A cached file that now lives at a new path has its stored path
         rewritten (no re-embedding -- the content is unchanged).
      3. A cached path whose *content changed* (new hash) has its stale hash
         entry evicted so it can be re-embedded cleanly.
      4. A file that is not cached at all is scheduled for embedding.

    CLI flags respected:
      * ``--no-cache``   -- treat the cache as empty: everything is uncached,
                            nothing is reused, and the reconciled cache is not
                            polluted with path rewrites / evictions.
      * ``--only-cache`` -- never schedule embedding for new files; only path
                            rewrites and evictions of cached entries are
                            applied.
      * ``--reset-cache``-- handled by :func:`init_cache`, which wipes the
                            cache before we reconcile against an empty store.

    Returns a ``{hash: path}`` dict of files that still need embedding.
    """
    init_cache(args)

    # Under --no-cache the on-disk cache is ignored entirely: every eligible
    # file is reported as uncached and no reconciliation is persisted.
    if args.no_cache:
        uncached = {}
        for _target, path, file_hash in _iter_target_files(args):
            uncached[file_hash] = path
        return uncached

    cache = torch.load(_pickle_path(args), weights_only=False)

    # Fast lookup of "which cached hash currently claims this real path".
    path_to_hash = {}
    for h, entry in cache.items():
        if isinstance(entry, dict) and entry.get("path"):
            path_to_hash[os.path.realpath(entry["path"])] = h

    uncached_hashes = {}
    cache_dirty = False

    for _target, path, file_hash in _iter_target_files(args):
        real_path = os.path.realpath(path)

        if file_hash in cache:
            # Requirement 1: content already embedded -- reuse it.
            # Requirement 2: refresh the stored path if the file moved.
            entry = cache[file_hash]
            if isinstance(entry, dict) and os.path.realpath(entry.get("path", "")) != real_path:
                log.debug("Updating cached path for %s: %s -> %s", entry["path"], file_hash, path)
                entry["path"] = path
                cache_dirty = True
            continue

        # Requirement 3: the path is cached but its content changed -- evict
        # the stale hash so the new content gets a clean entry.
        stale_hash = path_to_hash.get(real_path)
        if stale_hash is not None and stale_hash != file_hash:
            log.debug("Content of %s changed; evicting stale hash %s", path, stale_hash)
            del cache[stale_hash]
            path_to_hash.pop(real_path, None)
            cache_dirty = True

        # Requirement 4: schedule embedding unless --only-cache forbids it.
        if not args.only_cache:
            uncached_hashes[file_hash] = path

    if cache_dirty:
        _write_cache(args, cache)

    return uncached_hashes




def load_uncached_hashes(args):
    """Backwards-compatible entry point: reconcile and return files to embed.

    Delegates to :func:`reconcile_cache`, which also persists any path
    rewrites / stale-hash evictions discovered while scanning.
    """
    return reconcile_cache(args)








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
      - Gitignore-style directory patterns: a trailing '/' (e.g. '.git/' or
        'vendor/') matches that directory and everything beneath it.  A bare
        directory name matches at any depth; one containing a '/' is anchored
        to the relative path.

    Glob patterns are anchored (wrapped with ``^`` and ``$``) so they match
    the full string, not a substring.  Following gitignore conventions:
      - Patterns containing '/' (or a trailing '/') are matched against the
        full relative path.
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
    #
    # A *trailing* slash is special: like gitignore, "dir/" means "the
    # directory 'dir' and everything beneath it".  We strip that trailing
    # slash here and, further down, expand the pattern into a subtree match.
    # Such directory patterns are always matched against the full relative
    # path (never the basename), so that the appended "/..." subtree can match.
    is_dir_pattern = pattern.endswith("/") and len(pattern) > 1
    core = pattern[:-1] if is_dir_pattern else pattern

    # A bare directory name like "build/" (no other separator) should match at
    # any depth, mirroring gitignore.  A directory pattern that already
    # contains a separator (e.g. "a/build/") stays anchored to the relative
    # path.
    dir_any_depth = is_dir_pattern and "/" not in core

    match_basename = ("/" not in core) and not is_dir_pattern

    # Convert gitignore-style glob to regex.
    regex = ""
    i = 0
    while i < len(core):
        c = core[i]
        if c == '*' and i + 1 < len(core) and core[i + 1] == '*':
            # '**' - match everything (including '/')
            if i + 2 < len(core) and core[i + 2] == '/':
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

    if is_dir_pattern:
        # Match the directory itself and its entire subtree.  Prepend an
        # optional leading-path segment when the directory name should match
        # at any depth.
        if dir_any_depth:
            regex = f"(.+/)?{regex}"
        regex = f"{regex}(/.*)?"

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