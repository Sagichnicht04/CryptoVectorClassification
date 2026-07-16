import numpy as np
import config
import os
import json
import base64


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

        best_fp = len(non_crypto_embeddings) + len(discarded_crypto_embeddings) + 1
        best_fp_f1 = -1.0
        best_fp_thr = 0.5
        best_fp_metrics = {}
        
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

            # 3. Min FP Search (break ties with highest F1 score)
            if total_fp < best_fp or (total_fp == best_fp and f1 > best_fp_f1):
                best_fp = total_fp
                best_fp_f1 = f1
                best_fp_thr = thr
                best_fp_metrics = {
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
            "min_fp_threshold": best_fp_thr,
            "min_fp_metrics": best_fp_metrics,
        }

        # Generate HTML report
        self.generate_html_report(
            crypto_embeddings,
            non_crypto_embeddings,
            discarded_crypto_embeddings,
            stats
        )
        
        return stats

    def generate_html_report(self, crypto_embeddings, non_crypto_embeddings, discarded_crypto_embeddings, stats):
        # We construct a rich data object for the frontend
        # For each file, we want to know its maximum score (which determines if it is classified as crypto at a given threshold),
        # its group, and the individual chunks with their text and probability scores.
        
        files_data = {}
        
        def process_group(embeddings_dict, group_name):
            for file, chunks in embeddings_dict.items():
                max_score = 0.0
                chunks_list = []
                for idx, c in enumerate(chunks):
                    prob = float(c["probability"])
                    if prob > max_score:
                        max_score = prob
                    
                    # Convert list/array clear_text back to string if needed
                    text_val = c["clear_text"]
                    if isinstance(text_val, list):
                        text_val = "\n".join(text_val)
                        
                    chunks_list.append({
                        "index": idx,
                        "probability": prob,
                        "clear_text": text_val
                    })
                files_data[file] = {
                    "filename": file,
                    "group": group_name,
                    "max_score": max_score,
                    "chunks": chunks_list
                }
                
        process_group(crypto_embeddings, "novel_crypto")
        process_group(non_crypto_embeddings, "non_crypto_unrelated")
        process_group(discarded_crypto_embeddings, "non_crypto_discarded")

        # Create single unified payload
        payload = {
            "stats": stats,
            "files": files_data,
            "config": {
                "input_type": config.REPRESENTATION,
                "classifier_type": config.CLASSIFIER,
                "default_threshold": float(config.CLASSIFIER_THRESHOLD)
            }
        }
        
        payload_json = json.dumps(payload, indent=2)
        
        # Base64 encode to completely prevent string escaping and script closing issues
        payload_b64 = base64.b64encode(payload_json.encode('utf-8')).decode('utf-8')
        
        html_template = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Model Evaluation Dashboard</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <style>
        .custom-scrollbar::-webkit-scrollbar {
            width: 6px;
            height: 6px;
        }
        .custom-scrollbar::-webkit-scrollbar-track {
            background: #1f2937;
        }
        .custom-scrollbar::-webkit-scrollbar-thumb {
            background: #4b5563;
            border-radius: 4px;
        }
        .custom-scrollbar::-webkit-scrollbar-thumb:hover {
            background: #6b7280;
        }
    </style>
