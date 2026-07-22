import os
import sys
import time
import json
import shutil
import subprocess
import csv
import traceback

# Backup original config.py
print("Backing up original config.py...")
if os.path.exists("config.py"):
    shutil.copyfile("config.py", "config.py.bak")

# Define target paths
RESULTS_DIR = "results"
os.makedirs(RESULTS_DIR, exist_ok=True)
os.makedirs(os.path.join(RESULTS_DIR, "summaries"), exist_ok=True)

# Parse CLI arguments
is_quick = True # Default to quick mode for easy testing/verification
if "--full" in sys.argv:
    is_quick = False
    print("Running in FULL SEQUENTIAL tuning mode (phased search).")
else:
    print("Running in QUICK tuning mode by default (2 models, 1 overlap, 2 classifiers). Use --full for sequential search.")

# Template for dynamic config.py generation
CONFIG_TEMPLATE = """import time
import os

DATA_DIR = "./data"

## Steps
PREPARE_DATA = False
CHUNK_DATA = {chunk_data}
BASE_EMBED_CHUNKS = {embed_chunks}
TRAIN_DIRTY_CLASSIFIER = {train_classifier}
CLASSIFY_CHUNKS = False
FINETUNE_MODEL = False
FINETUNED_EMBED_CHUNKS = False
TRAIN_FINETUNED_CLASSIFIER = False
EVALUATE_CLASSIFIER = {evaluate_classifier}

SKIP_FINETUNE = True

RANDOM_SEED = {random_seed}

MODEL_NAME = "{model_name}"
CLASSIFIER = "{classifier}"
REPRESENTATION = "text"

# Model maximum context lengths mapping
MODEL_CONTEXT_LENGTHS = {{
    "microsoft/codebert-base": 512,
    "Salesforce/codet5-small": 512,
    "bigcode/starcoder2-15b": 16384,
    "Qwen/Qwen2.5-7B-Instruct": 32768,
}}

TOKEN_SIZE = MODEL_CONTEXT_LENGTHS.get(MODEL_NAME, 512)
OVERLAP_PROPORTION = {overlap_proportion}
OVERLAP = int(TOKEN_SIZE * OVERLAP_PROPORTION)

NUM_EVAL_FILES = {num_eval_files}
NUM_TRAIN_FILES = {num_train_files}

USE_MODIFIED_FILES = False
CLASSIFIER_THRESHOLD = 0.95
INTERPRETER = "venv/bin/python3"

# Random Forest Hyperparameters
RF_N_ESTIMATORS = {rf_n_estimators}
RF_MAX_DEPTH = {rf_max_depth}
RF_MIN_SAMPLES_LEAF = {rf_min_samples_leaf}

# Neural Network Hyperparameters
NN_LEARNING_RATE = {nn_learning_rate}
NN_BATCH_SIZE = {nn_batch_size}
NN_EPOCHS = {nn_epochs}

MODEL_CLEAN = "{model_clean}"
OVERLAP_CLEAN = "{overlap_clean}"

CHUNKS_PATH = "cache/text_chunks_{model_clean}_{overlap_clean}.pt"
BASE_EMBEDDINGS_PATH = "cache/text_base_embeddings_{model_clean}_{overlap_clean}.pt"

DIRTY_CLASSIFIER_PATH = "{dirty_classifier_path}"
EVALUATION_RESULT_PATH = "{evaluation_result_path}"

# Standard dummy paths to satisfy workflow references
CHUNK_CLASSIFICATION_PATH = f"cache/text_{{CLASSIFIER}}_classifications_{{MODEL_CLEAN}}_{{OVERLAP_CLEAN}}.pt"
FINE_TUNED_MODEL_DIR = f"cache/text_{{CLASSIFIER}}_fine_tuned_{{MODEL_CLEAN}}_{{OVERLAP_CLEAN}}/"
FINE_TUNED_EMBEDDINGS_PATH = f"cache/text_fine_tuned_embeddings_{{MODEL_CLEAN}}_{{OVERLAP_CLEAN}}.pt"
FINE_TUNED_CLASSIFIER_PATH = f"cache/fine_tuned_text_{{CLASSIFIER}}_{{MODEL_CLEAN}}_{{OVERLAP_CLEAN}}.pt"
"""

