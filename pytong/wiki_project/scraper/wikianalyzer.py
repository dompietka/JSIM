import pandas as pd
import json
from wordfreq import zipf_frequency, top_n_list
import matplotlib.pyplot as plt
import numpy as np


def _normalize_zipf_freq(zipf_freq):
    if zipf_freq > 0:
        return 10 ** (zipf_freq-2) #zipf -> na milion słów
    else:
        return float('nan')

def _normalize_count(count, total_words):
    return (count / total_words) * 1_000_000  # na milion słów

def analyze_relative_word_frequency(args):
    mode = args.mode
    count = args.count
    chart_path = args.chart
    language = args.language or "en"
    
    if not mode or not count:
        return "Error: --mode and --count arguments are required for relative word frequency analysis."
    
    with open("word-counts.json", "r", encoding="utf-8") as f:
        data = json.load(f)
        
    if data is {}:
        return "Error: no data in word-counts.json."
    
    total_words = sum(data.values())
    
    if mode == "article":
        df = pd.DataFrame(
            data.items(),
            columns=["word", "count"]
        )
        df["frequency_in_the_article"] = df["count"].apply(lambda x: _normalize_count(x, total_words))
        res = df.drop(columns=["count"]).sort_values(by="frequency_in_the_article", ascending=False).head(count)
        res["frequency_in_wiki_language"] = res["word"].apply(lambda w: _normalize_zipf_freq(zipf_frequency(w, language)))
    
    elif mode == "language":
        res = top_n_list(language, count)
        df = pd.DataFrame(
            res,
            columns=["word"]
        )
        df["frequency_in_wiki_language"] = df["word"].apply(lambda w: _normalize_zipf_freq(zipf_frequency(w, language)))
        df["count"] = df["word"].apply(lambda w: data.get(w, float('nan')))
        df["frequency_in_the_article"] = df["count"].apply(lambda x: _normalize_count(x, total_words))
        res = df.drop(columns=["count"]).sort_values(by="frequency_in_wiki_language", ascending=False)
        
    if chart_path:
        plot_data = res.fillna(0) 
        plt.figure(figsize=(10, 6))
        
        indices = np.arange(len(plot_data))
        bar_width = 0.35  # Troszkę szersze słupki są ładniejsze

        # Rysowanie słupków
        plt.bar(indices, plot_data["frequency_in_the_article"], bar_width, label='Frequency in Article', color='pink')
        plt.bar(indices + bar_width, plot_data["frequency_in_wiki_language"], bar_width, label='Frequency in Wiki Language', color='turquoise')

        # Etykiety i tytuły
        plt.xlabel('Words')
        plt.ylabel('Frequency (per million words)')
        plt.title('Relative Word Frequency Analysis')

        # Opcjonalnie: Skala logarytmiczna, jeśli różnice są ogromne
        # plt.yscale('log') 

        # Ustawienie etykiet na osi X
        plt.xticks(indices + bar_width / 2, plot_data["word"], rotation=45)
        plt.legend()
        plt.tight_layout()
        
        try:
            plt.savefig(chart_path)
            print(f"Bar chart saved to {chart_path}")
        except IOError as e:
            print(f"Error saving chart: {e}")

    return res.to_string(na_rep='')