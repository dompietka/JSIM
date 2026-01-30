import requests
from bs4 import BeautifulSoup
import pandas as pd
from io import StringIO
import re
from collections import Counter
import json
import os 

class WikiArticle:
    def __init__(self, html_content):
        self.soup = BeautifulSoup(html_content, 'html.parser')
        
    def _get_content_div(self):
        content_div = self.soup.find('div', class_='mw-parser-output')
        
        if not content_div:
            #potencjalnie inna struktura
            content_div = self.soup.find('div', id='mw-content-text')

        if not content_div:
            return None
        return content_div

    def get_summary(self):
        content_div = self._get_content_div()
        if not content_div:
            return "Error: Could not find content in the article."

        paragraphs = content_div.find_all('p', recursive=False) #wszystkie paragrafy
        
        for p in paragraphs:
            text = p.get_text().strip()
            if text: #pierwszy niepusty paragraf
                return text
        
        return "Info: No summary paragraph found."
    
    def get_table(self, phrase, table_number=1, first_row_is_header=False, first_column_is_not_index=False):
        tables = self.soup.find_all('table')
        
        if len(tables) < table_number:
            return f"Error: Article contains only {len(tables)} tables."
        
        table = tables[table_number - 1] #indeksowanie od 0
       
        try:
            header_arg = 0 if first_row_is_header else None
            html_str = str(table)
            dfs = pd.read_html(StringIO(html_str), header=header_arg, flavor='bs4') #pandas parsuje stringa z htmlem, stringio wynika z futurewarning
            
            if not dfs:
                return "table could not be parsed."
                
            df = dfs[0]
            if not df.empty and not first_column_is_not_index:
                df = df.set_index(df.columns[0])

        except Exception as e:
            return f"Error parsing table: {e}"

        filename = f"{phrase}.csv"
        try:
            df.to_csv(filename)
            print(f"Data saved to file: {filename}")
        except IOError as e:
            print(f"Error saving file: {e}")

        value_counts = df.stack().value_counts().sort_index().reset_index() #zlicza, sortuje i przywraca do pandasowej ladnej formy
        value_counts.columns = ['Wartość', 'Liczba wystąpień']

        if value_counts.empty:
            return "No values found in the table."
        else:
            return value_counts.to_string(index=False)
        
        
    def count_words(self):
        content_div = self._get_content_div()
        if not content_div:
            return "Error: Could not find content in the article."
        selectors_to_remove = [
            'script',                   # Skrypty JS
            'style',                    # Style CSS
            '.mw-collapsible-toggle',   # Przyciski "Expand", "Show", "Hide"
            '.mw-editsection',          # Linki "[edit]" przy nagłówkach
            '#toc',                     # Spis treści (Table of Contents)
            '.shortcutbox',             # Ramki "Shortcuts: XYZ"
            '#catlinks',                # Pasek kategorii na samym dole
            '.navbox',                  # Paski nawigacyjne na dole strony
            '.printfooter',             # Stopka "Retrieved from..."
            '.roundy.shortcut',         # Alternatywne ramki skrótów na Bulbapedii
        ]

        # Iterujemy po selektorach i usuwamy pasujące elementy
        for selector in selectors_to_remove:
            # soup.select() pozwala szukać po klasach CSS (kropka) i ID (hash)
            for element in content_div.select(selector):
                element.decompose()
                
        text = content_div.get_text(" ", strip=True)
        
        words = re.findall(r'\w+', text.lower()) #wszystkie slowa, mala litera
    
        current_counts = Counter(words)
        word_count = sum(current_counts.values())
        words_used = len(current_counts)

        json_file = "word-counts.json"
        total_counts = {}

        if os.path.exists(json_file): 
            try:
                with open(json_file, 'r', encoding='utf-8') as f: 
                    total_counts = json.load(f) 
            except json.JSONDecodeError:
                return "Error: JSON file is corrupted, Article word count: {word_count}, unique words: {words_used}."
            
        for word, count in current_counts.items(): 
            total_counts[word] = total_counts.get(word, 0) + count 
            
        try:
            with open(json_file, 'w', encoding='utf-8') as f: 
                json.dump(total_counts, f, ensure_ascii=False, indent=4)
        except IOError as e:
            return f"Error saving JSON file: {e}, Article word count: {word_count}, unique words: {words_used}."

        return f"Article word count: {word_count}, unique words: {words_used}, word-counts.json updated successfully."