# Helper function to execute a single workflow run
def execute_run(run_id, model, overlap, classifier_type, conf, exp_dir_subpath):
    model_clean = model.split("/")[-1].replace("-", "_").replace(".", "_")
    overlap_clean = f"overlap_1_{int(1/overlap)}" if overlap > 0 else "overlap_none"
    
    chunks_cache_file = f"cache/text_chunks_{model_clean}_{overlap_clean}.pt"
    embeddings_cache_file = f"cache/text_base_embeddings_{model_clean}_{overlap_clean}.pt"
    
    exp_dir = os.path.join(RESULTS_DIR, exp_dir_subpath)
    os.makedirs(exp_dir, exist_ok=True)
    
    config_json_path = os.path.join(exp_dir, "config.json")
    evaluation_json_path = os.path.join(exp_dir, "evaluation.json")
    
    run_info = {
        "experiment_id": run_id,
        "embedding_model": model,
        "chunk_size": 512 if "codebert" in model or "codet5" in model else (16384 if "starcoder2" in model else 32768),
        "overlap_proportion": overlap,
        "overlap": int((512 if "codebert" in model or "codet5" in model else (16384 if "starcoder2" in model else 32768)) * overlap),
        "classifier_type": classifier_type,
        "hyperparameters": conf,
        "directory": exp_dir,
        "status": "pending",
        "runtime": 0.0,
        "metrics": None
    }
    
    # Check if results already exist for resumption
    if os.path.exists(config_json_path) and os.path.exists(evaluation_json_path):
        try:
            with open(config_json_path, "r") as f:
                loaded_info = json.load(f)
            with open(evaluation_json_path, "r") as f:
                loaded_metrics = json.load(f)
            run_info["status"] = loaded_info.get("status", "success")
            run_info["runtime"] = loaded_info.get("runtime", 0.0)
            run_info["metrics"] = loaded_metrics
            print(f"  [Resumed Run {run_id}] F1: {loaded_metrics.get('best_f1_metrics', {}).get('val_f1', 0.0)}")
            return run_info
        except Exception:
            pass
            
    needs_chunking = not os.path.exists(chunks_cache_file)
    needs_embedding = not os.path.exists(embeddings_cache_file)
    
    model_file_ext = "pkl" if classifier_type == "random_forest_classifier" else "pt"
    dirty_classifier_path = os.path.join(exp_dir, f"model.{model_file_ext}")
    
    # Generate config.py content
    config_content = CONFIG_TEMPLATE.format(
        chunk_data="True" if needs_chunking else "False",
        embed_chunks="True" if needs_embedding else "False",
        train_classifier="True",
        evaluate_classifier="True",
        random_seed=999999,  # fixed seed for fair comparison
        model_name=model,
        classifier=classifier_type,
        overlap_proportion=overlap,
        num_eval_files=5 if is_quick else 50,
        num_train_files=10 if is_quick else 500,
        rf_n_estimators=conf.get("n_estimators", 100),
        rf_max_depth=f"'{conf.get('max_depth')}'" if isinstance(conf.get("max_depth"), str) else conf.get("max_depth"),
        rf_min_samples_leaf=conf.get("min_samples_leaf", 1),
        nn_learning_rate=conf.get("learning_rate", 1e-3),
        nn_batch_size=conf.get("batch_size", 32),
        nn_epochs=conf.get("epochs", 100),
        model_clean=model_clean,
        overlap_clean=overlap_clean,
        dirty_classifier_path=dirty_classifier_path,
        evaluation_result_path=exp_dir + "/"
    )
    
    with open("config.py", "w") as f:
        f.write(config_content)
        
    log_file_path = os.path.join(exp_dir, "training.log")
    start_time = time.time()
    
    try:
        with open(log_file_path, "w") as log_file:
            subprocess.run(
                ["venv/bin/python3", "run_workflow.py"],
                stdout=log_file,
                stderr=log_file,
                check=True
            )
        elapsed = time.time() - start_time
        run_info["runtime"] = elapsed
        
        if os.path.exists(evaluation_json_path):
            with open(evaluation_json_path, "r") as f:
                metrics = json.load(f)
            run_info["status"] = "success"
            run_info["metrics"] = metrics
            
            # Compute quick f1 to print
            best_f1_metrics = metrics.get("best_f1_metrics", {})
            tp = best_f1_metrics.get("novel_crypto_tp", 0)
            fn = best_f1_metrics.get("novel_crypto_fn", 0)
            fp = best_f1_metrics.get("non_crypto_unrelated_fp", 0) + best_f1_metrics.get("non_crypto_discarded_fp", 0)
            prec = tp / (tp + fp) if (tp + fp) > 0 else 0.0
            rec = tp / (tp + fn) if (tp + fn) > 0 else 0.0
            f1 = 2 * prec * rec / (prec + rec) if (prec + rec) > 0 else 0.0
            print(f"  [Run {run_id} Success] F1 Score: {f1:.4f} (Runtime: {elapsed:.2f}s)")
        else:
            run_info["status"] = "failed"
            print(f"  [Run {run_id} Failed] evaluation.json was not generated.")
    except Exception as exc:
        elapsed = time.time() - start_time
        run_info["status"] = "failed"
        run_info["runtime"] = elapsed
        print(f"  [Run {run_id} Failed] Error: {exc}")
        with open(os.path.join(exp_dir, "error.log"), "w") as err_f:
            err_f.write(str(exc) + "\n")
            traceback.print_exc(file=err_f)
            
    with open(config_json_path, "w") as f:
        json.dump(run_info, f, indent=2)
        
    return run_info

