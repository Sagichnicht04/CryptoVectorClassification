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
    exclusion_list: str | Path = "/nonexistent/.exclude",
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
        exclusion_list=str(exclusion_list),
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


# --------------------------------------------------------------------------- #
# _compile_pattern
# --------------------------------------------------------------------------- #
class TestCompilePattern:
    """Unit tests for the glob-to-regex conversion helper.

    _compile_pattern returns a (compiled_re, match_basename) tuple.
    In these unit tests we extract the regex with [0] and test matching
    directly.  The match_basename flag is tested via is_excluded().
    """

    def test_literal_filename(self):
        pat, basename = cache._compile_pattern("notes.txt")
        assert pat.search("notes.txt")
        assert not pat.search("other.txt")
        assert basename is True  # no '/' in pattern

    def test_star_matches_filename(self):
        pat, basename = cache._compile_pattern("*.txt")
        assert pat.search("notes.txt")
        assert pat.search("readme.txt")
        assert not pat.search("notes.c")
        assert basename is True

    def test_star_does_not_cross_slash(self):
        pat, _ = cache._compile_pattern("*.txt")
        # '*' anchored: matches exact basename "notes.txt"
        assert pat.search("notes.txt")
        assert pat.search("readme.txt")
        assert not pat.search("notes.c")
        # '*' should not match a slash within the name
        assert not pat.search("sub/notes.txt")

    def test_basename_pattern_matched_via_is_excluded(self, tmp_path):
        """Patterns without '/' match basenames, so they apply at any depth
        when used through is_excluded (which extracts the basename)."""
        patterns = [cache._compile_pattern("test_*")]
        assert cache.is_excluded(
            str(tmp_path / "test_foo"), str(tmp_path), patterns
        )
        # Matches nested file by basename too (gitignore convention).
        assert cache.is_excluded(
            str(tmp_path / "sub" / "test_foo"), str(tmp_path), patterns
        )

    def test_doublestar_slash_matches_any_depth(self):
        pat, basename = cache._compile_pattern("**/test_*")
        assert pat.search("test_foo.c")
        assert pat.search("a/test_foo.c")
        assert pat.search("a/b/c/test_foo.c")
        assert basename is False  # pattern contains '/'

    def test_doublestar_without_slash(self):
        pat, basename = cache._compile_pattern("vendor/**")
        assert pat.search("vendor/lib.c")
        assert pat.search("vendor/sub/lib.c")
        assert basename is False

    def test_question_mark(self):
        pat, _ = cache._compile_pattern("file?.c")
        assert pat.search("file1.c")
        assert pat.search("fileA.c")
        assert not pat.search("file12.c")

    def test_dot_is_literal(self):
        pat, _ = cache._compile_pattern("*.c")
        assert pat.search("foo.c")
        # The dot shouldn't match arbitrary characters
        assert not pat.search("fooXc")

    def test_raw_regex_passthrough(self):
        """Patterns with regex-specific metacharacters are used as-is."""
        pat, basename = cache._compile_pattern(r"^vendor/.*\.c$")
        assert pat.search("vendor/foo.c")
        assert not pat.search("other/foo.c")
        assert not pat.search("vendor/foo.txt")
        assert basename is False  # raw regex always matches full path


# --------------------------------------------------------------------------- #
# load_exclusion_patterns
# --------------------------------------------------------------------------- #
class TestLoadExclusionPatterns:

    def test_returns_empty_when_file_missing(self, tmp_path):
        args = _make_args(
            tmp_path / "cache", tmp_path / "target",
            exclusion_list=tmp_path / "nonexistent_file",
        )
        assert cache.load_exclusion_patterns(args) == []

    def test_loads_patterns_from_file(self, tmp_path):
        exclude_file = tmp_path / ".exclude"
        exclude_file.write_text("*.txt\nvendor/**\n")
        args = _make_args(
            tmp_path / "cache", tmp_path / "target",
            exclusion_list=exclude_file,
        )
        patterns = cache.load_exclusion_patterns(args)
        assert len(patterns) == 2

    def test_ignores_comments_and_blank_lines(self, tmp_path):
        exclude_file = tmp_path / ".exclude"
        exclude_file.write_text("# this is a comment\n\n*.txt\n  \n# another\nvendor/**\n")
        args = _make_args(
            tmp_path / "cache", tmp_path / "target",
            exclusion_list=exclude_file,
        )
        patterns = cache.load_exclusion_patterns(args)
        assert len(patterns) == 2

    def test_empty_file_returns_empty_list(self, tmp_path):
        exclude_file = tmp_path / ".exclude"
        exclude_file.write_text("")
        args = _make_args(
            tmp_path / "cache", tmp_path / "target",
            exclusion_list=exclude_file,
        )
        assert cache.load_exclusion_patterns(args) == []


