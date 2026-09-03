import os
import json
import pickle
import logging
import numpy as np

log = logging.getLogger("garden_weeding")

class RandomForestClassifier:
    """
    Random Forest trained on labelled embedding vectors.

    Label convention:  1 = crypto,  0 = non-crypto.
    """

    def __init__(self, args):
        self.model = None
        self.path = args.classifier_file
        if not os.path.exists(self.path):
            raise RuntimeError("RandomForestClassifier not found")
        
        try:
            with open(self.path, "rb") as f:
                self.model = pickle.load(f)
        except:
            raise RuntimeError("Error loading RandomForestClassifier")
        
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

        crypto_probs = []
        for prob in probs:
            crypto_probs.append(prob[1])

        return crypto_probs

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

        log.debug("Training Random Forest on %d crypto chunks "
                  "and %d non-crypto chunks...",
                  len(all_crypto_embs), len(all_non_crypto_embs))


        self.model = _RFC(
            n_estimators=200,
            max_depth=12,
            min_samples_split=5,
            bootstrap=False,
            n_jobs=-1,
        )
        self.model.fit(X, y)

        with open(self.path, "wb") as f:
            pickle.dump(self.model, f)
        log.debug("Random Forest classifier saved to '%s'", self.path)