# Step 1: Prepare data ONCE to freeze train/evaluation split for fair comparison
print("\n=== STEP 1: Preparing dataset once ===")
try:
    subprocess.run("venv/bin/python3 dataset.py prepare", shell=True, check=True)
    print("Dataset prepared successfully.")
except Exception as e:
    print(f"Error preparing dataset: {e}. Proceeding assuming dataset is already prepared.")

# Sequential Search execution
all_runs = []
run_counter = 0

try:
    if is_quick:
        # Quick validation mode (identical to prior quick mode)
        MODELS = ["microsoft/codebert-base", "Salesforce/codet5-small"]
        for m in MODELS:
            # Random Forest
            run_counter += 1
            info = execute_run(run_counter, m, 0.25, "random_forest_classifier", {"n_estimators": 100, "max_depth": 10, "min_samples_leaf": 1}, f"random_forest/{m.split('/')[-1]}/overlap_1_4/quick")
            all_runs.append(info)
            # Neural Network
            run_counter += 1
            info = execute_run(run_counter, m, 0.25, "neural_network_binary_classifier", {"learning_rate": 1e-3, "batch_size": 32, "epochs": 50}, f"neural_network/{m.split('/')[-1]}/overlap_1_4/quick")
            all_runs.append(info)
            
    else:
        # FULL PHASED SEQUENTIAL TUNING WORKFLOW (Optimized as per change suggestions)
        
        # Phase 1: Find Best Embedding Model using standard parameters
        # Standard parameters: overlap = 0.25, RF = {est=100, depth=None, leaf=1}, NN = {lr=1e-3, bs=32, ep=100}
        print("\n=== PHASE 1: Finding Best Embedding Model (using standard parameters) ===")
        PHASE1_MODELS = [
            "microsoft/codebert-base",
            "Salesforce/codet5-small",
            "Qwen/Qwen2.5-7B-Instruct",
            "bigcode/starcoder2-15b"
        ]
        
        model_scores = {}
        
        for m in PHASE1_MODELS:
            print(f"\nEvaluating Model: {m}")
            # RF Run
            run_counter += 1
            rf_info = execute_run(
                run_counter, m, 0.25, "random_forest_classifier", 
                {"n_estimators": 100, "max_depth": None, "min_samples_leaf": 1},
                f"phase1_model_search/rf_{m.split('/')[-1]}"
            )
            all_runs.append(rf_info)
            
            # NN Run
            run_counter += 1
            nn_info = execute_run(
                run_counter, m, 0.25, "neural_network_binary_classifier", 
                {"learning_rate": 1e-3, "batch_size": 32, "epochs": 100},
                f"phase1_model_search/nn_{m.split('/')[-1]}"
            )
            all_runs.append(nn_info)
            
            # Compute average F1 for this model to find the best
            f1s = []
            for info in [rf_info, nn_info]:
                if info["status"] == "success" and info["metrics"]:
                    best_f1_metrics = info["metrics"].get("best_f1_metrics", {})
                    tp = best_f1_metrics.get("novel_crypto_tp", 0)
                    fn = best_f1_metrics.get("novel_crypto_fn", 0)
                    fp = best_f1_metrics.get("non_crypto_unrelated_fp", 0) + best_f1_metrics.get("non_crypto_discarded_fp", 0)
                    prec = tp / (tp + fp) if (tp + fp) > 0 else 0.0
                    rec = tp / (tp + fn) if (tp + fn) > 0 else 0.0
                    f1 = 2 * prec * rec / (prec + rec) if (prec + rec) > 0 else 0.0
                    f1s.append(f1)
            
            if f1s:
                avg_f1 = sum(f1s) / len(f1s)
                model_scores[m] = avg_f1
                print(f"Model {m} Average F1: {avg_f1:.4f}")
            else:
                print(f"Model {m} Failed both standard evaluations.")
                
        if not model_scores:
            print("All models failed Phase 1! Defaulting to 'microsoft/codebert-base'.")
            BEST_MODEL = "microsoft/codebert-base"
        else:
            BEST_MODEL = max(model_scores.keys(), key=lambda k: model_scores[k])
            print(f"\nLocked BEST_MODEL: '{BEST_MODEL}' with Avg F1: {model_scores[BEST_MODEL]:.4f}")
            
        # Phase 2: Find Best Overlap based on Locked Model
        print(f"\n=== PHASE 2: Finding Best Overlap Size for model '{BEST_MODEL}' ===")
        # Standard runs for locked model at 0.25 overlap were already executed in Phase 1!
        # We only need to run overlaps 1/8 (0.125) and 1/2 (0.5)
        overlap_scores = {}
        
        # Extract Phase 1 scores for 0.25 overlap to avoid re-running!
        avg_f1_025 = model_scores.get(BEST_MODEL, 0.0)
        overlap_scores[0.25] = avg_f1_025
        print(f"Loaded existing 0.25 overlap Avg F1: {avg_f1_025:.4f}")
        
        for ov in [0.125, 0.5]:
            print(f"\nEvaluating Overlap: {ov}")
            run_counter += 1
            rf_info = execute_run(
                run_counter, BEST_MODEL, ov, "random_forest_classifier",
                {"n_estimators": 100, "max_depth": None, "min_samples_leaf": 1},
                f"phase2_overlap_search/rf_ov_{int(1/ov) if ov > 0 else 'none'}"
            )
            all_runs.append(rf_info)
            
            run_counter += 1
            nn_info = execute_run(
                run_counter, BEST_MODEL, ov, "neural_network_binary_classifier",
                {"learning_rate": 1e-3, "batch_size": 32, "epochs": 100},
                f"phase2_overlap_search/nn_ov_{int(1/ov) if ov > 0 else 'none'}"
            )
            all_runs.append(nn_info)
            
            f1s = []
            for info in [rf_info, nn_info]:
                if info["status"] == "success" and info["metrics"]:
                    best_f1_metrics = info["metrics"].get("best_f1_metrics", {})
                    tp = best_f1_metrics.get("novel_crypto_tp", 0)
                    fn = best_f1_metrics.get("novel_crypto_fn", 0)
                    fp = best_f1_metrics.get("non_crypto_unrelated_fp", 0) + best_f1_metrics.get("non_crypto_discarded_fp", 0)
                    prec = tp / (tp + fp) if (tp + fp) > 0 else 0.0
                    rec = tp / (tp + fn) if (tp + fn) > 0 else 0.0
                    f1 = 2 * prec * rec / (prec + rec) if (prec + rec) > 0 else 0.0
                    f1s.append(f1)
            if f1s:
                avg_f1 = sum(f1s) / len(f1s)
                overlap_scores[ov] = avg_f1
                print(f"Overlap {ov} Average F1: {avg_f1:.4f}")
                
        BEST_OVERLAP = max(overlap_scores.keys(), key=lambda k: overlap_scores[k])
        print(f"\nLocked BEST_OVERLAP: {BEST_OVERLAP} with Avg F1: {overlap_scores[BEST_OVERLAP]:.4f}")
        
        # Phase 3: Classifier Hyperparameter Fine-tuning based on Locked Model and Overlap
        print(f"\n=== PHASE 3: Fine-Tuning Classifiers based on '{BEST_MODEL}' and overlap {BEST_OVERLAP} ===")
        model_clean = BEST_MODEL.split("/")[-1].replace("-", "_").replace(".", "_")
        overlap_clean = f"overlap_1_{int(1/BEST_OVERLAP)}" if BEST_OVERLAP > 0 else "overlap_none"
        
        # 3a. Fine-tune Random Forest Classifier: 27 configurations
        print("\n--- 3a: Fine-tuning Random Forest (27 combinations) ---")
        rf_configs = []
        for n_est in [100, 300, 500]:
            for depth in [10, 20, None]:
                for leaf in [1, 2, 5]:
                    rf_configs.append({"n_estimators": n_est, "max_depth": depth, "min_samples_leaf": leaf})
                    
        for idx, conf in enumerate(rf_configs, 1):
            run_counter += 1
            param_str = f"est_{conf['n_estimators']}_depth_{conf['max_depth']}_leaf_{conf['min_samples_leaf']}"
            print(f"RF Config {idx}/27: {conf}")
            info = execute_run(
                run_counter, BEST_MODEL, BEST_OVERLAP, "random_forest_classifier", conf,
                f"random_forest/{model_clean}/{overlap_clean}/{param_str}"
            )
            all_runs.append(info)
            
        # 3b. Fine-tune Neural Network Classifier: 48 configurations
        print("\n--- 3b: Fine-tuning Neural Network (48 combinations) ---")
        nn_configs = []
        for lr in [1e-4, 5e-4, 1e-3, 5e-3]:
            for bs in [16, 32, 64]:
                for epochs in [50, 100, 200, 500]:
                    nn_configs.append({"learning_rate": lr, "batch_size": bs, "epochs": epochs})
                    
        for idx, conf in enumerate(nn_configs, 1):
            run_counter += 1
            param_str = f"lr_{conf['learning_rate']}_bs_{conf['batch_size']}_ep_{conf['epochs']}"
            print(f"NN Config {idx}/48: {conf}")
            info = execute_run(
                run_counter, BEST_MODEL, BEST_OVERLAP, "neural_network_binary_classifier", conf,
                f"neural_network/{model_clean}/{overlap_clean}/{param_str}"
            )
            all_runs.append(info)