# --------------------------------------------------------------------------- #
# is_excluded
# --------------------------------------------------------------------------- #
class TestIsExcluded:

    def test_no_patterns_never_excludes(self):
        assert not cache.is_excluded("/some/path/foo.c", "/some/path", [])

    def test_glob_star_matches_extension(self, tmp_path):
        patterns = [cache._compile_pattern("*.txt")]
        assert cache.is_excluded(
            str(tmp_path / "notes.txt"), str(tmp_path), patterns
        )
        assert not cache.is_excluded(
            str(tmp_path / "main.c"), str(tmp_path), patterns
        )

    def test_doublestar_matches_nested(self, tmp_path):
        patterns = [cache._compile_pattern("**/secret_*")]
        assert cache.is_excluded(
            str(tmp_path / "a" / "b" / "secret_key.c"), str(tmp_path), patterns
        )
        assert cache.is_excluded(
            str(tmp_path / "secret_key.c"), str(tmp_path), patterns
        )
        assert not cache.is_excluded(
            str(tmp_path / "public_key.c"), str(tmp_path), patterns
        )

    def test_directory_glob(self, tmp_path):
        patterns = [cache._compile_pattern("vendor/*")]
        assert cache.is_excluded(
            str(tmp_path / "vendor" / "lib.c"), str(tmp_path), patterns
        )
        assert not cache.is_excluded(
            str(tmp_path / "src" / "lib.c"), str(tmp_path), patterns
        )

    def test_regex_pattern(self, tmp_path):
        patterns = [cache._compile_pattern(r"^test[0-9]+\.c$")]
        assert cache.is_excluded(
            str(tmp_path / "test123.c"), str(tmp_path), patterns
        )
        assert not cache.is_excluded(
            str(tmp_path / "test.c"), str(tmp_path), patterns
        )

    def test_multiple_patterns_any_match_excludes(self, tmp_path):
        patterns = [
            cache._compile_pattern("*.txt"),
            cache._compile_pattern("**/vendor/*"),
        ]
        assert cache.is_excluded(
            str(tmp_path / "readme.txt"), str(tmp_path), patterns
        )
        assert cache.is_excluded(
            str(tmp_path / "a" / "vendor" / "lib.c"), str(tmp_path), patterns
        )
        assert not cache.is_excluded(
            str(tmp_path / "src" / "main.c"), str(tmp_path), patterns
        )


# --------------------------------------------------------------------------- #
# load_uncached_hashes with exclusion list integration
# --------------------------------------------------------------------------- #
def test_load_uncached_hashes_excludes_by_glob_pattern(workspace):
    """Files matching an exclusion pattern should not appear in results."""
    exclude_file = workspace["tmp_path"] / ".exclude"
    exclude_file.write_text("*.c\n")

    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        exclusion_list=exclude_file,
    )
    result = cache.load_uncached_hashes(args)

    # a.c should be excluded; b.cpp, c.cc, d.cxx should remain
    for path in result.values():
        assert not path.endswith(".c"), f"{path} should have been excluded"
    assert len(result) == 3  # b.cpp, c.cc, d.cxx


def test_load_uncached_hashes_excludes_directory_glob(workspace):
    """Patterns like 'sub/*' should exclude all files under that directory."""
    exclude_file = workspace["tmp_path"] / ".exclude"
    exclude_file.write_text("sub/*\n")

    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        exclusion_list=exclude_file,
    )
    result = cache.load_uncached_hashes(args)

    for path in result.values():
        assert "/sub/" not in path and "\\sub\\" not in path
    # a.c and b.cpp remain; c.cc and d.cxx (in sub/) are excluded
    assert len(result) == 2


def test_load_uncached_hashes_excludes_with_regex(workspace):
    """Raw regex patterns should also work in the exclusion file."""
    exclude_file = workspace["tmp_path"] / ".exclude"
    # Exclude files whose names start with 'a' or 'b'
    exclude_file.write_text(r"^[ab]\." + "\n")

    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        exclusion_list=exclude_file,
    )
    result = cache.load_uncached_hashes(args)

    filenames = {Path(p).name for p in result.values()}
    assert "a.c" not in filenames
    assert "b.cpp" not in filenames
    # c.cc and d.cxx should remain
    assert len(result) == 2


def test_load_uncached_hashes_no_exclusion_file_excludes_nothing(workspace):
    """When the exclusion list file doesn't exist, all files are included."""
    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        exclusion_list=workspace["tmp_path"] / "does_not_exist",
    )
    result = cache.load_uncached_hashes(args)
    assert set(result.values()) == _c_cpp_source_paths(workspace)


def test_load_uncached_hashes_exclusion_with_comments_and_blanks(workspace):
    """Comments and blank lines in the exclusion file must be ignored."""
    exclude_file = workspace["tmp_path"] / ".exclude"
    exclude_file.write_text("# Exclude text files\n\n*.c\n\n# done\n")

    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        exclusion_list=exclude_file,
    )
    result = cache.load_uncached_hashes(args)

    for path in result.values():
        assert not path.endswith(".c")
    assert len(result) == 3  # b.cpp, c.cc, d.cxx


