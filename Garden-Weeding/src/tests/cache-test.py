"""
Tests for `garden_weeding.cache`.

We deliberately load `cache.py` directly by file path (via `importlib`) instead
of doing `from garden_weeding.cache import ...`. The reason is that the
package's `src/garden_weeding/__init__.py` calls `main()` on import, which
parses `sys.argv` and would break under pytest. Loading the module file
directly bypasses the package `__init__` entirely.

Run with:  pytest    (from the Garden-Weeding project root)
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import sys
from pathlib import Path

import pytest
import torch


# --------------------------------------------------------------------------- #
# Load cache.py without importing the surrounding package
# --------------------------------------------------------------------------- #
_CACHE_PATH = (
    Path(__file__).resolve().parent.parent / "garden_weeding" / "cache.py"
)
_spec = importlib.util.spec_from_file_location("gw_cache_under_test", _CACHE_PATH)
cache = importlib.util.module_from_spec(_spec)
sys.modules["gw_cache_under_test"] = cache
_spec.loader.exec_module(cache)


# --------------------------------------------------------------------------- #
# Helpers / fixtures
# --------------------------------------------------------------------------- #
def _make_args(
    cache_dir: Path,
    target: Path,
    include_non_c_files: bool = False,
    no_cache: bool = False,
    only_cache: bool = False,
):
    """Build a minimal argparse.Namespace matching what cache.py reads."""
    return argparse.Namespace(
        cache_dir=str(cache_dir),
        target=str(target),
        include_non_c_files=include_non_c_files,
        no_cache=no_cache,
        only_cache=only_cache,
    )


@pytest.fixture
def workspace(tmp_path: Path):
    """
    Layout created on disk (under pytest's tmp_path):

        <tmp>/cache/                      (empty; init_cache will populate)
        <tmp>/target/a.c
        <tmp>/target/b.cpp
        <tmp>/target/sub/c.cc
        <tmp>/target/sub/d.cxx
        <tmp>/target/notes.txt            (non-source file)
    """
    cache_dir = tmp_path / "cache"
    target = tmp_path / "target"
    (target / "sub").mkdir(parents=True)

    files = {
        target / "a.c":           b"int main(void) { return 0; }\n",
        target / "b.cpp":         b"int main() { return 1; }\n",
        target / "sub" / "c.cc":  b"// cc file\nint x = 2;\n",
        target / "sub" / "d.cxx": b"// cxx file\nint y = 3;\n",
        target / "notes.txt":     b"just some notes, not a source file\n",
    }
    for p, content in files.items():
        p.write_bytes(content)

    return {
        "tmp_path": tmp_path,
        "cache_dir": cache_dir,
        "target": target,
        "files": files,
    }


# --------------------------------------------------------------------------- #
# Small, focused tests for the pure helpers
# --------------------------------------------------------------------------- #
@pytest.mark.parametrize(
    "path, expected",
    [
        ("foo.c", "c"),
        ("foo.cpp", "cpp"),
        ("foo.cc", "cpp"),
        ("foo.cxx", "cpp"),
        ("foo.txt", None),
        ("foo", None),
        ("foo.h", None),  # header files are not classified by cache.py
    ],
)
def test_get_lang_from_path(path, expected):
    assert cache.get_lang_from_path(path) == expected


def test_get_md5_hash_matches_hashlib(tmp_path: Path):
    payload = b"hello world, this is some file content\n" * 10
    f = tmp_path / "sample.bin"
    f.write_bytes(payload)

    expected = hashlib.md5(payload).hexdigest()
    assert cache.get_md5_hash(str(f)) == expected


# --------------------------------------------------------------------------- #
# init_cache
# --------------------------------------------------------------------------- #
def test_init_cache_creates_files(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])
    assert not workspace["cache_dir"].exists()

    cache.init_cache(args)

    hashes_file = workspace["cache_dir"] / "embedded_files_hashes.json"
    embedded_file = workspace["cache_dir"] / "embedded_files.pkl"

    assert workspace["cache_dir"].is_dir()
    assert hashes_file.is_file()
    assert embedded_file.is_file()

    # hashes JSON must be a valid empty list
    assert json.loads(hashes_file.read_text()) == []

    # pickle must be a valid torch-saved empty dict
    assert torch.load(embedded_file, weights_only=False) == {}


def test_init_cache_is_idempotent(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])
    cache.init_cache(args)

    hashes_file = workspace["cache_dir"] / "embedded_files_hashes.json"

    # Seed the JSON with a fake hash and make sure a second init_cache call
    # does *not* overwrite it.
    hashes_file.write_text(json.dumps(["deadbeef"]))
    cache.init_cache(args)
    assert json.loads(hashes_file.read_text()) == ["deadbeef"]


# --------------------------------------------------------------------------- #
# get_embedded_files_hashes
# --------------------------------------------------------------------------- #
def test_get_embedded_files_hashes_reads_json(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])
    cache.init_cache(args)

    assert cache.get_embedded_files_hashes(args) == []

    hashes_file = workspace["cache_dir"] / "embedded_files_hashes.json"
    hashes_file.write_text(json.dumps(["aa", "bb", "cc"]))

    assert cache.get_embedded_files_hashes(args) == ["aa", "bb", "cc"]


# --------------------------------------------------------------------------- #
# get_embedded_files (reads the torch-pickled dict of embeddings)
# --------------------------------------------------------------------------- #
def test_get_embedded_files_reads_pickle(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])
    cache.init_cache(args)

    # Freshly-initialised cache: pickle contains an empty dict.
    assert cache.get_embedded_files(args) == {}

    # Overwrite with a non-trivial payload and make sure we get it back.
    embedded_file = workspace["cache_dir"] / "embedded_files.pkl"
    payload = {"deadbeef": torch.tensor([1.0, 2.0, 3.0])}
    torch.save(payload, embedded_file)

    loaded = cache.get_embedded_files(args)
    assert set(loaded.keys()) == {"deadbeef"}
    assert torch.equal(loaded["deadbeef"], payload["deadbeef"])


# --------------------------------------------------------------------------- #
# load_target_hashes -- the main behaviour under test
#
# Returns a tuple (uncached_hashes, cached_hashes), each a dict {md5: path}.
# Honours --no-cache (treat cache as empty) and --only-cache (don't emit
# uncached entries).
# --------------------------------------------------------------------------- #
def _c_cpp_source_paths(workspace):
    return {
        str(p)
        for p in workspace["files"]
        if cache.get_lang_from_path(str(p))
    }


def test_load_target_hashes_all_uncached_on_empty_cache(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])

    uncached, cached = cache.load_target_hashes(args)

    # Every C/C++ file is uncached; the non-source .txt is filtered out.
    assert set(uncached.values()) == _c_cpp_source_paths(workspace)
    assert cached == {}

    txt_path = workspace["target"] / "notes.txt"
    assert str(txt_path) not in uncached.values()

    # keys must be md5 hashes of the respective file contents
    for md5, path in uncached.items():
        assert md5 == cache.get_md5_hash(path)


def test_load_target_hashes_partitions_cached_and_uncached(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])

    # Mark a.c as already cached.
    a_c = workspace["target"] / "a.c"
    a_c_hash = cache.get_md5_hash(str(a_c))

    cache.init_cache(args)
    hashes_file = workspace["cache_dir"] / "embedded_files_hashes.json"
    hashes_file.write_text(json.dumps([a_c_hash]))

    uncached, cached = cache.load_target_hashes(args)

    # a.c should now appear on the cached side, not the uncached side.
    assert cached == {a_c_hash: str(a_c)}
    assert a_c_hash not in uncached
    assert str(a_c) not in uncached.values()

    # The other C/C++ files should still show up as uncached.
    expected_uncached = _c_cpp_source_paths(workspace) - {str(a_c)}
    assert set(uncached.values()) == expected_uncached


def test_load_target_hashes_include_non_c_files(workspace):
    args = _make_args(
        workspace["cache_dir"], workspace["target"], include_non_c_files=True
    )

    uncached, cached = cache.load_target_hashes(args)

    all_files = {str(p) for p in workspace["files"]}
    assert set(uncached.values()) == all_files
    assert cached == {}


def test_load_target_hashes_no_cache_flag_ignores_existing_cache(workspace):
    """With --no-cache, previously cached hashes must be treated as uncached."""
    args_seed = _make_args(workspace["cache_dir"], workspace["target"])

    # Seed the cache with a.c's hash.
    a_c = workspace["target"] / "a.c"
    a_c_hash = cache.get_md5_hash(str(a_c))

    cache.init_cache(args_seed)
    hashes_file = workspace["cache_dir"] / "embedded_files_hashes.json"
    hashes_file.write_text(json.dumps([a_c_hash]))

    # Now request with no_cache=True.
    args = _make_args(workspace["cache_dir"], workspace["target"], no_cache=True)
    uncached, cached = cache.load_target_hashes(args)

    # Everything C/C++ should be uncached; nothing should end up in cached.
    assert set(uncached.values()) == _c_cpp_source_paths(workspace)
    assert cached == {}


def test_load_target_hashes_only_cache_flag_suppresses_uncached(workspace):
    """With --only-cache, uncached files must not be reported at all."""
    args_seed = _make_args(workspace["cache_dir"], workspace["target"])

    a_c = workspace["target"] / "a.c"
    a_c_hash = cache.get_md5_hash(str(a_c))

    cache.init_cache(args_seed)
    hashes_file = workspace["cache_dir"] / "embedded_files_hashes.json"
    hashes_file.write_text(json.dumps([a_c_hash]))

    args = _make_args(
        workspace["cache_dir"], workspace["target"], only_cache=True
    )
    uncached, cached = cache.load_target_hashes(args)

    # Only the seeded hash comes back, on the cached side.
    assert uncached == {}
    assert cached == {a_c_hash: str(a_c)}


def test_load_target_hashes_auto_initialises_cache(workspace):
    """load_target_hashes calls init_cache internally, so calling it on a
    fresh workspace (no cache dir yet) must still work end-to-end."""
    args = _make_args(workspace["cache_dir"], workspace["target"])
    assert not workspace["cache_dir"].exists()

    uncached, cached = cache.load_target_hashes(args)

    assert workspace["cache_dir"].is_dir()
    assert (workspace["cache_dir"] / "embedded_files_hashes.json").is_file()
    assert (workspace["cache_dir"] / "embedded_files.pkl").is_file()
    assert len(uncached) > 0
    assert cached == {}