</head>
<body class="bg-gray-950 text-gray-100 font-sans min-h-screen flex flex-col">

    <!-- Header -->
    <header class="bg-gray-900 border-b border-gray-800 px-6 py-4 flex flex-wrap justify-between items-center gap-4">
        <div>
            <h1 class="text-2xl font-bold tracking-tight text-white flex items-center gap-2">
                <svg class="w-8 h-8 text-indigo-500" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 19v-6a2 2 0 00-2-2H5a2 2 0 00-2 2v6a2 2 0 002 2h2a2 2 0 002-2zm0 0V9a2 2 0 012-2h2a2 2 0 012 2v10m-6 0a2 2 0 002 2h2a2 2 0 002-2m0 0V5a2 2 0 012-2h2a2 2 0 012 2v14a2 2 0 01-2 2h-2a2 2 0 01-2-2z"></path>
                </svg>
                Model Evaluation Dashboard
            </h1>
            <p class="text-sm text-gray-400 mt-1">
                Representation: <span class="text-indigo-400 font-semibold uppercase">__REPRESENTATION__</span> | 
                Classifier: <span class="text-indigo-400 font-semibold">__CLASSIFIER__</span>
            </p>
        </div>
        <div class="flex items-center gap-3">
            <span class="text-xs bg-gray-800 text-gray-300 px-3 py-1.5 rounded-full border border-gray-700">Interactive Report</span>
        </div>
    </header>

    <!-- Main Content Area -->
    <main class="flex-1 flex flex-col lg:flex-row overflow-hidden">
        
        <!-- Left Panel: Controls, Thresholds & Metrics -->
        <div class="w-full lg:w-2/5 p-6 border-r border-gray-800 flex flex-col gap-6 overflow-y-auto custom-scrollbar lg:h-[calc(100vh-80px)]">
            
            <!-- Threshold Adjustment Card -->
            <div class="bg-gray-900 rounded-xl p-5 border border-gray-800">
                <h2 class="text-lg font-semibold text-white mb-4 flex items-center gap-2">
                    <svg class="w-5 h-5 text-indigo-400" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 6V4m0 2a2 2 0 100 4m0-4a2 2 0 110 4m-6 8a2 2 0 100-4m0 4a2 2 0 110-4m0 4v2m0-6V4m6 6v10m6-2a2 2 0 100-4m0 4a2 2 0 110-4m0 4v2m0-6V4"></path></svg>
                    Threshold Tuning
                </h2>
                
                <div class="flex justify-between items-center mb-2">
                    <span class="text-sm text-gray-400">Decision Threshold</span>
                    <span id="threshold-val" class="text-2xl font-mono font-bold text-indigo-400">0.500</span>
                </div>
                
                <input type="range" id="threshold-slider" min="0" max="1" step="0.001" value="0.5" 
                       class="w-full h-2 bg-gray-800 rounded-lg appearance-none cursor-pointer accent-indigo-500 mb-6">
                
                <!-- Quick Set Targets -->
                <div class="space-y-3">
                    <span class="text-xs font-semibold text-gray-400 uppercase tracking-wider block">Target Optimizations</span>
                    
                    <button onclick="setThreshold(__BEST_F1_THRESHOLD__)" 
                            class="w-full flex justify-between items-center bg-gray-800 hover:bg-indigo-950 hover:border-indigo-800 transition px-4 py-2.5 rounded-lg border border-gray-700 text-left">
                        <div class="flex flex-col">
                            <span class="text-sm font-semibold text-white">Optimize F1-Score</span>
                            <span class="text-xs text-gray-400">Balanced Precision & Recall</span>
                        </div>
                        <span class="bg-indigo-900 text-indigo-200 text-xs px-2.5 py-1 rounded font-mono font-bold border border-indigo-700">
                            __BEST_F1_THRESHOLD_STR__
                        </span>
                    </button>
                    
                    <button onclick="setThreshold(__MIN_FN_THRESHOLD__)" 
                            class="w-full flex justify-between items-center bg-gray-800 hover:bg-emerald-950 hover:border-emerald-800 transition px-4 py-2.5 rounded-lg border border-gray-700 text-left">
                        <div class="flex flex-col">
                            <span class="text-sm font-semibold text-white">Minimize False Negatives (Min FN)</span>
                            <span class="text-xs text-gray-400">Highest Security / Detection</span>
                        </div>
                        <span class="bg-emerald-900 text-emerald-200 text-xs px-2.5 py-1 rounded font-mono font-bold border border-emerald-700">
                            __MIN_FN_THRESHOLD_STR__
                        </span>
                    </button>

                    <button onclick="setThreshold(__MIN_FP_THRESHOLD__)" 
                            class="w-full flex justify-between items-center bg-gray-800 hover:bg-amber-950 hover:border-amber-800 transition px-4 py-2.5 rounded-lg border border-gray-700 text-left">
                        <div class="flex flex-col">
                            <span class="text-sm font-semibold text-white">Minimize False Positives (Min FP)</span>
                            <span class="text-xs text-gray-400">Lowest False Alarm Rate</span>
                        </div>
                        <span class="bg-amber-900 text-amber-200 text-xs px-2.5 py-1 rounded font-mono font-bold border border-amber-700">
                            __MIN_FP_THRESHOLD_STR__
                        </span>
                    </button>
                </div>
            </div>
            
            <!-- Real-time Metrics Overview -->
            <div class="bg-gray-900 rounded-xl p-5 border border-gray-800">
                <h2 class="text-lg font-semibold text-white mb-4 flex items-center gap-2">
                    <svg class="w-5 h-5 text-indigo-400" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M11 3.055A9.003 9.003 0 1020.945 13H11V3.055z"></path><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M20.488 9H15V3.512A9.025 9.003 0 0120.488 9z"></path></svg>
                    Performance Metrics
                </h2>
                
                <div class="grid grid-cols-3 gap-3 mb-6">
                    <div class="bg-gray-850 p-3 rounded-lg border border-gray-850 text-center">
                        <div class="text-xs text-gray-400 mb-1 font-semibold">F1-Score</div>
                        <div id="metric-f1" class="text-xl font-bold font-mono text-indigo-400">0.00%</div>
                    </div>
                    <div class="bg-gray-850 p-3 rounded-lg border border-gray-850 text-center">
                        <div class="text-xs text-gray-400 mb-1 font-semibold">Precision</div>
                        <div id="metric-precision" class="text-xl font-bold font-mono text-indigo-400">0.00%</div>
                    </div>
                    <div class="bg-gray-850 p-3 rounded-lg border border-gray-850 text-center">
                        <div class="text-xs text-gray-400 mb-1 font-semibold">Recall</div>
                        <div id="metric-recall" class="text-xl font-bold font-mono text-indigo-400">0.00%</div>
                    </div>
                </div>
                
                <!-- Confusion Matrix Grid -->
                <div class="border border-gray-800 rounded-xl overflow-hidden">
                    <div class="grid grid-cols-3 bg-gray-800 text-center py-2 text-xs font-semibold tracking-wider text-gray-400 uppercase">
                        <div>Actual / Pred</div>
                        <div>Crypto</div>
                        <div>Non-Crypto</div>
                    </div>
                    
                    <!-- Row 1: Actual Crypto -->
                    <div class="grid grid-cols-3 border-t border-gray-800 items-center text-center py-4 bg-gray-900">
                        <div class="text-xs font-bold text-gray-300">Crypto</div>
                        
                        <!-- True Positive -->
                        <div class="px-2">
                            <div class="bg-emerald-950/40 text-emerald-400 border border-emerald-800/50 py-2 rounded-lg font-mono">
                                <span id="cm-tp" class="text-lg font-bold">0</span>
                                <span class="text-[10px] block font-semibold uppercase text-emerald-500/70 tracking-tight">True Pos (TP)</span>
                            </div>
                        </div>
                        
                        <!-- False Negative -->
                        <div class="px-2">
                            <div class="bg-rose-950/40 text-rose-400 border border-rose-800/50 py-2 rounded-lg font-mono">
                                <span id="cm-fn" class="text-lg font-bold">0</span>
                                <span class="text-[10px] block font-semibold uppercase text-rose-500/70 tracking-tight">False Neg (FN)</span>
                            </div>
                        </div>
                    </div>
                    
                    <!-- Row 2: Actual Non-Crypto -->
                    <div class="grid grid-cols-3 border-t border-gray-800 items-center text-center py-4 bg-gray-900">
                        <div class="text-xs font-bold text-gray-300">Non-Crypto</div>
                        
                        <!-- False Positive -->
                        <div class="px-2">
                            <div class="bg-rose-950/40 text-rose-400 border border-rose-800/50 py-2 rounded-lg font-mono">
                                <span id="cm-fp" class="text-lg font-bold">0</span>
                                <span class="text-[10px] block font-semibold uppercase text-rose-500/70 tracking-tight">False Pos (FP)</span>
                            </div>
                        </div>
                        
                        <!-- True Negative -->
                        <div class="px-2">
                            <div class="bg-emerald-950/40 text-emerald-400 border border-emerald-800/50 py-2 rounded-lg font-mono">
                                <span id="cm-tn" class="text-lg font-bold">0</span>
                                <span class="text-[10px] block font-semibold uppercase text-emerald-500/70 tracking-tight">True Neg (TN)</span>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
            
        </div>
        
        <!-- Center Panel: Files List & Search -->
        <div class="w-full lg:w-1/4 border-r border-gray-800 flex flex-col overflow-hidden lg:h-[calc(100vh-80px)]">
            <div class="p-4 border-b border-gray-800 bg-gray-900 flex flex-col gap-3">
                <h2 class="text-base font-semibold text-white flex items-center gap-2">
                    <svg class="w-4.5 h-4.5 text-indigo-400" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M3 7v10a2 2 0 002 2h14a2 2 0 002-2V9a2 2 0 00-2-2h-6l-2-2H5a2 2 0 00-2 2z"></path></svg>
                    Evaluated Files
                </h2>
                
                <!-- Search -->
                <div class="relative">
                    <span class="absolute inset-y-0 left-0 pl-3 flex items-center pointer-events-none">
                        <svg class="h-4 w-4 text-gray-500" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z"></path></svg>
                    </span>
                    <input type="text" id="search-input" placeholder="Search files..." 
                           class="w-full bg-gray-950 border border-gray-850 rounded-lg pl-9 pr-3 py-2 text-sm text-gray-200 placeholder-gray-500 focus:outline-none focus:border-indigo-600 transition">
                </div>

                <!-- Group Filters -->
                <div class="flex flex-wrap gap-1.5 mt-1">
                    <button onclick="setFilterGroup('all')" id="btn-filter-all" class="text-[10px] px-2.5 py-1 rounded bg-indigo-600 text-white font-semibold">All</button>
                    <button onclick="setFilterGroup('novel_crypto')" id="btn-filter-novel_crypto" class="text-[10px] px-2.5 py-1 rounded bg-gray-800 hover:bg-gray-700 text-gray-300 font-semibold">Novel Crypto</button>
                    <button onclick="setFilterGroup('non_crypto_unrelated')" id="btn-filter-non_crypto_unrelated" class="text-[10px] px-2.5 py-1 rounded bg-gray-800 hover:bg-gray-700 text-gray-300 font-semibold">Unrelated</button>
                    <button onclick="setFilterGroup('non_crypto_discarded')" id="btn-filter-non_crypto_discarded" class="text-[10px] px-2.5 py-1 rounded bg-gray-800 hover:bg-gray-700 text-gray-300 font-semibold">Discarded</button>
                </div>
            </div>
            
            <!-- File Tree List -->
            <div id="file-list" class="flex-1 overflow-y-auto custom-scrollbar p-3 space-y-1 bg-gray-950">
                <!-- Populated dynamically by JavaScript -->
            </div>
        </div>

        <!-- Right Panel: Chunks heatmap & source codes -->
        <div class="flex-1 flex flex-col overflow-hidden lg:h-[calc(100vh-80px)] bg-gray-900">
            
            <!-- Dynamic File Header / No Selection Panel -->
            <div id="details-header" class="px-6 py-4 border-b border-gray-800 bg-gray-900 flex justify-between items-center">
                <div>
                    <h3 id="selected-filename" class="text-base font-bold text-white tracking-wide">No File Selected</h3>
                    <p id="selected-filegroup" class="text-xs text-gray-500 uppercase mt-0.5 font-mono">Select an evaluated file from the explorer list on the left to see its chunking layout.</p>
                </div>
                <div id="selected-filebadge" class="hidden"></div>
            </div>

            <div id="details-body" class="flex-1 flex flex-col md:flex-row overflow-hidden hidden">
                
                <!-- Chunks Heatmap list -->
                <div class="w-full md:w-1/3 border-r border-gray-800 flex flex-col overflow-hidden">
                    <div class="p-3 border-b border-gray-850 bg-gray-900/50 flex justify-between items-center">
                        <span class="text-xs font-bold text-gray-400 uppercase tracking-wider">File Chunks</span>
                        <span id="chunk-count" class="text-xs text-indigo-400 font-bold font-mono">0 Chunks</span>
                    </div>
                    <div id="chunks-container" class="flex-1 overflow-y-auto custom-scrollbar p-3 space-y-2 bg-gray-950/40">
                        <!-- Populated dynamically by JavaScript -->
                    </div>
                </div>

                <!-- Decoded code content panel -->
                <div class="flex-1 flex flex-col overflow-hidden bg-gray-900">
                    <div class="p-3 border-b border-gray-850 bg-gray-900/50 flex justify-between items-center">
                        <span class="text-xs font-bold text-gray-400 uppercase tracking-wider">Chunk Token Decoded Clear Text</span>
                        <span id="selected-chunk-label" class="text-xs font-bold font-mono text-indigo-400">No Chunk Active</span>
                    </div>
                    <div id="text-view-container" class="flex-1 p-6 overflow-y-auto custom-scrollbar font-mono text-xs text-gray-300 leading-relaxed select-text whitespace-pre bg-gray-950/70 border border-gray-900 rounded-lg m-4 shadow-inner">
                        <!-- Decoded chunk text populated dynamically by JavaScript -->
                        <div class="text-gray-600 italic h-full flex items-center justify-center">Click any chunk block on the left to view its decoded clear text representation.</div>
                    </div>
                </div>
                
            </div>
            
            <!-- Default Placeholder when no file is active -->
            <div id="details-placeholder" class="flex-1 flex flex-col items-center justify-center p-8 text-center bg-gray-950/30">
                <svg class="w-16 h-16 text-gray-700 mb-4" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M8 9l3 3-3 3m5 0h3M5 20h14a2 2 0 002-2V6a2 2 0 00-2-2H5a2 2 0 00-2 2v12a2 2 0 002 2z"></path></svg>
                <h4 class="text-lg font-bold text-gray-300">Visual Chunk Inspector</h4>
                <p class="text-sm text-gray-500 max-w-sm mt-1">Select any evaluated file in the sidebar explorer to examine how it was divided into chunks, each chunk's classifier probability, and its decoded raw token contents.</p>
            </div>
            
        </div>
        
    </main>

    <!-- Embedded Data Payload -->
    <script id="dashboard-payload" type="application/json">
