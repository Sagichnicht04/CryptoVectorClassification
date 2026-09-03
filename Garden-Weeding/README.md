# Garden-Weeding

Detect cryptographic implementations source code files.

Garden-Weeding scans a directory of source files, embeds their contents using a large language model and classifies each file as cryptographic or non-cryptographic using a Random Forest classifier.

## Installation

Create a virtual environment, clone the repository and install with pip:

```bash
python -m venv venv
source venv/bin/activate
git clone <repository-url>
cd Garden-Weeding
pip install .
```

For development (includes pytest):

```bash
pip install -e ".[dev]"
```

For 4-bit quantization support (will fail if no NVIDIA GPU is installed):

```bash
pip install -e ".[quantization]"
```


## How it works

Large language models can not only be applied for text generation. These models are also capable to take input text and transform it into a vectorized representation (embedding) which is basically a large array of numbers. Somewhere within these numbers lies the natural language meaning of the input text. While humans can not interpret these numbers by themselves, computers are experts at understanding numbers and identify patterns within them and match them to a specific class. 

The tool presented here follows this principle. It uses a large language model to embed source code into a vector. Then, a random forest classifier analyzes these numbers and classifies whether the numbers appear to represent cryptographic code or non-cryptographic code. For a detailed expanation, feel free to take a look into my [bachelor thesis](../Bachelorarbeit.pdf) about this topic. 

1. **Tokenisation & chunking** -- Using large language models on simple consumer hardware often requires to chunk down source code files, as whole files can not be processed in a single step. Each source file is tokenised and split into overlapping chunks (configurable size and overlap).
2. **Embedding** -- Every chunk is passed through a transformer model (default: `Alibaba-NLP/gte-Qwen2-1.5B-instruct`) to produce a vector embedding. GPU is used automatically when available, with a CPU fallback.
3. **Classification** -- A pre-trained Random Forest classifier assigns a cryptographic probability to each chunk.
4. **Thresholding** -- If any chunk in a file exceeds the configured threshold, the file is flagged as cryptographic.

Embeddings are cached on disk (keyed by file content hash and model configuration), so re-running the tool on an unchanged codebase is nearly instant.

## Requirements

- Python >= 3.10
- A HuggingFace transformer model downloaded locally (the tool runs in offline
  mode by default -- set `HF_HUB_OFFLINE=0` to allow downloads)
- (Optional) An NVIDIA GPU with CUDA for faster embedding

## Usage

After installation, the tool is available as a command-line program:

```bash
garden-weeding --target ./src
```

It can also be invoked as a Python module:

```bash
python -m garden_weeding --target ./src
```

Or used programmatically:

```python
from garden_weeding import main
main()
```

### Quick examples

```bash
# Scan all C/C++ files in a directory
garden-weeding --target ./src

# Scan with a strict threshold on GPU
garden-weeding --target ./src --strict-threshold --force-gpu

# Scan all files (not just C/C++) with a rough threshold
garden-weeding --target ./src --rough-threshold --include-non-c-files

# Re-embed everything from scratch with 4-bit quantization
garden-weeding --target ./src --no-cache --use-quantization

# Train a new classifier from labelled data
garden-weeding --train --positives ./crypto --negatives ./non-crypto
```

### Full option reference

Run `garden-weeding --help` to see all options. The highlights:

| Option | Description |
|---|---|
| `--target DIR` | Directory to scan (default: `./`) |
| `--threshold FLOAT` | Crypto detection threshold (default: `0.22`) |
| `--strict-threshold` | Use threshold `0.22` (fewer false negatives) |
| `--rough-threshold` | Use threshold `0.78` (fewer false positives) |
| `--verbose` | Enable debug output (config dump, per-chunk probabilities, device info) |
| `--force-gpu` / `--force-cpu` | Override automatic hardware detection |
| `--use-quantization` | Enable 4-bit quantization to reduce VRAM usage |
| `--include-non-c-files` | Process all files, not just `.c`/`.cpp`/`.cc`/`.cxx` |
| `--exclusion-list FILE` | Path to exclusion patterns file (default: `./.exclude`) |
| `--no-cache` | Ignore existing cache; re-embed all files |
| `--only-cache` | Only classify already-cached files |
| `--reset-cache` | Wipe cache before starting |
| `--cache-dir DIR` | Cache directory (default: `~/.cache/garden_weeding/`) |
| `--embedding-model-name NAME` | HuggingFace model to use for embedding |
| `--token-size INT` | Max tokens per chunk (default: `4096`) |
| `--chunk-overlap-size INT` | Token overlap between chunks (default: `512`) |
| `--classifier-file PATH` | Path to the pickled Random Forest classifier |
| `--train` | Switch to training mode |
| `--positives DIR` | Directory of crypto-positive training samples |
| `--negatives DIR` | Directory of crypto-negative training samples |

