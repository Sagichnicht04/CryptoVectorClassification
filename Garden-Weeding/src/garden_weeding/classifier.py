import os
import numpy as np
import pickle

import random
import time

# ---------------------------------------------------------------------------
# Random-forest classifier
# ---------------------------------------------------------------------------

class RandomForestClassifier:
    """
    A scikit-learn Random Forest trained on labelled embedding vectors.

    Label convention:  1 = crypto,  0 = non-crypto.

    The classifier is saved / loaded with pickle so the full workflow can
    serialise it between the training step and the evaluation step.

    The decision threshold is read from config.py (CLASSIFIER_THRESHOLD).
    """

    def __init__(self, path, config):
        self.model = None
        self.path = path

        self.RF_N_ESTIMATORS = config["n_estimators"]
        self.RF_MAX_DEPTH = config["max_depth"]
        self.RF_MIN_SAMPLES_SPLIT = config["min_samples_split"]
        self.BOOTSTRAP = config["bootstrap"]
        random.seed(config["seed"])

    # ------------------------------------------------------------------
    def train(self, crypto_embeddings: list,
              non_crypto_embeddings: list) -> None:
        """
        Train the Random Forest on the supplied embeddings and save to disk.
        """
        from sklearn.ensemble import RandomForestClassifier as _RFC
        from sklearn.model_selection import cross_val_score

        # Flatten the list of lists of embeddings into a single list of embeddings
        all_crypto_embs = [emb for file_embs in crypto_embeddings for emb in file_embs]
        all_non_crypto_embs = [emb for file_embs in non_crypto_embeddings for emb in file_embs]

        X = np.vstack(all_crypto_embs + all_non_crypto_embs).astype("float32")
        y = np.array([1] * len(all_crypto_embs) + [-1] * len(all_non_crypto_embs))

        print(f"  Training Random Forest on {len(all_crypto_embs)} crypto chunks "
              f"and {len(all_non_crypto_embs)} non-crypto chunks…")

        self.model = _RFC(
            n_estimators=self.RF_N_ESTIMATORS,
            max_depth=self.RF_MAX_DEPTH,
            min_samples_split=self.RF_MIN_SAMPLES_SPLIT,
            bootstrap=self.BOOTSTRAP,
            n_jobs=8,
            random_state=random.randrange(10000),
        )
        self.model.fit(X, y)

        # Quick cross-validated accuracy estimate on the training data
        if len(X) >= 5:
            cv_scores = cross_val_score(self.model, X, y, cv=min(5, len(X)), scoring="accuracy")
            print(f"  RF cross-val accuracy: {cv_scores.mean():.2%} ± {cv_scores.std():.2%}")

        with open(self.path, "wb") as f:
            pickle.dump(self.model, f)
        print(f"  Random Forest classifier saved to '{self.path}'")

    def load(self) -> bool:
        if not os.path.exists(self.path):
            return False
        try:
            with open(self.path, "rb") as f:
                self.model = pickle.load(f)
        except:
            return False
        return True

    # ------------------------------------------------------------------
    def predict_proba(self, embeddings: list) -> np.ndarray:
        """
        Returns the class probabilities for each chunk embedding.
        """
        if self.model is None:
            raise RuntimeError("RandomForestClassifier has not been trained/loaded.")
        
        if not embeddings:
            return np.array([])


        X = np.vstack(embeddings)
        probs = self.model.predict_proba(X)
        return probs


# ---------------------------------------------------------------------------
# BCE (Binary Cross Entropy) PyTorch neural classifier
# ---------------------------------------------------------------------------

import torch
import torch.nn as nn
import torch.optim as optim


class MultiLayerClassifierModule(nn.Module):
    def __init__(self, input_dim):
        super().__init__()

        self.net = nn.Sequential(
            nn.Linear(input_dim, 512),
            nn.LayerNorm(512),
            nn.GELU(),
            nn.Dropout(0.2),

            nn.Linear(512, 128),
            nn.LayerNorm(128),
            nn.GELU(),
            nn.Dropout(0.2),

            nn.Linear(128, 1)
        )

        self._init_weights()

    def _init_weights(self):
        for m in self.modules():
            if isinstance(m, nn.Linear):
                nn.init.kaiming_normal_(m.weight, nonlinearity='relu')
                if m.bias is not None:
                    nn.init.constant_(m.bias, 0.0)
            elif isinstance(m, nn.LayerNorm):
                nn.init.constant_(m.bias, 0.0)
                nn.init.constant_(m.weight, 1.0)

    def forward(self, x):
        return self.net(x)

class BCEClassifierModule(nn.Module):
    def __init__(self, input_dim):
        super().__init__()
        self.linear = nn.Linear(input_dim, 1)

    def forward(self, x):
        return self.linear(x)

