"""
Tests for `garden_weeding.cache`.

We load `cache.py` directly by file path (via `importlib`) instead of doing
`from garden_weeding.cache import ...`, because `src/garden_weeding/__init__.py`
calls `main()` on import which would parse `sys.argv` under pytest.

Every filesystem side effect uses pytest's `tmp_path` fixture, so no on-disk
state persists beyond the test run.

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
    *,
    include_non_c_files: bool = False,
    no_cache: bool = False,
    only_cache: bool = False,
    reset_cache: bool = False,
    train: bool = False,
    positives: Path | None = None,
    negatives: Path | None = None,
    token_size: int = 4096,
    chunk_overlap_size: int = 512,
    embedding_model_name: str = "test-model",
):
    """Build a minimal argparse.Namespace matching what cache.py reads."""
    return argparse.Namespace(
        cache_dir=str(cache_dir),
        target=str(target),
        include_non_c_files=include_non_c_files,
        no_cache=no_cache,
        only_cache=only_cache,
        reset_cache=reset_cache,
        train=train,
        positives=str(positives) if positives is not None else str(target),
        negatives=str(negatives) if negatives is not None else str(target),
        token_size=token_size,
        chunk_overlap_size=chunk_overlap_size,
        embedding_model_name=embedding_model_name,
    )


def _identifier(args) -> str:
    return cache.get_cache_identifier(args)


def _hashes_json(cache_dir: Path, args) -> Path:
    return cache_dir / f"{_identifier(args)}_embedded_files_hashes.json"


def _pickle_file(cache_dir: Path, args) -> Path:
    return cache_dir / f"{_identifier(args)}_embedded_files.pkl"


@pytest.fixture
def workspace(tmp_path: Path):
    """
    Layout created on disk under pytest's `tmp_path` (auto-cleaned):

        <tmp>/cache/                       (empty; init_cache will populate)
        <tmp>/target/a.c
        <tmp>/target/b.cpp
        <tmp>/target/sub/c.cc
        <tmp>/target/sub/d.cxx
        <tmp>/target/notes.txt             (non-source file)
        <tmp>/positives/pos1.c
        <tmp>/positives/pos2.txt
        <tmp>/negatives/neg1.cpp
        <tmp>/negatives/neg2.md
    """
    cache_dir = tmp_path / "cache"
    target = tmp_path / "target"
    positives = tmp_path / "positives"
    negatives = tmp_path / "negatives"
    (target / "sub").mkdir(parents=True)
    positives.mkdir()
    negatives.mkdir()

    files = {
        target / "a.c":           b"int main(void) { return 0; }\n",
        target / "b.cpp":         b"int main() { return 1; }\n",
        target / "sub" / "c.cc":  b"// cc file\nint x = 2;\n",
        target / "sub" / "d.cxx": b"// cxx file\nint y = 3;\n",
        target / "notes.txt":     b"just some notes, not a source file\n",
    }
    training_files = {
        positives / "pos1.c":   b"crypto_positive_a\n",
        positives / "pos2.txt": b"crypto_positive_b\n",
        negatives / "neg1.cpp": b"crypto_negative_a\n",
        negatives / "neg2.md":  b"crypto_negative_b\n",
    }
    for p, content in {**files, **training_files}.items():
        p.write_bytes(content)

    return {
        "tmp_path": tmp_path,
        "cache_dir": cache_dir,
        "target": target,
        "positives": positives,
        "negatives": negatives,
        "files": files,
        "training_files": training_files,
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
# get_cache_identifier
# --------------------------------------------------------------------------- #
def test_get_cache_identifier_deterministic(workspace):
    a = _make_args(workspace["cache_dir"], workspace["target"])
    b = _make_args(workspace["cache_dir"], workspace["target"])
    assert cache.get_cache_identifier(a) == cache.get_cache_identifier(b)


def test_get_cache_identifier_matches_manual_hash(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])
    expected = hashlib.md5(
        f"{args.token_size}_{args.chunk_overlap_size}_{args.embedding_model_name}".encode()
    ).hexdigest()
    assert cache.get_cache_identifier(args) == expected


@pytest.mark.parametrize(
    "field, value",
    [
        ("token_size", 2048),
        ("chunk_overlap_size", 256),
        ("embedding_model_name", "different-model"),
    ],
)
def test_get_cache_identifier_changes_with_config(workspace, field, value):
    base = _make_args(workspace["cache_dir"], workspace["target"])
    other = _make_args(workspace["cache_dir"], workspace["target"], **{field: value})
    assert cache.get_cache_identifier(base) != cache.get_cache_identifier(other)


# --------------------------------------------------------------------------- #
# init_cache
# --------------------------------------------------------------------------- #
def test_init_cache_creates_files(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])
    assert not workspace["cache_dir"].exists()

    cache.init_cache(args)

    hashes_file = _hashes_json(workspace["cache_dir"], args)
    embedded_file = _pickle_file(workspace["cache_dir"], args)

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

    hashes_file = _hashes_json(workspace["cache_dir"], args)

    # Seed the JSON with a fake hash; a second init_cache call must NOT
    # overwrite it (because reset_cache is False).
    hashes_file.write_text(json.dumps(["deadbeef"]))
    cache.init_cache(args)
    assert json.loads(hashes_file.read_text()) == ["deadbeef"]


def test_init_cache_reset_flag_overwrites_existing_cache(workspace):
    """With --reset-cache, previously seeded cache files must be wiped clean."""
    args = _make_args(workspace["cache_dir"], workspace["target"])
    cache.init_cache(args)

    hashes_file = _hashes_json(workspace["cache_dir"], args)
    embedded_file = _pickle_file(workspace["cache_dir"], args)

    # Populate with non-empty content.
    hashes_file.write_text(json.dumps(["cafefade"]))
    torch.save({"cafefade": "some payload"}, embedded_file)

    reset_args = _make_args(
        workspace["cache_dir"], workspace["target"], reset_cache=True
    )
    cache.init_cache(reset_args)

    assert json.loads(hashes_file.read_text()) == []
    assert torch.load(embedded_file, weights_only=False) == {}


def test_init_cache_uses_identifier_prefixed_filenames(workspace):
    """Different embedding configs must produce different on-disk cache files."""
    args_a = _make_args(workspace["cache_dir"], workspace["target"])
    args_b = _make_args(
        workspace["cache_dir"], workspace["target"], token_size=2048
    )
    cache.init_cache(args_a)
    cache.init_cache(args_b)

    a_hash = _hashes_json(workspace["cache_dir"], args_a)
    b_hash = _hashes_json(workspace["cache_dir"], args_b)

    assert a_hash != b_hash
    assert a_hash.is_file()
    assert b_hash.is_file()



# --------------------------------------------------------------------------- #
# get_embedded_files_hashes & get_embedded_files
# --------------------------------------------------------------------------- #
def test_get_embedded_files_hashes_reads_json(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])
    cache.init_cache(args)

    assert cache.get_embedded_files_hashes(args) == []

    hashes_file = _hashes_json(workspace["cache_dir"], args)
    hashes_file.write_text(json.dumps(["aa", "bb", "cc"]))

    assert cache.get_embedded_files_hashes(args) == ["aa", "bb", "cc"]


def test_get_embedded_files_reads_pickle(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])
    cache.init_cache(args)

    # Freshly-initialised cache: pickle contains an empty dict.
    assert cache.get_embedded_files(args) == {}

    # Overwrite with a non-trivial payload and make sure we get it back.
    embedded_file = _pickle_file(workspace["cache_dir"], args)
    payload = {"deadbeef": torch.tensor([1.0, 2.0, 3.0])}
    torch.save(payload, embedded_file)

    loaded = cache.get_embedded_files(args)
    assert set(loaded.keys()) == {"deadbeef"}
    assert torch.equal(loaded["deadbeef"], payload["deadbeef"])


# --------------------------------------------------------------------------- #
# update_cache
# --------------------------------------------------------------------------- #
def test_update_cache_persists_pickle_and_hashes(workspace):
    """update_cache must write both the pickle AND the JSON hashes list,
    keeping them in sync via list(new_cache.keys())."""
    args = _make_args(workspace["cache_dir"], workspace["target"])
    cache.init_cache(args)

    payload = {
        "aa": {"embedding": [1, 2, 3], "path": "/somewhere/a.c"},
        "bb": {"embedding": [4, 5, 6], "path": "/somewhere/b.cpp"},
    }
    cache.update_cache(args, payload)

    # Pickle survived the round-trip.
    assert cache.get_embedded_files(args) == payload

    # Hashes JSON reflects the pickle's keys (order-independent).
    assert set(cache.get_embedded_files_hashes(args)) == {"aa", "bb"}


def test_update_cache_overwrites_previous_contents(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])
    cache.init_cache(args)

    cache.update_cache(args, {"aa": {"embedding": [1]}})
    cache.update_cache(args, {"bb": {"embedding": [2]}})

    assert cache.get_embedded_files(args) == {"bb": {"embedding": [2]}}
    assert cache.get_embedded_files_hashes(args) == ["bb"]



# --------------------------------------------------------------------------- #
# map_training_data
# --------------------------------------------------------------------------- #
def test_map_training_data_hashes_all_files_regardless_of_extension(workspace):
    """map_training_data walks positives/negatives and hashes every file,
    with NO extension filtering."""
    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        positives=workspace["positives"],
        negatives=workspace["negatives"],
    )
    result = cache.map_training_data(args)

    assert set(result.keys()) == {"positives", "negatives"}

    expected_pos = {
        cache.get_md5_hash(str(p)) for p in workspace["positives"].iterdir()
    }
    expected_neg = {
        cache.get_md5_hash(str(p)) for p in workspace["negatives"].iterdir()
    }
    assert set(result["positives"]) == expected_pos
    assert set(result["negatives"]) == expected_neg

    # 2 positives + 2 negatives created in the workspace fixture.
    assert len(result["positives"]) == 2
    assert len(result["negatives"]) == 2


def test_map_training_data_empty_dirs_return_empty_lists(tmp_path):
    positives = tmp_path / "pos"
    negatives = tmp_path / "neg"
    positives.mkdir()
    negatives.mkdir()

    args = _make_args(
        tmp_path / "cache",
        tmp_path / "target",
        positives=positives,
        negatives=negatives,
    )
    result = cache.map_training_data(args)
    assert result == {"positives": [], "negatives": []}



# --------------------------------------------------------------------------- #
# load_uncached_hashes -- the main behaviour under test
#
# Returns a single dict {md5: path} of files that still need embedding.
# Honours --no-cache (treat cache as empty), --only-cache (return {}),
# --train (walk positives/negatives instead of target),
# --include-non-c-files (drop the extension filter).
# --------------------------------------------------------------------------- #
def _c_cpp_source_paths(workspace):
    return {
        str(p)
        for p in workspace["files"]
        if cache.get_lang_from_path(str(p))
    }


def test_load_uncached_hashes_all_uncached_on_empty_cache(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])

    result = cache.load_uncached_hashes(args)

    # Every C/C++ file is uncached; the non-source .txt is filtered out.
    assert set(result.values()) == _c_cpp_source_paths(workspace)

    txt_path = workspace["target"] / "notes.txt"
    assert str(txt_path) not in result.values()

    # keys must be md5 hashes of the respective file contents
    for md5, path in result.items():
        assert md5 == cache.get_md5_hash(path)


def test_load_uncached_hashes_skips_already_cached(workspace):
    args = _make_args(workspace["cache_dir"], workspace["target"])

    a_c = workspace["target"] / "a.c"
    a_c_hash = cache.get_md5_hash(str(a_c))

    cache.init_cache(args)
    hashes_file = _hashes_json(workspace["cache_dir"], args)
    hashes_file.write_text(json.dumps([a_c_hash]))

    result = cache.load_uncached_hashes(args)

    assert a_c_hash not in result
    assert str(a_c) not in result.values()

    expected = _c_cpp_source_paths(workspace) - {str(a_c)}
    assert set(result.values()) == expected


def test_load_uncached_hashes_include_non_c_files(workspace):
    args = _make_args(
        workspace["cache_dir"], workspace["target"], include_non_c_files=True
    )
    result = cache.load_uncached_hashes(args)

    all_files = {str(p) for p in workspace["files"]}
    assert set(result.values()) == all_files



def test_load_uncached_hashes_no_cache_flag_ignores_existing_cache(workspace):
    """With --no-cache, previously cached hashes must be treated as uncached."""
    args_seed = _make_args(workspace["cache_dir"], workspace["target"])

    a_c = workspace["target"] / "a.c"
    a_c_hash = cache.get_md5_hash(str(a_c))

    cache.init_cache(args_seed)
    hashes_file = _hashes_json(workspace["cache_dir"], args_seed)
    hashes_file.write_text(json.dumps([a_c_hash]))

    args = _make_args(workspace["cache_dir"], workspace["target"], no_cache=True)
    result = cache.load_uncached_hashes(args)

    assert set(result.values()) == _c_cpp_source_paths(workspace)


def test_load_uncached_hashes_only_cache_flag_returns_empty(workspace):
    """With --only-cache, load_uncached_hashes returns an empty dict:
    the caller should rely on get_embedded_files() for the cached data."""
    args = _make_args(
        workspace["cache_dir"], workspace["target"], only_cache=True
    )
    assert cache.load_uncached_hashes(args) == {}


def test_load_uncached_hashes_train_mode_walks_positives_and_negatives(workspace):
    """In --train mode, args.target is ignored; positives + negatives are walked."""
    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        train=True,
        include_non_c_files=True,  # training data has non-C extensions.
        positives=workspace["positives"],
        negatives=workspace["negatives"],
    )
    result = cache.load_uncached_hashes(args)

    expected_paths = {str(p) for p in workspace["training_files"]}
    assert set(result.values()) == expected_paths

    # None of the target-tree paths appear.
    for target_file in workspace["files"]:
        assert str(target_file) not in result.values()


def test_load_uncached_hashes_auto_initialises_cache(workspace):
    """load_uncached_hashes calls init_cache internally, so calling it on a
    fresh workspace (no cache dir yet) must still work end-to-end."""
    args = _make_args(workspace["cache_dir"], workspace["target"])
    assert not workspace["cache_dir"].exists()

    result = cache.load_uncached_hashes(args)

    assert workspace["cache_dir"].is_dir()
    assert _hashes_json(workspace["cache_dir"], args).is_file()
    assert _pickle_file(workspace["cache_dir"], args).is_file()
    assert len(result) > 0