__PAYLOAD_B64__
    </script>

    <!-- Dashboard Controller JS -->
    <script>
        // Load and decode embedded base64 data safely supporting full UTF-8 contents
        const b64Data = document.getElementById('dashboard-payload').textContent.trim();
        const binString = atob(b64Data);
        const bytes = Uint8Array.from(binString, (m) => m.codePointAt(0));
        const payloadRaw = new TextDecoder().decode(bytes);
        const payload = JSON.parse(payloadRaw);

        const files = payload.files;
        const stats = payload.stats;
        const config = payload.config;
        
        let currentThreshold = config.default_threshold;
        let activeFileId = null;
        let activeChunkIndex = null;
        let activeFilter = 'all'; // all, novel_crypto, non_crypto_unrelated, non_crypto_discarded
        let searchQuery = '';

        // UI Element References
        const thresholdSlider = document.getElementById('threshold-slider');
        const thresholdVal = document.getElementById('threshold-val');
        
        const metricF1 = document.getElementById('metric-f1');
        const metricPrecision = document.getElementById('metric-precision');
        const metricRecall = document.getElementById('metric-recall');
        
        const cmTp = document.getElementById('cm-tp');
        const cmFn = document.getElementById('cm-fn');
        const cmFp = document.getElementById('cm-fp');
        const cmTn = document.getElementById('cm-tn');
        
        const fileListContainer = document.getElementById('file-list');
        const searchInput = document.getElementById('search-input');
        
        const detailsBody = document.getElementById('details-body');
        const detailsPlaceholder = document.getElementById('details-placeholder');
        const selectedFilename = document.getElementById('selected-filename');
        const selectedFilegroup = document.getElementById('selected-filegroup');
        const selectedFilebadge = document.getElementById('selected-filebadge');
        const chunkCount = document.getElementById('chunk-count');
        const chunksContainer = document.getElementById('chunks-container');
        const textViewContainer = document.getElementById('text-view-container');
        const selectedChunkLabel = document.getElementById('selected-chunk-label');

        // Set Slider default value
        thresholdSlider.value = currentThreshold;
        thresholdVal.textContent = currentThreshold.toFixed(3);

        // Slider Input Event
        thresholdSlider.addEventListener('input', (e) => {
            currentThreshold = parseFloat(e.target.value);
            thresholdVal.textContent = currentThreshold.toFixed(3);
            updateDashboard();
        });

        // Search Input Event
        searchInput.addEventListener('input', (e) => {
            searchQuery = e.target.value.toLowerCase();
            renderFileList();
        });

        // Function to set and sync slider
        window.setThreshold = function(val) {
            currentThreshold = val;
            thresholdSlider.value = val;
            thresholdVal.textContent = val.toFixed(3);
            updateDashboard();
        }

        window.setFilterGroup = function(group) {
            activeFilter = group;
            // Update filter button styles
            const buttons = ['all', 'novel_crypto', 'non_crypto_unrelated', 'non_crypto_discarded'];
            buttons.forEach(b => {
                const btn = document.getElementById('btn-filter-' + b);
                if (b === group) {
                    btn.className = 'text-[10px] px-2.5 py-1 rounded bg-indigo-600 text-white font-semibold';
                } else {
                    btn.className = 'text-[10px] px-2.5 py-1 rounded bg-gray-800 hover:bg-gray-700 text-gray-300 font-semibold';
                }
            });
            renderFileList();
        }

        // Calculate metrics based on current threshold
        function calculateCurrentMetrics() {
            let tp = 0; // Novel Crypto with max_score >= threshold
            let fn = 0; // Novel Crypto with max_score < threshold
            let unrelated_fp = 0; // non_crypto_unrelated with max_score >= threshold
            let unrelated_tn = 0; // non_crypto_unrelated with max_score < threshold
            let discarded_fp = 0; // non_crypto_discarded with max_score >= threshold
            let discarded_tn = 0; // non_crypto_discarded with max_score < threshold
            
            Object.values(files).forEach(file => {
                const maxScore = file.max_score;
                const isPredCrypto = maxScore >= currentThreshold;
                
                if (file.group === 'novel_crypto') {
                    if (isPredCrypto) tp++; else fn++;
                } else if (file.group === 'non_crypto_unrelated') {
                    if (isPredCrypto) unrelated_fp++; else unrelated_tn++;
                } else if (file.group === 'non_crypto_discarded') {
                    if (isPredCrypto) discarded_fp++; else discarded_tn++;
                }
            });
            
            const totalFp = unrelated_fp + discarded_fp;
            const totalTn = unrelated_tn + discarded_tn;
            
            const precision = tp + totalFp > 0 ? tp / (tp + totalFp) : 0;
            const recall = tp + fn > 0 ? tp / (tp + fn) : 0;
            const f1 = precision + recall > 0 ? (2 * precision * recall) / (precision + recall) : 0;
            
            return {
                tp, fn, fp: totalFp, tn: totalTn, precision, recall, f1
            };
        }

        // Update counts, matrix and gauges
        function updateDashboard() {
            const m = calculateCurrentMetrics();
            
            // Stats Panels
            metricF1.textContent = (m.f1 * 100).toFixed(2) + '%';
            metricPrecision.textContent = (m.precision * 100).toFixed(2) + '%';
            metricRecall.textContent = (m.recall * 100).toFixed(2) + '%';
            
            // Confusion Matrix
            cmTp.textContent = m.tp;
            cmFn.textContent = m.fn;
            cmFp.textContent = m.fp;
            cmTn.textContent = m.tn;
            
            // Refresh list to update correct/incorrect markers and borders
            renderFileList();
            
            // Refresh file details if selected to update badges
            if (activeFileId) {
                updateDetailsPanel();
            }
        }

        // Render File Sidebar
        function renderFileList() {
            fileListContainer.innerHTML = '';
            
            // Filter and sort files
            const filteredFiles = Object.values(files).filter(file => {
                // Group filter
                if (activeFilter !== 'all' && file.group !== activeFilter) return false;
                // Search query filter
                if (searchQuery && !file.filename.toLowerCase().includes(searchQuery)) return false;
                return true;
            }).sort((a, b) => b.max_score - a.max_score); // Sort by highest probability first

            if (filteredFiles.length === 0) {
                fileListContainer.innerHTML = `<div class="text-center py-8 text-gray-500 italic text-sm">No files found matching criteria.</div>`;
                return;
            }

            filteredFiles.forEach(file => {
                const maxScore = file.max_score;
                const isPredCrypto = maxScore >= currentThreshold;
                const isActualCrypto = file.group === 'novel_crypto';
                const isCorrect = isPredCrypto === isActualCrypto;
                
                // Styling classes
                let statusBorderColor = isCorrect ? 'border-emerald-900/40 hover:border-emerald-700/60 bg-emerald-950/10' : 'border-rose-950 hover:border-rose-800/60 bg-rose-950/10';
                let activeBgClass = activeFileId === file.filename ? 'ring-1 ring-indigo-500 border-indigo-500/80 bg-indigo-950/20' : '';
                
                const fileCard = document.createElement('button');
                fileCard.onclick = () => selectFile(file.filename);
                fileCard.className = `w-full p-2.5 rounded-lg border text-left flex justify-between items-center transition gap-3 overflow-hidden ${statusBorderColor} ${activeBgClass} mb-1.5`;
                
                let groupLabel = 'Crypto';
                let groupColor = 'text-indigo-400';
                if (file.group === 'non_crypto_unrelated') { groupLabel = 'Unrelated'; groupColor = 'text-gray-400'; }
                if (file.group === 'non_crypto_discarded') { groupLabel = 'Discarded'; groupColor = 'text-amber-400'; }

                fileCard.innerHTML = `
                    <div class="flex-1 min-w-0">
                        <div class="text-xs font-semibold truncate text-white" title="${file.filename}">${file.filename.split('/').pop()}</div>
                        <div class="text-[10px] ${groupColor} mt-0.5 font-medium flex items-center gap-1.5">
                            <span>${groupLabel}</span>
                            <span class="text-gray-600">•</span>
                            <span class="truncate max-w-[150px]">${file.filename}</span>
                        </div>
                    </div>
                    <div class="text-right flex flex-col items-end">
                        <span class="text-[11px] font-mono font-semibold ${maxScore >= currentThreshold ? 'text-indigo-400' : 'text-gray-400'}">${maxScore.toFixed(3)}</span>
                        <span class="text-[8px] font-bold uppercase tracking-wider px-1 py-0.5 rounded mt-0.5 ${isPredCrypto ? 'bg-indigo-900/50 text-indigo-300' : 'bg-gray-800 text-gray-400'}">
                            ${isPredCrypto ? 'Crypto' : 'Safe'}
                        </span>
                    </div>
                `;
                fileListContainer.appendChild(fileCard);
            });
        }

        // Select file event
        window.selectFile = function(filename) {
            activeFileId = filename;
            activeChunkIndex = 0; // default to first chunk
            
            // Show panel
            detailsPlaceholder.classList.add('hidden');
            detailsBody.classList.remove('hidden');
            
            // Highlight list item
            renderFileList();
            updateDetailsPanel();
        }

        // Update File inspection details (middle & right panel contents)
        function updateDetailsPanel() {
            const file = files[activeFileId];
            if (!file) return;
            
            const maxScore = file.max_score;
            const isPredCrypto = maxScore >= currentThreshold;
            const isActualCrypto = file.group === 'novel_crypto';
            const isCorrect = isPredCrypto === isActualCrypto;
            
            selectedFilename.textContent = file.filename;
            selectedFilename.title = file.filename;
            
            // Format Group Header
            let groupLabel = 'Novel Crypto Dataset';
            if (file.group === 'non_crypto_unrelated') groupLabel = 'Non-Crypto (Unrelated Dataset)';
            if (file.group === 'non_crypto_discarded') groupLabel = 'Non-Crypto (Discarded/API Dataset)';
            selectedFilegroup.textContent = `Actual: ${groupLabel} | Max Chunk Score: ${maxScore.toFixed(4)}`;

            // Render Header Status Badge
            selectedFilebadge.classList.remove('hidden');
            if (isCorrect) {
                selectedFilebadge.className = "text-xs font-bold bg-emerald-950 text-emerald-400 border border-emerald-800/80 px-3 py-1 rounded-full flex items-center gap-1.5";
                selectedFilebadge.innerHTML = `<span class="w-1.5 h-1.5 rounded-full bg-emerald-400"></span> Correctly Predicted: ${isPredCrypto ? 'Crypto' : 'Safe'}`;
            } else {
                selectedFilebadge.className = "text-xs font-bold bg-rose-950 text-rose-400 border border-rose-800/80 px-3 py-1 rounded-full flex items-center gap-1.5";
                selectedFilebadge.innerHTML = `<span class="w-1.5 h-1.5 rounded-full bg-rose-400"></span> Incorrectly Predicted: ${isPredCrypto ? 'Crypto' : 'Safe'}`;
            }

            // Render ChunksHeatmap Layout
            chunkCount.textContent = `${file.chunks.length} Chunks`;
            chunksContainer.innerHTML = '';
            
            file.chunks.forEach(chunk => {
                const chunkScore = chunk.probability;
                const isChunkCrypto = chunkScore >= currentThreshold;
                
                // Color heatmap bar class
                let barColor = 'bg-gray-800 border-gray-700';
                let fillBarColor = 'bg-indigo-600';
                let activeRing = '';
                
                if (chunkScore >= currentThreshold) {
                    barColor = 'bg-indigo-950/20 border-indigo-800/40';
                    fillBarColor = 'bg-indigo-500';
                }
                
                if (activeChunkIndex === chunk.index) {
                    activeRing = 'ring-1 ring-indigo-500 border-indigo-500 bg-indigo-950/50';
                }
                
                const chunkButton = document.createElement('button');
                chunkButton.onclick = () => selectChunk(chunk.index);
                chunkButton.className = `w-full p-2.5 rounded-lg border text-left transition ${barColor} ${activeRing}`;
                
                // HTML structure for chunk bar
                chunkButton.innerHTML = `
                    <div class="flex justify-between items-center text-[10px] font-bold text-gray-300 mb-1.5">
                        <span class="text-white">Chunk #${chunk.index + 1}</span>
                        <span class="font-mono ${isChunkCrypto ? 'text-indigo-400' : 'text-gray-400'}">${chunkScore.toFixed(4)}</span>
                    </div>
                    
                    <!-- Progress Bar Track -->
                    <div class="w-full bg-gray-900 rounded-full h-1.5 overflow-hidden">
                        <div class="h-1.5 rounded-full ${fillBarColor}" style="width: ${chunkScore * 100}%"></div>
                    </div>
                `;
                
                chunksContainer.appendChild(chunkButton);
            });

            // Load currently active chunk text
            updateChunkTextView();
        }

        // Select specific chunk
        window.selectChunk = function(idx) {
            activeChunkIndex = idx;
            // Update chunk borders
            updateDetailsPanel();
        }

        // Render decoded plain text of chunk with custom line formatting
        function updateChunkTextView() {
            const file = files[activeFileId];
            if (!file) return;
            const chunk = file.chunks[activeChunkIndex];
            if (!chunk) {
                textViewContainer.innerHTML = `<div class="text-gray-600 italic h-full flex items-center justify-center">Click any chunk block on the left to view its decoded clear text representation.</div>`;
                selectedChunkLabel.textContent = "No Chunk Active";
                return;
            }

            selectedChunkLabel.textContent = `Chunk #${chunk.index + 1} (Score: ${chunk.probability.toFixed(4)})`;

            // Render text with simulated line numbers safely supporting strings or arrays
            let rawText = '';
            if (chunk.clear_text !== undefined && chunk.clear_text !== null) {
                if (Array.isArray(chunk.clear_text)) {
                    rawText = chunk.clear_text.join('\\n');
                } else if (typeof chunk.clear_text === 'string') {
                    rawText = chunk.clear_text;
                } else {
                    rawText = String(chunk.clear_text);
                }
            }
            
            // Robust regex split for newlines, completely immune to raw string escape issues
            const lines = rawText.split(/\\r?\\n/);
            
            let html = '<table class="w-full select-text border-collapse">';
            lines.forEach((line, idx) => {
                // Handle empty spaces cleanly
                const formattedLine = line
                    .replace(/&/g, '&')
                    .replace(/</g, '<')
                    .replace(/>/g, '>')
                    .replace(/"/g, '"')
                    .replace(/'/g, '&#039;');
                
                html += `
                    <tr class="hover:bg-gray-900/30">
                        <td class="text-gray-600 font-mono text-[10px] text-right pr-4 select-none align-top w-12 border-r border-gray-800/40">${idx + 1}</td>
                        <td class="pl-4 font-mono text-[11px] whitespace-pre-wrap break-all text-gray-300">${formattedLine || ' '}</td>
                    </tr>
                `;
            });
            html += '</table>';
            
            textViewContainer.innerHTML = html;
        }

        // Initial launch initialization
        updateDashboard();
        
    </script>
</body>
</html>
"""
        # Inject values using basic string replace (completely bypasses Python f-string bracket interpretation conflict)
        html_content = html_template.replace("__PAYLOAD_B64__", payload_b64)
        html_content = html_content.replace("__REPRESENTATION__", config.REPRESENTATION)
        html_content = html_content.replace("__CLASSIFIER__", config.CLASSIFIER)
        
        html_content = html_content.replace("__BEST_F1_THRESHOLD__", str(stats["best_f1_threshold"]))
        html_content = html_content.replace("__BEST_F1_THRESHOLD_STR__", f"{stats['best_f1_threshold']:.3f}")
        
        html_content = html_content.replace("__MIN_FN_THRESHOLD__", str(stats["min_fn_threshold"]))
        html_content = html_content.replace("__MIN_FN_THRESHOLD_STR__", f"{stats['min_fn_threshold']:.3f}")
        
        html_content = html_content.replace("__MIN_FP_THRESHOLD__", str(stats["min_fp_threshold"]))
        html_content = html_content.replace("__MIN_FP_THRESHOLD_STR__", f"{stats['min_fp_threshold']:.3f}")

        os.makedirs(os.path.dirname(config.EVALUATION_RESULT_PATH), exist_ok=True)
        report_path = f"{config.EVALUATION_RESULT_PATH}evaluation_report.html"
        
        with open(report_path, "w", encoding="utf-8") as f:
            f.write(html_content)
        
        print(f"Generated gorgeous interactive HTML evaluation report: {report_path}")