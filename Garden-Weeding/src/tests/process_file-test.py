"""
Tests for `garden_weeding.process_file`.

We deliberately avoid instantiating the full `file_processor` class, because
its `__init__` downloads a multi-GB HuggingFace model. Instead we test the
self-contained tensor-arithmetic helper `_pool_embeddings`, which contains
the entire "if pooler_output is missing, fall back to attention-masked mean
pooling" fallback logic.

IMPORTANT ON DISK/STATE HYGIENE
-------------------------------
`process_file.py` mutates `os.environ` at *import* time (setting `HF_HOME`,
`TRANSFORMERS_CACHE`, `HF_HUB_OFFLINE`). A session-scoped autouse fixture
snapshots those variables (and the current working directory) before the
first import and restores them afterwards, so importing this test module
does not leak state into the surrounding pytest process.
"""

from __future__ import annotations

import importlib.util
import os
import sys
from pathlib import Path
from types import SimpleNamespace

import pytest
import torch


# --------------------------------------------------------------------------- #
# Snapshot env before importing process_file (which mutates it on import)
# --------------------------------------------------------------------------- #
_ENV_KEYS = ("HF_HOME", "TRANSFORMERS_CACHE", "HF_HUB_OFFLINE")
_env_snapshot = {k: os.environ.get(k) for k in _ENV_KEYS}


def _restore_env():
    for k, v in _env_snapshot.items():
        if v is None:
            os.environ.pop(k, None)
        else:
            os.environ[k] = v


@pytest.fixture(scope="module", autouse=True)
def _preserve_env():
    """Autouse module-scoped fixture: restore env variables after the module
    finishes, no matter what process_file.py did on import."""
    yield
    _restore_env()


# --------------------------------------------------------------------------- #
# Load process_file.py without importing the surrounding package
# --------------------------------------------------------------------------- #
_PF_PATH = (
    Path(__file__).resolve().parent.parent / "garden_weeding" / "process_file.py"
)
_spec = importlib.util.spec_from_file_location("gw_process_file_under_test", _PF_PATH)
process_file_mod = importlib.util.module_from_spec(_spec)
sys.modules["gw_process_file_under_test"] = process_file_mod
_spec.loader.exec_module(process_file_mod)

file_processor = process_file_mod.file_processor


# --------------------------------------------------------------------------- #
# _pool_embeddings
# --------------------------------------------------------------------------- #
def _dummy_instance() -> file_processor:
    """Return an *unconstructed* file_processor so we can call bound
    methods that don't touch self.MODEL / self.TOKENIZER."""
    return file_processor.__new__(file_processor)


def test_pool_embeddings_returns_pooler_output_when_available():
    """If the model produces a `pooler_output`, that tensor is returned
    unchanged (the attention-mask path is skipped entirely)."""
    pooler = torch.tensor([[7.0, 8.0, 9.0]])
    outputs = SimpleNamespace(pooler_output=pooler, last_hidden_state=None)
    attention_mask = torch.tensor([[1, 1, 0]])  # ignored on this path

    result = _dummy_instance()._pool_embeddings(outputs, attention_mask)

    assert torch.equal(result, pooler)


def test_pool_embeddings_falls_back_to_masked_mean_when_pooler_missing():
    """Without a pooler_output, the fallback averages token embeddings
    weighted by the attention mask."""
    # 1 sample, 3 tokens, 2-d embeddings.
    last_hidden = torch.tensor([[[1.0, 2.0], [3.0, 4.0], [999.0, 999.0]]])
    attention_mask = torch.tensor([[1, 1, 0]])  # third token is padding

    outputs = SimpleNamespace(pooler_output=None, last_hidden_state=last_hidden)
    result = _dummy_instance()._pool_embeddings(outputs, attention_mask)

    # Expected: mean of the first two tokens per dim ((1+3)/2, (2+4)/2).
    expected = torch.tensor([[2.0, 3.0]])
    assert torch.allclose(result, expected)


def test_pool_embeddings_all_masked_returns_zeros():
    """When the entire attention mask is zero, the sum_mask clamp (min=1e-9)
    prevents a divide-by-zero and yields a near-zero embedding."""
    last_hidden = torch.tensor([[[1.0, 2.0], [3.0, 4.0]]])
    attention_mask = torch.tensor([[0, 0]])

    outputs = SimpleNamespace(pooler_output=None, last_hidden_state=last_hidden)
    result = _dummy_instance()._pool_embeddings(outputs, attention_mask)

    assert torch.allclose(result, torch.zeros_like(result), atol=1e-6)


def test_pool_embeddings_batch_dimension_preserved():
    """A batch of size 2 in must yield a pooled tensor with batch size 2."""
    last_hidden = torch.tensor(
        [
            [[1.0, 1.0], [1.0, 1.0]],
            [[2.0, 2.0], [4.0, 4.0]],
        ]
    )
    attention_mask = torch.tensor([[1, 1], [1, 1]])
    outputs = SimpleNamespace(pooler_output=None, last_hidden_state=last_hidden)

    result = _dummy_instance()._pool_embeddings(outputs, attention_mask)

    assert result.shape == (2, 2)
    assert torch.allclose(result[0], torch.tensor([1.0, 1.0]))
    assert torch.allclose(result[1], torch.tensor([3.0, 3.0]))


# --------------------------------------------------------------------------- #
# Module-import side-effects
# --------------------------------------------------------------------------- #
def test_import_sets_hf_env_vars():
    """process_file.py sets HF_HUB_OFFLINE=1 at import time; the autouse
    _preserve_env fixture restores the caller's original env after the
    test module finishes."""
    assert os.environ.get("HF_HUB_OFFLINE") == "1"
    assert "HF_HOME" in os.environ
    assert "TRANSFORMERS_CACHE" in os.environ
