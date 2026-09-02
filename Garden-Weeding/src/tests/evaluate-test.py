"""
Tests for `garden_weeding.evaluate`.

The `evaluation.evaluate` method returns `(stats, html_content)`:
- `stats` is a metrics dict (thresholds, F1, TP/FP/TN/FN counts).
- `html_content` is a self-contained HTML dashboard string.

All tests use in-memory data structures only; nothing is written to disk.
"""

from __future__ import annotations

import base64
import importlib.util
import json
import re
import sys
from pathlib import Path

import pytest


# --------------------------------------------------------------------------- #
# Load evaluate.py without importing the surrounding package
# --------------------------------------------------------------------------- #
_EVAL_PATH = (
    Path(__file__).resolve().parent.parent / "garden_weeding" / "evaluate.py"
)
_spec = importlib.util.spec_from_file_location("gw_evaluate_under_test", _EVAL_PATH)
evaluate_mod = importlib.util.module_from_spec(_spec)
sys.modules["gw_evaluate_under_test"] = evaluate_mod
_spec.loader.exec_module(evaluate_mod)

evaluation = evaluate_mod.evaluation


# --------------------------------------------------------------------------- #
# Data builders
# --------------------------------------------------------------------------- #
def _file(*probabilities, texts=None):
    """Build the per-file chunk list evaluate() expects."""
    if texts is None:
        texts = [f"chunk {i}" for i in range(len(probabilities))]
    return [
        {"probability": p, "clear_text": t}
        for p, t in zip(probabilities, texts)
    ]


def _extract_payload(html: str) -> dict:
    """Pull the base64-encoded JSON payload out of the generated HTML."""
    m = re.search(
        r'<script[^>]*id="dashboard-payload"[^>]*>\s*([A-Za-z0-9+/=]+)\s*</script>',
        html,
        flags=re.DOTALL,
    )
    assert m is not None, "Base64 payload not found in generated HTML."
    return json.loads(base64.b64decode(m.group(1)).decode("utf-8"))


# --------------------------------------------------------------------------- #
# evaluate() -- perfect separation
# --------------------------------------------------------------------------- #
def test_evaluate_perfect_separation_yields_f1_of_1():
    """When crypto files score 1.0 and non-crypto files score 0.0, there
    exists a threshold that achieves precision=recall=F1=1.0."""
    crypto = {
        "crypto_a.c": _file(1.0, 0.9),
        "crypto_b.c": _file(0.95),
    }
    non_crypto = {
        "plain_a.c": _file(0.0, 0.1),
        "plain_b.c": _file(0.05),
    }
    discarded = {"old_crypto.c": _file(0.0)}

    stats, _ = evaluation().evaluate(
        crypto, non_crypto, discarded, "clear_text", "random_forest"
    )

    assert stats["best_f1_metrics"]["f1"] == pytest.approx(1.0)
    assert stats["best_f1_metrics"]["novel_crypto_tp"] == 2
    assert stats["best_f1_metrics"]["novel_crypto_fn"] == 0
    assert stats["best_f1_metrics"]["non_crypto_unrelated_fp"] == 0
    assert stats["best_f1_metrics"]["non_crypto_discarded_fp"] == 0
    assert 0.005 <= stats["best_f1_threshold"] <= 0.999


# --------------------------------------------------------------------------- #
# evaluate() -- returned structure
# --------------------------------------------------------------------------- #
def test_evaluate_returns_expected_stats_keys():
    stats, _ = evaluation().evaluate(
        {"c.c": _file(0.8)}, {"n.c": _file(0.2)}, {}, "REPR", "CLF"
    )

    expected_top_level = {
        "input_type",
        "classifier_type",
        "best_f1_threshold",
        "best_f1_metrics",
        "min_fn_threshold",
        "min_fn_metrics",
        "min_fp_threshold",
        "min_fp_metrics",
    }
    assert set(stats.keys()) == expected_top_level
    assert stats["input_type"] == "REPR"
    assert stats["classifier_type"] == "CLF"

    for metrics_key in ("best_f1_metrics", "min_fn_metrics", "min_fp_metrics"):
        m = stats[metrics_key]
        for k in (
            "novel_crypto_tp",
            "novel_crypto_fn",
            "non_crypto_unrelated_tn",
            "non_crypto_unrelated_fp",
            "non_crypto_discarded_tn",
            "non_crypto_discarded_fp",
        ):
            assert k in m


