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
def _make_args(cache_dir: Path, target: Path, include_non_c_files: bool = False):
    """Build a minimal argparse.Namespace matching what cache.py reads."""
    return argparse.Namespace(
        cache_dir=str(cache_dir),
        target=str(target),
        include_non_c_files=include_non_c_files,
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
# get_uncached_files -- the main behaviour under test
# --------------------------------------------------------------------------- #
def test_get_uncached_files_returns_all_c_cpp_files_on_empty_cache(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])

    result = cache.get_uncached_files(args)

    # Expected: every C/C++ file, keyed by its md5.
    expected_source_files = {
        p for p in workspace["files"] if cache.get_lang_from_path(str(p))
    }
    assert set(result.values()) == {str(p) for p in expected_source_files}

    # non-source file must NOT be present
    txt_path = workspace["target"] / "notes.txt"
    assert str(txt_path) not in result.values()

    # keys must be md5 hashes of the respective file contents
    for md5, path in result.items():
        assert md5 == cache.get_md5_hash(path)


def test_get_uncached_files_skips_cached_hashes(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])

    # First: compute the md5 of a.c and mark it as already cached.
    a_c = workspace["target"] / "a.c"
    a_c_hash = cache.get_md5_hash(str(a_c))

    cache.init_cache(args)
    hashes_file = workspace["cache_dir"] / "embedded_files_hashes.json"
    hashes_file.write_text(json.dumps([a_c_hash]))

    result = cache.get_uncached_files(args)

    # a.c is now cached -> must NOT appear anymore
    assert a_c_hash not in result
    assert str(a_c) not in result.values()

    # The other C/C++ files should still be returned
    other_sources = {
        str(p)
        for p in workspace["files"]
        if cache.get_lang_from_path(str(p)) and p != a_c
    }
    assert set(result.values()) == other_sources


def test_get_uncached_files_include_non_c_files(workspace):
    args = _make_args(
        workspace["cache_dir"], workspace["target"], include_non_c_files=True
    )

    result = cache.get_uncached_files(args)

    all_files = {str(p) for p in workspace["files"]}
    assert set(result.values()) == all_files


def test_get_uncached_files_auto_initialises_cache(workspace):
    """get_uncached_files calls init_cache internally, so calling it on a
    fresh workspace (no cache dir yet) must still work end-to-end."""
    args = _make_args(workspace["cache_dir"], workspace["target"])
    assert not workspace["cache_dir"].exists()

    result = cache.get_uncached_files(args)

    assert workspace["cache_dir"].is_dir()
    assert (workspace["cache_dir"] / "embedded_files_hashes.json").is_file()
    assert (workspace["cache_dir"] / "embedded_files.pkl").is_file()
    assert len(result) > 0