class BCEClassifier:
    """
    A PyTorch-based neural classifier trained using Binary Cross Entropy (BCE) loss
    on top of the vector embeddings.
    """
    def __init__(self, path, config):
        self.model = None
        self.path = path
        # Respect the global USE_GPU flag from the shared config module so the entire
        # workflow can be forced onto CPU when no CUDA GPU is available.
        try:
            import config as _cfg
            _use_gpu = bool(getattr(_cfg, "USE_GPU", True))
        except Exception:
            _use_gpu = True
        self.device = torch.device("cuda" if (_use_gpu and torch.cuda.is_available()) else "cpu")

        self.NN_LEARNING_RATE = config.get("lr", config.get("NN_LEARNING_RATE", 0.005))
        self.NN_EPOCHS = config.get("num_epochs", config.get("NN_EPOCHS", 500))
        self.NN_BATCH_SIZE = config.get("batch_size", config.get("NN_BATCH_SIZE", 32))
        self.ARCHITECTURE = config.get("architecture", config.get("ARCHITECTURE", 1))


    def train(self, crypto_embeddings: list, non_crypto_embeddings: list) -> None:
        """
        Train the PyTorch BCE model on the supplied embeddings and save to disk.
        """
        # Flatten lists of lists
        all_crypto_embs = [emb for file_embs in crypto_embeddings for emb in file_embs]
        all_non_crypto_embs = [emb for file_embs in non_crypto_embeddings for emb in file_embs]

        X_np = np.vstack(all_crypto_embs + all_non_crypto_embs).astype("float32")
        y_np = np.array([1.0] * len(all_crypto_embs) + [0.0] * len(all_non_crypto_embs), dtype="float32")

        t_data_start = time.time()
        X = torch.tensor(X_np).to(self.device)
        y = torch.tensor(y_np).unsqueeze(1).to(self.device)

        input_dim = X.shape[1]
        if self.ARCHITECTURE == 1:
            self.model = BCEClassifierModule(input_dim).to(self.device)
        else:
            self.model = MultiLayerClassifierModule(input_dim).to(self.device)
            
        self.model.train()

        criterion = nn.BCEWithLogitsLoss()
        # Use fused Adam on CUDA for speedup
        use_fused = (self.device.type == "cuda")
        optimizer = optim.Adam(self.model.parameters(), lr=self.NN_LEARNING_RATE, fused=use_fused)

        epochs = self.NN_EPOCHS
        batch_size = self.NN_BATCH_SIZE
        dataset_size = len(X)
        t_setup_done = time.time()

        print(f"  Training BCE Classifier on {len(all_crypto_embs)} crypto chunks "
              f"and {len(all_non_crypto_embs)} non-crypto chunks…")
        print(f"    [Profiling] Device: {self.device} ({torch.cuda.get_device_name(0) if self.device.type == 'cuda' else 'CPU'})")
        print(f"    [Profiling] Data transfer & model setup: {t_setup_done - t_data_start:.4f} seconds")

        t_train_start = time.time()
        for epoch in range(epochs):
            # Shuffle once per epoch and slice contiguously to eliminate non-contiguous GPU indexing overhead
            permutation = torch.randperm(dataset_size, device=self.device)
            shuffled_X = X[permutation]
            shuffled_y = y[permutation]
            epoch_loss = 0.0
            num_batches = 0
            for i in range(0, dataset_size, batch_size):
                batch_X = shuffled_X[i : i + batch_size]
                batch_y = shuffled_y[i : i + batch_size]

                optimizer.zero_grad(set_to_none=True)
                outputs = self.model(batch_X)
                loss = criterion(outputs, batch_y)
                loss.backward()
                optimizer.step()

                epoch_loss += loss.detach()
                num_batches += 1
            
            if (epoch + 1) % 20 == 0 or epoch == 0:
                avg_loss = (epoch_loss / num_batches).item() if torch.is_tensor(epoch_loss) else epoch_loss / num_batches
                print(f"    Epoch {epoch+1}/{epochs} - Loss: {avg_loss:.4f}")

        t_train_end = time.time()
        total_train_time = t_train_end - t_train_start
        throughput = (dataset_size * epochs) / total_train_time if total_train_time > 0 else 0.0
        print(f"    [Profiling] Training complete. Duration: {total_train_time:.4f}s | Speed: {throughput:.2f} samples/sec")

        # Save model
        torch.save(self.model.state_dict(), self.path)
        print(f"  BCE classifier saved to '{self.path}'")

    def load(self) -> bool:
        if not os.path.exists(self.path):
            return False
        try:
            state_dict = torch.load(self.path, map_location=self.device, weights_only=False)
            
            # Dynamically detect architecture based on state_dict keys
            if "linear.weight" in state_dict:
                input_dim = state_dict["linear.weight"].shape[1]
                self.model = BCEClassifierModule(input_dim).to(self.device)
            elif "net.0.weight" in state_dict:
                input_dim = state_dict["net.0.weight"].shape[1]
                self.model = MultiLayerClassifierModule(input_dim).to(self.device)
            else:
                # Fallback search for any weight parameter to extract input dimension
                input_dim = 1536
                for key in state_dict.keys():
                    if "weight" in key:
                        input_dim = state_dict[key].shape[1]
                        break
                if self.ARCHITECTURE == 1:
                    self.model = BCEClassifierModule(input_dim).to(self.device)
                else:
                    self.model = MultiLayerClassifierModule(input_dim).to(self.device)
            
            self.model.load_state_dict(state_dict)
            self.model.eval()
        except Exception as e:
            print(f"  Error loading classifier model from '{self.path}': {e}")
            return False
        return True


    def predict_proba(self, embeddings: list) -> np.ndarray:
        """
        Returns the class probabilities for each chunk embedding.
        """
        if self.model is None:
            raise RuntimeError("BCEClassifier has not been trained/loaded.")
        
        if not embeddings:
            return np.array([])

        self.model.eval()
        with torch.no_grad():
            X = torch.tensor(np.vstack(embeddings).astype("float32")).to(self.device)
            logits = self.model(X)
            probs = torch.sigmoid(logits).cpu().numpy().squeeze(axis=1)

        probs_2d = np.zeros((len(probs), 2))
        probs_2d[:, 0] = 1.0 - probs
        probs_2d[:, 1] = probs
        return probs_2d




def get_classifier(classifier, path, config):
    """Return the classifier instance configured in config.CLASSIFIER_TYPE."""
    if classifier == "random_forest_classifier":
        return RandomForestClassifier(path, config)
    elif classifier == "neural_network_binary_classifier":
        return BCEClassifier(path, config)
    else:
        raise ValueError(
            f"Unknown CLASSIFIER_TYPE '{config.CLASSIFIER}'. "
        )