# --------------------------------------------------------------------------- #
# evaluate() -- min-FN / min-FP objectives
# --------------------------------------------------------------------------- #
def test_evaluate_min_fn_prefers_low_thresholds():
    """A low threshold catches every crypto file (FN=0)."""
    crypto = {"c1.c": _file(0.3), "c2.c": _file(0.4)}
    non_crypto = {"n1.c": _file(0.9)}
    stats, _ = evaluation().evaluate(crypto, non_crypto, {}, "x", "y")

    assert stats["min_fn_metrics"]["novel_crypto_fn"] == 0
    assert stats["min_fn_threshold"] <= 0.3


def test_evaluate_min_fp_prefers_high_thresholds():
    """A high threshold rejects every non-crypto file (FP=0)."""
    crypto = {"c1.c": _file(0.9)}
    non_crypto = {"n1.c": _file(0.3), "n2.c": _file(0.4)}
    discarded = {"d1.c": _file(0.2)}
    stats, _ = evaluation().evaluate(crypto, non_crypto, discarded, "x", "y")

    total_fp = (
        stats["min_fp_metrics"]["non_crypto_unrelated_fp"]
        + stats["min_fp_metrics"]["non_crypto_discarded_fp"]
    )
    assert total_fp == 0
    assert stats["min_fp_threshold"] > 0.4


# --------------------------------------------------------------------------- #
# evaluate() -- edge cases
# --------------------------------------------------------------------------- #
def test_evaluate_empty_inputs_do_not_crash():
    """With no files at all, evaluate should still return well-formed output."""
    stats, html = evaluation().evaluate({}, {}, {}, "x", "y")

    # With zero inputs every threshold yields TP=FN=FP=0 and F1=0. The first
    # iteration wins the `f1 > best_f1` comparison (best_f1 starts at -1.0),
    # so best_f1_metrics gets populated with an all-zeros counters dict.
    m = stats["best_f1_metrics"]
    assert m["f1"] == 0
    assert m["novel_crypto_tp"] == 0
    assert m["novel_crypto_fn"] == 0
    assert m["non_crypto_unrelated_fp"] == 0
    assert m["non_crypto_discarded_fp"] == 0
    assert isinstance(html, str)


def test_evaluate_file_with_no_chunks_scores_zero():
    """A file whose chunk list is empty produces max-score=0 and therefore
    counts as a false negative for any positive threshold."""
    crypto = {"empty.c": [], "good.c": _file(0.9)}
    stats, _ = evaluation().evaluate(crypto, {}, {}, "x", "y")

    assert stats["best_f1_metrics"]["novel_crypto_tp"] == 1
    assert stats["best_f1_metrics"]["novel_crypto_fn"] == 1


# --------------------------------------------------------------------------- #
# generate_html_report()
# --------------------------------------------------------------------------- #
def test_generate_html_report_embeds_base64_payload():
    """The HTML report must contain a base64 blob whose decoded JSON has the
    expected top-level structure and configuration values."""
    crypto = {"c.c": _file(0.8, texts=["hello\nworld"])}
    non_crypto = {"n.c": _file(0.2)}
    discarded = {"d.c": _file(0.1)}
    _, html = evaluation().evaluate(
        crypto, non_crypto, discarded, "MY_REPR", "MY_CLF"
    )

    assert "MY_REPR" in html
    assert "MY_CLF" in html
    # Placeholders in the template must have been substituted.
    assert "__PAYLOAD_B64__" not in html
    assert "__REPRESENTATION__" not in html
    assert "__CLASSIFIER__" not in html

    payload = _extract_payload(html)
    assert payload["config"]["input_type"] == "MY_REPR"
    assert payload["config"]["classifier_type"] == "MY_CLF"
    assert set(payload["files"].keys()) == {"c.c", "n.c", "d.c"}
    assert payload["files"]["c.c"]["group"] == "novel_crypto"
    assert payload["files"]["n.c"]["group"] == "non_crypto_unrelated"
    assert payload["files"]["d.c"]["group"] == "non_crypto_discarded"
    assert payload["files"]["c.c"]["max_score"] == pytest.approx(0.8)


def test_generate_html_report_joins_list_clear_text():
    r"""When a chunk's clear_text is a list, the report joins with '\n'."""
    crypto = {
        "c.c": [
            {"probability": 0.9, "clear_text": ["line1", "line2"]},
        ]
    }
    _, html = evaluation().evaluate(crypto, {}, {}, "x", "y")

    payload = _extract_payload(html)
    assert payload["files"]["c.c"]["chunks"][0]["clear_text"] == "line1\nline2"

