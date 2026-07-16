
## Steps
PREPARE_DATA = True
CHUNK_DATA = True
BASE_EMBED_CHUNKS = True
TRAIN_DIRTY_CLASSIFIER = True
CLASSIFY_CHUNKS = True
FINETUNE_MODEL = True
FINETUNED_EMBED_CHUNKS = True
TRAIN_FINETUNED_CLASSIFIER = True
EVALUATE_CLASSIFIER = True



# Add models from Hugging Face or local paths.
# For local models, provide the absolute path to the model directory.
MODELS = [
    "microsoft/codebert-base",
    "Qwen/Qwen2.5-Coder-14B",
    "Qwen/Qwen2.5-Coder-14B-Instruct",
    "bigcode/starcoder2-15b",
    "Qwen/Qwen2.5-14B-Instruct",
    "Qwen/Qwen2.5-7B-Instruct",
    "arcee-ai/SuperNova-Medius",
    "deepseek-ai/deepseek-coder-6.7b-instruct",
    "microsoft/codebert-base-text",  # Example of a local fine-tuned model
    "microsoft/codebert-base-graph",
    "microsoft/codebert-base-math",
    "/home/flo/codebert-local",
]

# The pre-trained model to use from Hugging Face
MODEL_NAME = MODELS[0]

# Either neural_network_binary_classifier or random_forest_classifier
CLASSIFIER = "neural_network_binary_classifier"

# Either graph or text
REPRESENTATION = "text"

#2**12 = 4096
#TOKEN_SIZE = 2**12
TOKEN_SIZE = 512
OVERLAP = 128

NUM_EVAL_FILES = 50
NUM_TRAIN_FILES = 500

USE_MODIFIED_FILES = False

# The classification threshold for detecting crypto files.
CLASSIFIER_THRESHOLD = 0.95

INTERPRETER = "venv/bin/python3"


## File Paths
CHUNKS_PATH = f"{REPRESENTATION}_chunks.pt"
BASE_EMBEDDINGS_PATH = f"{REPRESENTATION}_base_embeddings.pt"
DIRTY_CLASSIFIER_PATH = f"{REPRESENTATION}_{CLASSIFIER}.{"pkl" if REPRESENTATION == "graph" else "pt"}"
CHUNK_CLASSIFICATION_PATH = f"{REPRESENTATION}_{CLASSIFIER}_classifications.pt"
FINE_TUNED_MODEL_DIR = f"{REPRESENTATION}_{CLASSIFIER}_fine_tuned/"
FINE_TUNED_EMBEDDINGS_PATH = f"{REPRESENTATION}_fine_tuned_embeddings.pt"
FINE_TUNED_CLASSIFIER_PATH = f"fine_tuned_{REPRESENTATION}_{CLASSIFIER}.{"pkl" if REPRESENTATION == "graph" else "pt"}"
EVALUATION_RESULT_PATH = f"{REPRESENTATION}_{CLASSIFIER}_evaluation/"