def test_load_uncached_hashes_exclusion_works_in_train_mode(workspace):
    """Exclusion patterns should also apply in --train mode."""
    exclude_file = workspace["tmp_path"] / ".exclude"
    exclude_file.write_text("*.txt\n*.md\n")

    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        train=True,
        include_non_c_files=True,
        positives=workspace["positives"],
        negatives=workspace["negatives"],
        exclusion_list=exclude_file,
    )
    result = cache.load_uncached_hashes(args)

    # pos2.txt and neg2.md should be excluded; pos1.c and neg1.cpp remain
    filenames = {Path(p).name for p in result.values()}
    assert "pos2.txt" not in filenames
    assert "neg2.md" not in filenames
    assert "pos1.c" in filenames
    assert "neg1.cpp" in filenames
    assert len(result) == 2


# --------------------------------------------------------------------------- #
# get_embedded_files with exclusion list (cached files must also be filtered)
# --------------------------------------------------------------------------- #
def test_get_embedded_files_excludes_cached_entries_by_glob(workspace):
    """Cached files whose path matches an exclusion pattern must not be
    returned by get_embedded_files, even though they remain in the pickle."""
    exclude_file = workspace["tmp_path"] / ".exclude"
    exclude_file.write_text("*.txt\n")

    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        exclusion_list=exclude_file,
    )
    cache.init_cache(args)

    payload = {
        "aa": {"embedding": [1, 2, 3], "path": str(workspace["target"] / "a.c")},
        "bb": {"embedding": [4, 5, 6], "path": str(workspace["target"] / "notes.txt")},
    }
    cache.update_cache(args, payload)

    loaded = cache.get_embedded_files(args)

    # notes.txt should be excluded, a.c should remain.
    assert "aa" in loaded
    assert "bb" not in loaded
    assert len(loaded) == 1


def test_get_embedded_files_excludes_cached_entries_by_directory(workspace):
    """Patterns like 'sub/*' must exclude cached files under that directory."""
    exclude_file = workspace["tmp_path"] / ".exclude"
    exclude_file.write_text("sub/*\n")

    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        exclusion_list=exclude_file,
    )
    cache.init_cache(args)

    payload = {
        "aa": {"embedding": [1], "path": str(workspace["target"] / "a.c")},
        "bb": {"embedding": [2], "path": str(workspace["target"] / "sub" / "c.cc")},
        "cc": {"embedding": [3], "path": str(workspace["target"] / "sub" / "d.cxx")},
    }
    cache.update_cache(args, payload)

    loaded = cache.get_embedded_files(args)

    assert "aa" in loaded
    assert "bb" not in loaded
    assert "cc" not in loaded
    assert len(loaded) == 1


def test_get_embedded_files_no_exclusion_file_returns_all(workspace):
    """When the exclusion file doesn't exist, all cached entries are returned."""
    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        exclusion_list=workspace["tmp_path"] / "does_not_exist",
    )
    cache.init_cache(args)

    payload = {
        "aa": {"embedding": [1], "path": str(workspace["target"] / "a.c")},
        "bb": {"embedding": [2], "path": str(workspace["target"] / "notes.txt")},
    }
    cache.update_cache(args, payload)

    loaded = cache.get_embedded_files(args)
    assert len(loaded) == 2
    assert set(loaded.keys()) == {"aa", "bb"}


def test_get_embedded_files_exclusion_applies_in_train_mode(workspace):
    """In --train mode, cached files matching exclusion patterns must also be
    filtered out based on the positives/negatives directories."""
    exclude_file = workspace["tmp_path"] / ".exclude"
    exclude_file.write_text("*.txt\n*.md\n")

    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        train=True,
        include_non_c_files=True,
        positives=workspace["positives"],
        negatives=workspace["negatives"],
        exclusion_list=exclude_file,
    )
    cache.init_cache(args)

    payload = {
        "aa": {"embedding": [1], "path": str(workspace["positives"] / "pos1.c")},
        "bb": {"embedding": [2], "path": str(workspace["positives"] / "pos2.txt")},
        "cc": {"embedding": [3], "path": str(workspace["negatives"] / "neg1.cpp")},
        "dd": {"embedding": [4], "path": str(workspace["negatives"] / "neg2.md")},
    }
    cache.update_cache(args, payload)

    loaded = cache.get_embedded_files(args)

    assert "aa" in loaded   # pos1.c  - not excluded
    assert "bb" not in loaded  # pos2.txt - excluded
    assert "cc" in loaded   # neg1.cpp - not excluded
    assert "dd" not in loaded  # neg2.md  - excluded
    assert len(loaded) == 2


def test_get_embedded_files_empty_cache_with_exclusion(workspace):
    """Empty cache returns empty dict even when exclusion patterns exist."""
    exclude_file = workspace["tmp_path"] / ".exclude"
    exclude_file.write_text("*.c\n")

    args = _make_args(
        workspace["cache_dir"],
        workspace["target"],
        exclusion_list=exclude_file,
    )
    cache.init_cache(args)

    assert cache.get_embedded_files(args) == {}

