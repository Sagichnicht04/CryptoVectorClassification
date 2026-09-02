"""
Tests for `garden_weeding.classifier`.

The classifier is loaded via `importlib` (like `cache.py`) to bypass the
package `__init__.py` that runs `main()` on import.

All on-disk artefacts (`classifier.pkl`) live under pytest's `tmp_path` and
are cleaned up automatically after the test session.
"""

from __future__ import annotations

import argparse
import importlib.util
import pickle
import sys
from pathlib import Path

import numpy as np
import pytest


# --------------------------------------------------------------------------- #
# Load classifier.py without importing the surrounding package
# --------------------------------------------------------------------------- #
_CLF_PATH = (
    Path(__file__).resolve().parent.parent / "garden_weeding" / "classifier.py"
)
_spec = importlib.util.spec_from_file_location("gw_classifier_under_test", _CLF_PATH)
classifier_mod = importlib.util.module_from_spec(_spec)
sys.modules["gw_classifier_under_test"] = classifier_mod
_spec.loader.exec_module(classifier_mod)

RandomForestClassifier = classifier_mod.RandomForestClassifier


# --------------------------------------------------------------------------- #
# Fixtures
# --------------------------------------------------------------------------- #
def _make_args(classifier_file: Path):
    return argparse.Namespace(classifier_file=str(classifier_file))


@pytest.fixture
def rng() -> np.random.Generator:
    """Deterministic random generator so training results are reproducible."""
    return np.random.default_rng(seed=42)


@pytest.fixture
def synthetic_training_data(rng):
    """
    Produce a tiny, linearly-separable training set:
    - "crypto" chunks live around vector (+1, +1, ...);
    - "non-crypto" chunks live around vector (-1, -1, ...).

    Returned in the nested-list-of-lists layout expected by
    RandomForestClassifier.train (one outer entry per file).
    """
    dim = 8
    n_files = 4
    chunks_per_file = 3

    crypto = []
    non_crypto = []
    for _ in range(n_files):
        crypto.append(
            [1.0 + 0.05 * rng.standard_normal(dim) for _ in range(chunks_per_file)]
        )
        non_crypto.append(
            [-1.0 + 0.05 * rng.standard_normal(dim) for _ in range(chunks_per_file)]
        )
    return crypto, non_crypto


@pytest.fixture
def trained_classifier_file(tmp_path, synthetic_training_data):
    """
    Fit a small Random Forest and persist it under tmp_path.

    NOTE about the "chicken-and-egg" flaw in classifier.py's __init__:
    the constructor RAISES if the file does not exist, so we can only
    construct-and-then-train if the file already exists. We work around
    this by pre-creating a placeholder file (matching production usage
    where `--classifier-file` points at a previously-trained model).
    """
    classifier_file = tmp_path / "classifier.pkl"
    # Placeholder so __init__ passes the existence check. Its contents get
    # loaded into self.model but immediately overwritten by .train().
    with open(classifier_file, "wb") as f:
        pickle.dump({"placeholder": True}, f)

    args = _make_args(classifier_file)
    clf = RandomForestClassifier(args)
    crypto, non_crypto = synthetic_training_data
    clf.train(crypto, non_crypto)

    return classifier_file


# --------------------------------------------------------------------------- #
# __init__ tests
# --------------------------------------------------------------------------- #
def test_init_raises_when_file_missing(tmp_path):
    args = _make_args(tmp_path / "does_not_exist.pkl")
    with pytest.raises(RuntimeError, match="not found"):
        RandomForestClassifier(args)


def test_init_raises_on_corrupt_pickle(tmp_path):
    classifier_file = tmp_path / "corrupt.pkl"
    classifier_file.write_bytes(b"this is definitely not a pickle stream")

    args = _make_args(classifier_file)
    with pytest.raises(RuntimeError, match="Error loading"):
        RandomForestClassifier(args)


def test_init_loads_valid_pickle(tmp_path):
    classifier_file = tmp_path / "valid.pkl"
    with open(classifier_file, "wb") as f:
        pickle.dump({"marker": 123}, f)

    args = _make_args(classifier_file)
    clf = RandomForestClassifier(args)
    assert clf.model == {"marker": 123}


# --------------------------------------------------------------------------- #
# train() -- end-to-end fit and persistence
# --------------------------------------------------------------------------- #
def test_train_persists_classifier_to_disk(trained_classifier_file):
    """After training, the pickle at args.classifier_file must contain a
    fitted sklearn RandomForestClassifier (not the placeholder we seeded)."""
    from sklearn.ensemble import RandomForestClassifier as _SkRFC

    with open(trained_classifier_file, "rb") as f:
        loaded = pickle.load(f)

    assert isinstance(loaded, _SkRFC)
    assert hasattr(loaded, "classes_")
    # Labels are 1 and -1 as per classifier.py train()
    assert set(loaded.classes_) == {1, -1}


def test_train_produces_classifier_that_separates_synthetic_data(
    trained_classifier_file, synthetic_training_data
):
    """The trained model should classify the synthetic training points
    correctly: crypto -> high probability, non-crypto -> low probability."""
    args = _make_args(trained_classifier_file)
    clf = RandomForestClassifier(args)

    crypto, non_crypto = synthetic_training_data
    crypto_flat = [emb for file_embs in crypto for emb in file_embs]
    non_crypto_flat = [emb for file_embs in non_crypto for emb in file_embs]

    crypto_probs = clf.predict_proba(crypto_flat)
    non_crypto_probs = clf.predict_proba(non_crypto_flat)

    # Sanity checks on the probabilities themselves.
    for p in crypto_probs:
        assert 0.0 <= p <= 1.0
    for p in non_crypto_probs:
        assert 0.0 <= p <= 1.0

    # On this trivially-separable synthetic data, every crypto point should
    # score higher than every non-crypto point.
    assert min(crypto_probs) > max(non_crypto_probs)


# --------------------------------------------------------------------------- #
# predict_proba() -- shape / edge cases
# --------------------------------------------------------------------------- #
def test_predict_proba_returns_one_probability_per_input(
    trained_classifier_file, synthetic_training_data
):
    args = _make_args(trained_classifier_file)
    clf = RandomForestClassifier(args)

    crypto, _ = synthetic_training_data
    flat = [emb for file_embs in crypto for emb in file_embs]

    probs = clf.predict_proba(flat)
    assert len(probs) == len(flat)


def test_predict_proba_empty_input_returns_empty_ndarray(trained_classifier_file):
    args = _make_args(trained_classifier_file)
    clf = RandomForestClassifier(args)

    result = clf.predict_proba([])
    assert isinstance(result, np.ndarray)
    assert result.size == 0

