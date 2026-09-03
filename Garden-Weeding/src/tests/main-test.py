"""
Tests for `garden_weeding.main`.

Running the full pipeline in-process is impractical (needs the HuggingFace
model + a trained classifier). We therefore only exercise the argument-
validation branches that call `exit(1)` BEFORE any heavy work happens.

These tests spawn subprocesses so that:
- pytest is not killed by main.py's `exit(1)`;
- no on-disk artefacts are created (the two mutually-exclusive flag
  combinations abort before any file I/O).

If a stray file were nevertheless produced, all args are redirected into
pytest's `tmp_path` so cleanup is automatic.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

import pytest


# The src/ directory must be on PYTHONPATH so `python -m garden_weeding` can
# find the package when running under the test subprocess.
_SRC_DIR = str(Path(__file__).resolve().parent.parent)


def _run_main(*extra_args, tmp_path: Path):
    """Invoke garden_weeding via ``python -m`` in a subprocess."""
    env = {**__import__("os").environ, "PYTHONPATH": _SRC_DIR}
    return subprocess.run(
        [
            sys.executable, "-m", "garden_weeding",
            "--cache-dir", str(tmp_path / "cache"),
            "--classifier-file", str(tmp_path / "classifier.pkl"),
            "--target", str(tmp_path / "target"),
            "--positives", str(tmp_path / "positives"),
            "--negatives", str(tmp_path / "negatives"),
            "--exclusion-list", str(tmp_path / ".exclude"),
            *extra_args,
        ],
        capture_output=True,
        text=True,
        timeout=30,
        env=env,
    )


def test_main_rejects_conflicting_gpu_cpu_flags(tmp_path):
    """--force-gpu and --force-cpu together must abort with exit code 1
    and a helpful message, BEFORE any file or model is touched."""
    result = _run_main("--force-gpu", "--force-cpu", tmp_path=tmp_path)

    assert result.returncode == 1
    combined = (result.stdout + result.stderr).lower()
    assert "force gpu and cpu" in combined

    # No cache dir should have been created (aborted before init_cache).
    assert not (tmp_path / "cache").exists()


def test_main_rejects_conflicting_threshold_flags(tmp_path):
    """--strict-threshold and --rough-threshold together must abort."""
    result = _run_main(
        "--strict-threshold", "--rough-threshold", tmp_path=tmp_path
    )

    assert result.returncode == 1
    combined = (result.stdout + result.stderr).lower()
    assert "strict and rough threshold" in combined
    assert not (tmp_path / "cache").exists()


def test_main_help_flag_exits_zero(tmp_path):
    """`--help` is handled by argparse and exits 0 without heavy work."""
    env = {**__import__("os").environ, "PYTHONPATH": _SRC_DIR}
    result = subprocess.run(
        [sys.executable, "-m", "garden_weeding", "--help"],
        capture_output=True,
        text=True,
        timeout=15,
        env=env,
    )
    assert result.returncode == 0
    # argparse writes the help text to stdout.
    assert "--cache-dir" in result.stdout
    assert "--target" in result.stdout
    assert "--train" in result.stdout

    # The program description should be present.
    assert "cryptographic" in result.stdout.lower()

    # Argument groups should be present.
    assert "cache control" in result.stdout.lower()
    assert "hardware control" in result.stdout.lower()
    assert "threshold control" in result.stdout.lower()
    assert "training mode" in result.stdout.lower()
    assert "file selection" in result.stdout.lower()
    assert "embedding model configuration" in result.stdout.lower()

    # Usage examples should appear in the epilog.
    assert "examples:" in result.stdout.lower()


def test_main_rejects_unknown_argument(tmp_path):
    """argparse must reject unknown flags with a non-zero exit code
    and no file-system side effects."""
    result = _run_main("--definitely-not-a-real-flag", tmp_path=tmp_path)
    assert result.returncode != 0
    assert not (tmp_path / "cache").exists()
