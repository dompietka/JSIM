import requests
from bs4 import BeautifulSoup
import pandas as pd
from io import StringIO

class WikiArticle:
    def __init__(self, html_content):
        self.soup = BeautifulSoup(html_content, 'html.parser')

    def get_summary(self):
        content_div = self.soup.find('div', class_='mw-parser-output')
        
        if not content_div:
            #potencjalnie inna struktura
            content_div = self.soup.find('div', id='mw-content-text')

        if not content_div:
            return "Error: Could not find article content structure."

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
            dfs = pd.read_html(StringIO(html_str), header=header_arg, flavor='bs4')
            
            if not dfs:
                return "table could not be parsed."
                
            df = dfs[0]
            if not df.empty and not first_column_is_not_index:
                df = df.set_index(df.columns[0])

        except Exception as e:
            return f"Error parsing table: {e}"

        # 5. Zapisanie do pliku CSV
        filename = f"{phrase}.csv"
        try:
            df.to_csv(filename)
            print(f"Data saved to file: {filename}")
        except IOError as e:
            print(f"Error saving file: {e}")

        
     
        value_counts = df.stack().value_counts().sort_index().reset_index()
        value_counts.columns = ['Wartość', 'Liczba wystąpień']
        

        if value_counts.empty:
            return "No values found in the table."
        else:
            return value_counts.to_string(index=False)