### Exclusion list

Create a file (default: `./.exclude`) with one pattern per line to skip files
during scanning. Blank lines and lines starting with `#` are ignored.

Two pattern styles are supported:

**Gitignore-style globs** (recommended):

```
# Skip all text files
*.txt

# Skip everything under vendor/ directories at any depth
**/vendor/*

# Skip a specific subdirectory
third_party/*
```

Patterns without a `/` are matched against the filename only (so `*.txt`
excludes text files at any depth). Patterns with a `/` are matched against the
path relative to the target directory.

**Python regular expressions** (auto-detected when the pattern contains
characters like `^`, `$`, `[`, `]`, `+`, `|`):

```
^test[0-9]+\.c$
^(build|dist)/
```

The exclusion list applies to both new files discovered during a scan and
files already present in the cache.

### Output

The classification report is printed to **stdout**. Progress messages and logs
go to **stderr**, so the report can be piped or redirected cleanly:

```bash
garden-weeding --target ./src > report.txt
```

Use `--verbose` for detailed debug output including per-file chunk
probabilities, full configuration, and device information.

## Caching

Embeddings are expensive to compute. Garden-Weeding caches them on disk so
unchanged files are not re-embedded across runs.

- Cache files are stored in `~/.cache/garden_weeding/` by default
  (`--cache-dir` to change).
- The cache is namespaced by model configuration (model name, token size,
  chunk overlap), so switching models creates a separate cache.
- File identity is determined by MD5 content hash -- if a file's contents
  change, it will be re-embedded automatically.
- Only files under the current `--target` directory are loaded from cache.
  Files cached from previous runs against different directories are not mixed
  in.

## Training

To train a new classifier, organise your labelled data into two directories
(one for cryptographic files, one for non-cryptographic) and run:

```bash
garden-weeding --train \
    --positives ./labelled/crypto \
    --negatives ./labelled/non-crypto
```

This embeds all training files, trains a Random Forest classifier (200 trees,
max depth 12), and saves it as a pickle file at the path given by
`--classifier-file`. The new classifier is then used for all subsequent
classification runs.

Training data can use any file extension if you pass `--include-non-c-files`.

## Running tests

Install development dependencies, then run pytest from the project root:

```bash
pip install -e ".[dev]"
pytest
```

For verbose test output:

```bash
pytest -v
```

The test suite covers all modules:

| Test file | Module | What is tested |
|---|---|---|
| `cache-test.py` | `cache.py` | Hashing, cache init/read/write, exclusion patterns, target filtering |
| `classifier-test.py` | `classifier.py` | Model loading, training, prediction, edge cases |
| `evaluate-test.py` | `evaluate.py` | Threshold sweeping, metrics, HTML report generation |
| `main-test.py` | `main.py` | CLI argument validation, `--help` output, error handling |
| `process_file-test.py` | `process_file.py` | Embedding pooling strategies, environment setup |

## Project structure

```
Garden-Weeding/
  pyproject.toml                   Build configuration and dependencies
  README.md
  src/
    garden_weeding/
      __init__.py                  Package entry point, exposes main()
      __main__.py                  Enables `python -m garden_weeding`
      main.py                      CLI argument parsing, pipeline orchestration
      cache.py                     File hashing, embedding cache, exclusion list
      classifier.py                Random Forest classifier (load, predict, train)
      process_file.py              Transformer-based embedding engine
      evaluate.py                  Model evaluation and HTML report generation
      classifier.pkl               Pre-trained Random Forest model
      train.py                     (placeholder)
    tests/
      cache-test.py
      classifier-test.py
      evaluate-test.py
      main-test.py
      process_file-test.py
```

## License

See repository for license information.
