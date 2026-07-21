import os
import re
import base64
import json

def main():
    eval_dir = "Evaluations"
    pattern = re.compile(r"^text_random_forest_classifier_evaluation_(\d+)$")
    
    results = []
    
    if not os.path.exists(eval_dir):
        print(f"Error: {eval_dir} directory not found.")
        return

    for folder in os.listdir(eval_dir):
        match = pattern.match(folder)
        if not match:
            continue
        
        html_path = os.path.join(eval_dir, folder, "evaluation_report.html")
        if not os.path.exists(html_path):
            continue
            
        with open(html_path, "r", encoding="utf-8") as f:
            content = f.read()
            
        # Find the embedded JSON payload
        payload_match = re.search(r'<script id="dashboard-payload" type="application/json">\s*([A-Za-z0-9+/=\s\n]+)\s*</script>', content)
        if not payload_match:
            continue
            
        b64_data = payload_match.group(1).replace("\n", "").replace(" ", "").strip()
        try:
            json_bytes = base64.b64decode(b64_data)
            data = json.loads(json_bytes.decode("utf-8"))
        except Exception:
            continue
            
        stats = data.get("stats", {})
        best_f1_threshold = stats.get("best_f1_threshold")
        best_f1_metrics = stats.get("best_f1_metrics", {})
        
        tp = best_f1_metrics.get("novel_crypto_tp", 0)
        fn = best_f1_metrics.get("novel_crypto_fn", 0)
        fp = best_f1_metrics.get("non_crypto_unrelated_fp", 0) + best_f1_metrics.get("non_crypto_discarded_fp", 0)
        
        precision = tp / (tp + fp) if (tp + fp) > 0 else 0
        recall = tp / (tp + fn) if (tp + fn) > 0 else 0
        f1 = 2 * precision * recall / (precision + recall) if (precision + recall) > 0 else 0
        
        results.append({
            "folder": folder,
            "threshold": best_f1_threshold,
            "f1": f1
        })
        
    if not results:
        print("No evaluation results found.")
        return
        
    # Find the folder with the minimum F1 score
    min_f1_result = min(results, key=lambda x: x["f1"])
    
    print("--- Scan Results ---")
    print(f"Folder with the minimum F1 score: {min_f1_result['folder']}")
    print(f"Best Threshold for this folder:   {min_f1_result['threshold']}")
    print(f"F1 Score at this threshold:       {min_f1_result['f1']:.6f}")
    print("\nAll folders sorted by F1 score (ascending):")
    for r in sorted(results, key=lambda x: x["f1"]):
        print(f"  {r['folder']}: Threshold={r['threshold']:.3f}, F1={r['f1']:.6f}")

if __name__ == "__main__":
    main()