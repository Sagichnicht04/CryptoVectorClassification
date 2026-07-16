import os
import numpy as np
import pickle

import config
import random


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

    def __init__(self, path):
        self.model = None
        self.path = path

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
            n_estimators=200,
            max_depth=None,
            min_samples_leaf=1,
            n_jobs=-1,
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
        with open(self.path, "rb") as f:
            self.model = pickle.load(f)
        return True

    def predict(self, embeddings: list) -> tuple[bool, float]:
        """
        Returns (is_crypto, max_crypto_probability).
        is_crypto is True when max P(crypto) >= CLASSIFIER_THRESHOLD.
        """
        if self.model is None:
            raise RuntimeError("RandomForestClassifier has not been trained/loaded.")
        
        if not embeddings:
            return False, 0.0
        
        # Get probability for each chunk
        probas = self.model.predict_proba(np.vstack(embeddings))
        
        # Use the max probability of any chunk as the file's probability
        max_proba = float(np.max(probas[:, 1]))
        
        threshold = getattr(config, 'CLASSIFIER_THRESHOLD', 0.25)
        is_crypto = max_proba >= threshold
        return is_crypto, max_proba

    # ------------------------------------------------------------------
    def predict_proba(self, embeddings: list) -> np.ndarray:
        """
        Returns the class probabilities for each chunk embedding.
        """
        if self.model is None:
            raise RuntimeError("RandomForestClassifier has not been trained/loaded.")
        
        if not embeddings:
            return np.array([])
            
        return self.model.predict_proba(np.vstack(embeddings))


# ---------------------------------------------------------------------------
# BCE (Binary Cross Entropy) PyTorch neural classifier
# ---------------------------------------------------------------------------

import torch
import torch.nn as nn
import torch.optim as optim

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
    def __init__(self, path):
        self.model = None
        self.path = path
        self.device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    def train(self, crypto_embeddings: list, non_crypto_embeddings: list) -> None:
        """
        Train the PyTorch BCE model on the supplied embeddings and save to disk.
        """
        # Flatten lists of lists
        all_crypto_embs = [emb for file_embs in crypto_embeddings for emb in file_embs]
        all_non_crypto_embs = [emb for file_embs in non_crypto_embeddings for emb in file_embs]

        X_np = np.vstack(all_crypto_embs + all_non_crypto_embs).astype("float32")
        y_np = np.array([1.0] * len(all_crypto_embs) + [0.0] * len(all_non_crypto_embs), dtype="float32")

        X = torch.tensor(X_np).to(self.device)
        y = torch.tensor(y_np).unsqueeze(1).to(self.device)

        input_dim = X.shape[1]
        self.model = BCEClassifierModule(input_dim).to(self.device)
        self.model.train()

        criterion = nn.BCEWithLogitsLoss()
        optimizer = optim.Adam(self.model.parameters(), lr=0.005)

        epochs = 100
        batch_size = 32
        dataset_size = len(X)

        print(f"  Training BCE Classifier on {len(all_crypto_embs)} crypto chunks "
              f"and {len(all_non_crypto_embs)} non-crypto chunks…")

        for epoch in range(epochs):
            # Shuffle each epoch
            permutation = torch.randperm(dataset_size)
            epoch_loss = 0.0
            num_batches = 0
            for i in range(0, dataset_size, batch_size):
                indices = permutation[i:i+batch_size]
                batch_X, batch_y = X[indices], y[indices]

                optimizer.zero_grad()
                outputs = self.model(batch_X)
                loss = criterion(outputs, batch_y)
                loss.backward()
                optimizer.step()

                epoch_loss += loss.item()
                num_batches += 1
            
            if (epoch + 1) % 20 == 0 or epoch == 0:
                avg_loss = epoch_loss / num_batches
                print(f"    Epoch {epoch+1}/{epochs} - Loss: {avg_loss:.4f}")

        # Save model
        torch.save(self.model.state_dict(), self.path)
        print(f"  BCE classifier saved to '{self.path}'")

    def load(self) -> bool:
        if not os.path.exists(self.path):
            return False
        state_dict = torch.load(self.path, map_location=self.device, weights_only=False)
        input_dim = state_dict['linear.weight'].shape[1]
        self.model = BCEClassifierModule(input_dim).to(self.device)
        self.model.load_state_dict(state_dict)
        self.model.eval()
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




def get_classifier(path):
    """Return the classifier instance configured in config.CLASSIFIER_TYPE."""
    if config.CLASSIFIER == "random_forest_classifier":
        return RandomForestClassifier(path)
    elif config.CLASSIFIER == "neural_network_binary_classifier":
        return BCEClassifier(path)
    else:
        raise ValueError(
            f"Unknown CLASSIFIER_TYPE '{config.CLASSIFIER}'. "
        )