finally:
    # Always restore original config.py when done
    print("\nRestoring original config.py...")
    if os.path.exists("config.py.bak"):
        shutil.copyfile("config.py.bak", "config.py")
        os.remove("config.py.bak")
    print("Original config.py restored.")

# Step 2: Result Aggregation and Summarization
print("\n=== STEP 2: Aggregating Results ===")
rows = []
successful_runs = []

for run in all_runs:
    hparams = run["hyperparameters"]
    metrics = run["metrics"]
    
    # Flatten parameters
    row = {
        "experiment_id": run["experiment_id"],
        "embedding_model": run["embedding_model"],
        "chunk_size": run["chunk_size"],
        "overlap_proportion": run["overlap_proportion"],
        "overlap": run["overlap"],
        "classifier_type": run["classifier_type"],
        "status": run["status"],
        "runtime_sec": f"{run['runtime']:.2f}"
    }
    
    # Add hyperparameters
    for k, v in hparams.items():
        row[f"param_{k}"] = v
        
    # Populate metrics if successful
    if run["status"] == "success" and metrics:
        successful_runs.append(run)
        
        # Get metrics for Best F1 setup
        best_f1_metrics = metrics.get("best_f1_metrics", {})
        tp = best_f1_metrics.get("novel_crypto_tp", 0)
        fn = best_f1_metrics.get("novel_crypto_fn", 0)
        unrelated_tn = best_f1_metrics.get("non_crypto_unrelated_tn", 0)
        unrelated_fp = best_f1_metrics.get("non_crypto_unrelated_fp", 0)
        discarded_tn = best_f1_metrics.get("non_crypto_discarded_tn", 0)
        discarded_fp = best_f1_metrics.get("non_crypto_discarded_fp", 0)
        
        fp = unrelated_fp + discarded_fp
        tn = unrelated_tn + discarded_tn
        
        precision = tp / (tp + fp) if (tp + fp) > 0 else 0.0
        recall = tp / (tp + fn) if (tp + fn) > 0 else 0.0
        f1 = 2 * precision * recall / (precision + recall) if (precision + recall) > 0 else 0.0
        accuracy = (tp + tn) / (tp + tn + fp + fn) if (tp + tn + fp + fn) > 0 else 0.0
        
        row["val_best_f1_threshold"] = f"{metrics.get('best_f1_threshold', 0.5):.3f}"
        row["val_tp"] = tp
        row["val_fn"] = fn
        row["val_fp"] = fp
        row["val_tn"] = tn
        row["val_precision"] = f"{precision:.4f}"
        row["val_recall"] = f"{recall:.4f}"
        row["val_f1"] = f"{f1:.4f}"
        row["val_accuracy"] = f"{accuracy:.4f}"
        
        # Min FN metrics
        min_fn_metrics = metrics.get("min_fn_metrics", {})
        min_fn_tp = min_fn_metrics.get("novel_crypto_tp", 0)
        min_fn_fn = min_fn_metrics.get("novel_crypto_fn", 0)
        min_fn_fp = min_fn_metrics.get("non_crypto_unrelated_fp", 0) + min_fn_metrics.get("non_crypto_discarded_fp", 0)
        min_fn_recall = min_fn_tp / (min_fn_tp + min_fn_fn) if (min_fn_tp + min_fn_fn) > 0 else 0.0
        row["val_min_fn_threshold"] = f"{metrics.get('min_fn_threshold', 0.5):.3f}"
        row["val_min_fn_fn_count"] = min_fn_fn
        row["val_min_fn_recall"] = f"{min_fn_recall:.4f}"
        
        # Min FP metrics
        min_fp_metrics = metrics.get("min_fp_metrics", {})
        min_fp_fp = min_fp_metrics.get("non_crypto_unrelated_fp", 0) + min_fp_metrics.get("non_crypto_discarded_fp", 0)
        row["val_min_fp_threshold"] = f"{metrics.get('min_fp_threshold', 0.5):.3f}"
        row["val_min_fp_fp_count"] = min_fp_fp
    else:
        row["val_precision"] = "0.0000"
        row["val_recall"] = "0.0000"
        row["val_f1"] = "0.0000"
        row["val_accuracy"] = "0.0000"
        
    rows.append(row)

