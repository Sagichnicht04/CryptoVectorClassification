import time
import os

DATA_DIR = "./data"

## Steps
PREPARE_DATA = False
CHUNK_DATA = True
BASE_EMBED_CHUNKS = True
TRAIN_DIRTY_CLASSIFIER = True
CLASSIFY_CHUNKS = False
FINETUNE_MODEL = False
FINETUNED_EMBED_CHUNKS = False
TRAIN_FINETUNED_CLASSIFIER = False
EVALUATE_CLASSIFIER = True

SKIP_FINETUNE = True

RANDOM_SEED = 999999

MODEL_NAME = "bigcode/starcoder2-15b"
CLASSIFIER = "random_forest_classifier"
REPRESENTATION = "text"

# Model maximum context lengths mapping
MODEL_CONTEXT_LENGTHS = {
    "microsoft/codebert-base": 512,
    "Salesforce/codet5-small": 512,
    "bigcode/starcoder2-15b": 16384,
    "Qwen/Qwen2.5-7B-Instruct": 32768,
}

TOKEN_SIZE = MODEL_CONTEXT_LENGTHS.get(MODEL_NAME, 512)
OVERLAP_PROPORTION = 0.25
OVERLAP = int(TOKEN_SIZE * OVERLAP_PROPORTION)

NUM_EVAL_FILES = 50
NUM_TRAIN_FILES = 500

USE_MODIFIED_FILES = False
CLASSIFIER_THRESHOLD = 0.95
INTERPRETER = "venv/bin/python3"

# Random Forest Hyperparameters
RF_N_ESTIMATORS = 100
RF_MAX_DEPTH = None
RF_MIN_SAMPLES_LEAF = 1

# Neural Network Hyperparameters
NN_LEARNING_RATE = 0.001
NN_BATCH_SIZE = 32
NN_EPOCHS = 100

MODEL_CLEAN = "starcoder2_15b"
OVERLAP_CLEAN = "overlap_1_4"

CHUNKS_PATH = "cache/text_chunks_starcoder2_15b_overlap_1_4.pt"
BASE_EMBEDDINGS_PATH = "cache/text_base_embeddings_starcoder2_15b_overlap_1_4.pt"

DIRTY_CLASSIFIER_PATH = "results/phase1_model_search/rf_starcoder2-15b/model.pkl"
EVALUATION_RESULT_PATH = "results/phase1_model_search/rf_starcoder2-15b/"

# Standard dummy paths to satisfy workflow references
CHUNK_CLASSIFICATION_PATH = f"cache/text_{CLASSIFIER}_classifications_{MODEL_CLEAN}_{OVERLAP_CLEAN}.pt"
FINE_TUNED_MODEL_DIR = f"cache/text_{CLASSIFIER}_fine_tuned_{MODEL_CLEAN}_{OVERLAP_CLEAN}/"
FINE_TUNED_EMBEDDINGS_PATH = f"cache/text_fine_tuned_embeddings_{MODEL_CLEAN}_{OVERLAP_CLEAN}.pt"
FINE_TUNED_CLASSIFIER_PATH = f"cache/fine_tuned_text_{CLASSIFIER}_{MODEL_CLEAN}_{OVERLAP_CLEAN}.pt"
