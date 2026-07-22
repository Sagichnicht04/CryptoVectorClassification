import time
import os

# Maybe important
#sudo sh -c "echo '0000:01:00.0' > /sys/bus/pci/drivers/nvidia/unbind"

DATA_DIR = "./data"

## Steps
PREPARE_DATA = True
CHUNK_DATA = False
BASE_EMBED_CHUNKS = False
TRAIN_DIRTY_CLASSIFIER = True
CLASSIFY_CHUNKS = False
FINETUNE_MODEL = False
FINETUNED_EMBED_CHUNKS = False
TRAIN_FINETUNED_CLASSIFIER = False
EVALUATE_CLASSIFIER = True

SKIP_FINETUNE = True

t = time.localtime()
fmt_time = time.strftime("%d%H%M%S", t)
RANDOM_SEED = int(fmt_time)
#RANDOM_SEED = 999999


# Add models from Hugging Face or local paths.
MODELS = [
    "microsoft/codebert-base",
    "Qwen/Qwen2.5-Coder-14B",
    "Qwen/Qwen2.5-Coder-14B-Instruct",
    "bigcode/starcoder2-15b",
    "Qwen/Qwen2.5-14B-Instruct",
    "Qwen/Qwen2.5-7B-Instruct",
    "arcee-ai/SuperNova-Medius",
    "deepseek-ai/deepseek-coder-6.7b-instruct",
    "microsoft/codebert-base-text",
    "microsoft/codebert-base-graph",
    "microsoft/codebert-base-math",
    "/home/flo/codebert-local",
    "Salesforce/codet5-small",
]

# The pre-trained model to use from Hugging Face
MODEL_NAME = "microsoft/codebert-base"

# Either neural_network_binary_classifier or random_forest_classifier
CLASSIFIER = "neural_network_binary_classifier"

# Either graph or text
REPRESENTATION = "text"

# Model maximum context lengths mapping
MODEL_CONTEXT_LENGTHS = {
    "microsoft/codebert-base": 512,
    "Salesforce/codet5-small": 512,
    "bigcode/starcoder2-15b": 16384,
    "Qwen/Qwen2.5-7B-Instruct": 32768,
}

# Automatically determine chunk size (TOKEN_SIZE) based on selected model
TOKEN_SIZE = MODEL_CONTEXT_LENGTHS.get(MODEL_NAME, 512)

# Chunk overlap proportion (default to 1/4)
OVERLAP_PROPORTION = 0.25
OVERLAP = int(TOKEN_SIZE * OVERLAP_PROPORTION)

NUM_EVAL_FILES = 50
NUM_TRAIN_FILES = 500

USE_MODIFIED_FILES = False

# The classification threshold for detecting crypto files.
CLASSIFIER_THRESHOLD = 0.95

INTERPRETER = "venv/bin/python3"

# Random Forest Hyperparameters
RF_N_ESTIMATORS = 200
RF_MAX_DEPTH = None
RF_MIN_SAMPLES_LEAF = 1

# Neural Network Hyperparameters
NN_LEARNING_RATE = 0.005
NN_BATCH_SIZE = 32
NN_EPOCHS = 500

# Conflict-free identifiers for model and overlap
MODEL_CLEAN = MODEL_NAME.split("/")[-1].replace("-", "_").replace(".", "_")
OVERLAP_CLEAN = f"overlap_1_{int(1/OVERLAP_PROPORTION)}" if OVERLAP_PROPORTION > 0 else "overlap_none"

## File Paths
CHUNKS_PATH = f"cache/{REPRESENTATION}_chunks_{MODEL_CLEAN}_{OVERLAP_CLEAN}.pt"
BASE_EMBEDDINGS_PATH = f"cache/{REPRESENTATION}_base_embeddings_{MODEL_CLEAN}_{OVERLAP_CLEAN}.pt"
DIRTY_CLASSIFIER_PATH = f"cache/{REPRESENTATION}_{CLASSIFIER}_{MODEL_CLEAN}_{OVERLAP_CLEAN}.{'pkl' if CLASSIFIER == 'random_forest_classifier' else 'pt'}"
CHUNK_CLASSIFICATION_PATH = f"cache/{REPRESENTATION}_{CLASSIFIER}_classifications_{MODEL_CLEAN}_{OVERLAP_CLEAN}.pt"
FINE_TUNED_MODEL_DIR = f"cache/{REPRESENTATION}_{CLASSIFIER}_fine_tuned_{MODEL_CLEAN}_{OVERLAP_CLEAN}/"
FINE_TUNED_EMBEDDINGS_PATH = f"cache/{REPRESENTATION}_fine_tuned_embeddings_{MODEL_CLEAN}_{OVERLAP_CLEAN}.pt"
FINE_TUNED_CLASSIFIER_PATH = f"cache/fine_tuned_{REPRESENTATION}_{CLASSIFIER}_{MODEL_CLEAN}_{OVERLAP_CLEAN}.{'pkl' if CLASSIFIER == 'random_forest_classifier' else 'pt'}"
EVALUATION_RESULT_PATH = f"Evaluations/{REPRESENTATION}_{CLASSIFIER}_evaluation_{RANDOM_SEED}/"