# Get complete list of headers across all rows
headers = []
for r in rows:
    for k in r.keys():
        if k not in headers:
            headers.append(k)

# Write metrics.csv
metrics_csv_path = os.path.join(RESULTS_DIR, "summaries", "metrics.csv")
with open(metrics_csv_path, "w", newline="") as f:
    writer = csv.DictWriter(f, fieldnames=headers)
    writer.writeheader()
    writer.writerows(rows)
print(f"Saved complete metrics summary to '{metrics_csv_path}'.")

# Write overall ranking.csv (sorted by Best F1 Score)
ranking_headers = [
    "rank", "experiment_id", "embedding_model", "classifier_type", "overlap_proportion", 
    "param_details", "val_f1", "val_precision", "val_recall", "val_accuracy", "runtime_sec", "status"
]

ranking_rows = []
sorted_successful_runs = sorted(
    [r for r in rows if r["status"] == "success"],
    key=lambda x: float(x.get("val_f1", 0.0)),
    reverse=True
)

for rank, r in enumerate(sorted_successful_runs, 1):
    hparams_clean = {k.replace("param_", ""): v for k, v in r.items() if k.startswith("param_")}
    param_details = ", ".join(f"{k}={v}" for k, v in hparams_clean.items())
    
    ranking_rows.append({
        "rank": rank,
        "experiment_id": r["experiment_id"],
        "embedding_model": r["embedding_model"],
        "classifier_type": r["classifier_type"],
        "overlap_proportion": r["overlap_proportion"],
        "param_details": param_details,
        "val_f1": r["val_f1"],
        "val_precision": r["val_precision"],
        "val_recall": r["val_recall"],
        "val_accuracy": r["val_accuracy"],
        "runtime_sec": r["runtime_sec"],
        "status": r["status"]
    })

