import numpy as np
import config


class evaluation:
    def evaluate(self, crypto_embeddings, non_crypto_embeddings, discarded_crypto_embeddings):
        
        thresholds = {"crypto":[], "non_crypto":[], "discarded_crypto":[]}

        def run_query(embeddings, group):
            score = 0
            for embedding in embeddings:
                if embedding["probability"] > score:
                    score = embedding["probability"]

            thresholds[group].append(score)

        for embedding in crypto_embeddings:
            run_query(crypto_embeddings[embedding], "crypto")

        for embedding in non_crypto_embeddings:
            run_query(non_crypto_embeddings[embedding], "non_crypto")

        for embedding in discarded_crypto_embeddings:
            run_query(discarded_crypto_embeddings[embedding], "discarded_crypto")

        best_f1 = -1.0
        best_f1_thr = 0.5
        best_f1_metrics = {}
        
        best_fn = len(crypto_embeddings) + 1
        best_fn_f1 = -1.0
        best_fn_thr = 0.5
        best_fn_metrics = {}
        
        for thr_int in range(5, 1000, 1):
            thr = thr_int / 1000.0
            tp = sum(1 for s in thresholds["crypto"] if s >= thr)
            fn = sum(1 for s in thresholds["crypto"] if s < thr)
            
            unrelated_tn = sum(1 for s in thresholds["non_crypto"] if s < thr)
            unrelated_fp = sum(1 for s in thresholds["non_crypto"] if s >= thr)
            
            discarded_tn = sum(1 for s in thresholds["discarded_crypto"] if s < thr)
            discarded_fp = sum(1 for s in thresholds["discarded_crypto"] if s >= thr)
            
            total_fp = unrelated_fp + discarded_fp
            precision = tp / (tp + total_fp) if (tp + total_fp) > 0 else 0
            recall = tp / (tp + fn) if (tp + fn) > 0 else 0
            f1 = 2 * precision * recall / (precision + recall) if (precision + recall) > 0 else 0
            
            # 1. Best F1 Search
            if f1 > best_f1:
                best_f1 = f1
                best_f1_thr = thr
                best_f1_metrics = {
                    "novel_crypto_tp": tp,
                    "novel_crypto_fn": fn,
                    "non_crypto_unrelated_tn": unrelated_tn,
                    "non_crypto_unrelated_fp": unrelated_fp,
                    "non_crypto_discarded_tn": discarded_tn,
                    "non_crypto_discarded_fp": discarded_fp
                }
                
            # 2. Min FN Search (break ties with highest F1 score to minimize false positives)
            if fn < best_fn or (fn == best_fn and f1 > best_fn_f1):
                best_fn = fn
                best_fn_f1 = f1
                best_fn_thr = thr
                best_fn_metrics = {
                    "novel_crypto_tp": tp,
                    "novel_crypto_fn": fn,
                    "non_crypto_unrelated_tn": unrelated_tn,
                    "non_crypto_unrelated_fp": unrelated_fp,
                    "non_crypto_discarded_tn": discarded_tn,
                    "non_crypto_discarded_fp": discarded_fp
                }

        stats = {
            "input_type": config.REPRESENTATION,
            "classifier_type": config.CLASSIFIER,
            "best_f1_threshold": best_f1_thr,
            "best_f1_metrics": best_f1_metrics,
            "min_fn_threshold": best_fn_thr,
            "min_fn_metrics": best_fn_metrics,
        }
        
        return stats
