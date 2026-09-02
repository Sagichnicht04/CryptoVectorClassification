import os
import json
import pickle
import numpy as np

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


def inference(args, embeddings):
    classifier = get_classifier("random_forest_classifier", config.DIRTY_CLASSIFIER_PATH,
                                        {
                                            "n_estimators": 200,
                                            "max_depth": 12,
                                            "min_samples_split": 5,
                                            "bootstrap": False,
                                            "seed": 0
                                        })
    if not dirty_classifier.load():
        print(f"Error: Classifier not found {dirty_classifier.path}")
        exit()

    fine_tuned_embedding = base_embedding
    fine_tuned_embedded_chunks = base_embedded_chunks
    fine_tuned_classifier = dirty_classifier


    print("-- Step 9: Evaluate Classifier --")
    if config.EVALUATE_CLASSIFIER:
        crypto_embeddings = {}
        non_crypto_embeddings = {}
        discarded_crypto_embeddings = {}
        files_probas = {}

        for file in fine_tuned_embedded_chunks:

            lang = get_lang_from_path(file)
            if lang:
                embeddings = []
                chunk_tokens = []
                for chunk in fine_tuned_embedded_chunks[file]:
                    embeddings.append(chunk["embedding"])
                    chunk_tokens.append(chunk["chunk_tokens"])

                probabilities = fine_tuned_classifier.predict_proba(embeddings)
                proba = 0
                discarded_crypto_embeddings[file] = [] 
                for index, probability in enumerate(probabilities):
                    discarded_crypto_embeddings[file].append({
                        "probability": probability[1],
                        "clear_text": fine_tuned_embedding.decode(chunk_tokens[index]["input_ids"])
                    })
                    if probability[1] > proba:
                        proba = probability[1]
                files_probas[file] = proba
                    

        os.makedirs(config.EVALUATION_RESULT_PATH, exist_ok=True)

        with open(f"{config.EVALUATION_RESULT_PATH}crypto_results.json", "w") as f:
            json.dump(crypto_embeddings, f)
        with open(f"{config.EVALUATION_RESULT_PATH}non_crypto_results.json", "w") as f:
            json.dump(non_crypto_embeddings, f)
        with open(f"{config.EVALUATION_RESULT_PATH}api_crypto_results.json", "w") as f:
            json.dump(discarded_crypto_embeddings, f)
        with open(f"{config.EVALUATION_RESULT_PATH}probas.json", "w") as f:
                json.dump(files_probas, f)


        evaluator = evaluation()

        id = config.EVALUATION_RESULT_PATH

        os.makedirs(f"{id}", exist_ok=True)


        results, html = evaluator.evaluate(crypto_embeddings,non_crypto_embeddings,discarded_crypto_embeddings, "text", "random_forest_classifier")
        with open(f"{id}/evaluation.json", "w") as f:
            json.dump(results, f)

        with open(f"{id}/evaluation.html", "w", encoding="utf-8") as f:
            f.write(html)

        print(f"Evaluated {id} - F1-Score: {results["best_f1_metrics"]["f1"]}")