# Add failed runs at the end
sorted_failed_runs = [r for r in rows if r["status"] != "success"]
for r in sorted_failed_runs:
    hparams_clean = {k.replace("param_", ""): v for k, v in r.items() if k.startswith("param_")}
    param_details = ", ".join(f"{k}={v}" for k, v in hparams_clean.items())
    ranking_rows.append({
        "rank": "N/A",
        "experiment_id": r["experiment_id"],
        "embedding_model": r["embedding_model"],
        "classifier_type": r["classifier_type"],
        "overlap_proportion": r["overlap_proportion"],
        "param_details": param_details,
        "val_f1": "0.0000",
        "val_precision": "0.0000",
        "val_recall": "0.0000",
        "val_accuracy": "0.0000",
        "runtime_sec": r["runtime_sec"],
        "status": r["status"]
    })

ranking_csv_path = os.path.join(RESULTS_DIR, "summaries", "ranking.csv")
with open(ranking_csv_path, "w", newline="") as f:
    writer = csv.DictWriter(f, fieldnames=ranking_headers)
    writer.writeheader()
    writer.writerows(ranking_rows)
print(f"Saved sorted rankings to '{ranking_csv_path}'.")

# Write best_models.csv (best configurations per classifier type)
best_models_csv_path = os.path.join(RESULTS_DIR, "summaries", "best_models.csv")
best_by_classifier = {}
for r in sorted_successful_runs:
    ctype = r["classifier_type"]
    if ctype not in best_by_classifier:
        best_by_classifier[ctype] = r

