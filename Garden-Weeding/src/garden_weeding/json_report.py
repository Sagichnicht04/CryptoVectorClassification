import json
import logging
log = logging.getLogger("garden_weeding")
from datetime import datetime, timezone


def _write_json_report(crypto_files, non_crypto_files, threshold, args):
    """Write a machine-readable JSON classification report for further analysis.

    The output is a single JSON object with a `metadata` block and a `files`
    list. Only JSON-serializable, analysis-relevant fields are included; the
    raw embeddings (numpy arrays) and chunk token tensors are intentionally
    omitted since they are neither serializable nor useful for downstream
    parsing of classification results.
    """
    output_path = getattr(args, "output_file", "") or ""
    if not output_path:
        log.debug("JSON output disabled (empty --output-file); skipping.")
        return

    def _build_entry(entry):
        # entry["probabilities"] is a list of (possibly numpy) floats, one per chunk.
        probabilities = [float(p) for p in entry.get("probabilities", [])]
        max_confidence = max(probabilities) if probabilities else 0.0
        return {
            "path": entry.get("path"),
            "is_crypto": bool(entry.get("is_crypto", False)),
            "max_confidence": round(max_confidence, 6),
            "num_chunks": len(probabilities),
            "chunk_probabilities": [round(p, 6) for p in probabilities],
        }

    files = [_build_entry(e) for e in crypto_files] + \
            [_build_entry(e) for e in non_crypto_files]
    # Highest-confidence first for convenient downstream inspection.
    files.sort(key=lambda e: e["max_confidence"], reverse=True)

    report = {
        "metadata": {
            "tool": "garden-weeding",
            "generated_at": datetime.now(timezone.utc).isoformat(),
            "target": args.target,
            "threshold": threshold,
            "embedding_model_name": getattr(args, "embedding_model_name", None),
            "token_size": getattr(args, "token_size", None),
            "chunk_overlap_size": getattr(args, "chunk_overlap_size", None),
            "files_scanned": len(crypto_files) + len(non_crypto_files),
            "crypto_count": len(crypto_files),
            "non_crypto_count": len(non_crypto_files),
        },
        "files": files,
    }

    try:
        with open(output_path, "w", encoding="utf-8") as f:
            json.dump(report, f, indent=2, sort_keys=False)
            f.write("\n")
    except OSError as exc:
        log.error("Failed to write JSON results to '%s': %s", output_path, exc)
        return

    log.info("Results written to %s", output_path)


def _print_report(crypto_files, non_crypto_files, threshold, args):
    """Print a structured classification report to stdout."""
    total = len(crypto_files) + len(non_crypto_files)

    print("=" * 72)
    print("  GARDEN-WEEDING CLASSIFICATION REPORT")
    print("=" * 72)
    print()
    print(f"  Target:       {args.target}")
    print(f"  Threshold:    {threshold}")
    print(f"  Files scanned: {total}")
    print(f"  Crypto:       {len(crypto_files)}")
    print(f"  Non-crypto:   {len(non_crypto_files)}")
    print()

    if non_crypto_files:
        print("-" * 72)
        print("  NON-CRYPTOGRAPHIC FILES")
        print("-" * 72)
        for entry in sorted(non_crypto_files, key=lambda e: max(e["probabilities"]) if e["probabilities"] else 0):
            max_prob = max(entry["probabilities"]) if entry["probabilities"] else 0.0
            print(f"    [OK]      {entry['path']}")
            log.debug("              max_score: %.4f  chunks: %d", max_prob, len(entry["probabilities"]))
        print()

    if crypto_files:
        print("-" * 72)
        print("  CRYPTOGRAPHIC FILES")
        print("-" * 72)
        # Ascending confidence: lowest first, highest last.
        for entry in sorted(crypto_files, key=lambda e: max(e["probabilities"])):
            max_prob = max(entry["probabilities"])
            print(f"    [CRYPTO]  {entry['path']}")
            print(f"              confidence: {max_prob:.4f}  chunks: {len(entry['probabilities'])}")
        print()

    print("=" * 72)

    if not crypto_files:
        print("  No cryptographic implementations detected.")
    else:
        print(f"  {len(crypto_files)} file(s) flagged as cryptographic.")
    print("=" * 72)