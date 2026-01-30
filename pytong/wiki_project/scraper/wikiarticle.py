import requests
from bs4 import BeautifulSoup
import os

class WikiArticle:
    def __init__(self, html_content):
        self.soup = BeautifulSoup(html_content, 'html.parser')

    def get_summary(self):
        print("Pobieranie streszczenia artykułu...")
        content_div = self.soup.find('div', class_='mw-parser-output')
        
        if not content_div:
            #potencjalnie inna struktura
            content_div = self.soup.find('div', id='mw-content-text')

        if not content_div:
            print("Błąd: Nie można znaleźć struktury treści artykułu.")
            return "Error: Could not find article content structure."

        paragraphs = content_div.find_all('p', recursive=False) #wszystkie paragrafy
        
        for p in paragraphs:
            text = p.get_text().strip()
            if text: #pierwszy niepusty paragraf
                print(text)
                return
        
        return "Info: No summary paragraph found."