best_models_rows = []
for idx, (ctype, r) in enumerate(best_by_classifier.items(), 1):
    hparams_clean = {k.replace("param_", ""): v for k, v in r.items() if k.startswith("param_")}
    param_details = ", ".join(f"{k}={v}" for k, v in hparams_clean.items())
    best_models_rows.append({
        "rank": idx,
        "experiment_id": r["experiment_id"],
        "embedding_model": r["embedding_model"],
        "classifier_type": r["classifier_type"],
        "overlap_proportion": r["overlap_proportion"],
        "param_details": param_details,
        "val_f1": r["val_f1"],
        "val_precision": r["val_precision"],
        "val_recall": r["val_recall"],
        "val_accuracy": r["val_accuracy"],
        "runtime_sec": r["runtime_sec"]
    })

best_models_headers = ["rank", "experiment_id", "embedding_model", "classifier_type", "overlap_proportion", "param_details", "val_f1", "val_precision", "val_recall", "val_accuracy", "runtime_sec"]
with open(best_models_csv_path, "w", newline="") as f:
    writer = csv.DictWriter(f, fieldnames=best_models_headers)
    writer.writeheader()
    writer.writerows(best_models_rows)
print(f"Saved best model configurations per classifier to '{best_models_csv_path}'.")

print("\n======================================================================")
print("                       HYPERPARAMETER TUNING SUMMARY                  ")
print("======================================================================")
print(f"Tuned {len(all_runs)} runs in total.")
print(f"Successful runs: {len(successful_runs)}")
print(f"Failed / Skipped runs: {len(all_runs) - len(successful_runs)}")
print("\n--- Top Best Models overall ---")
for idx, r in enumerate(ranking_rows[:5], 1):
    if r["rank"] != "N/A":
        print(f"{idx}. ID {r['experiment_id']} - Model: {r['embedding_model']} - Classifier: {r['classifier_type']} - Overlap: {r['overlap_proportion']} - F1: {r['val_f1']} ({r['param_details']})")
print("======================================================================")