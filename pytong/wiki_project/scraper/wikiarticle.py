import requests
from bs4 import BeautifulSoup
import os

class WikiArticle:
    """
    Klasa reprezentująca pojedynczy artykuł z wiki.
    Odpowiada za parsowanie HTML i wyciąganie konkretnych danych.
    """
    def __init__(self, html_content):
        self.soup = BeautifulSoup(html_content, 'html.parser')

    def get_summary(self):
        """
        Znajduje pierwszy paragraf artykułu i zwraca jego czysty tekst.
        Dla Bulbapedii szukamy wewnątrz div z contentem.
        """
        # Specyfika Bulbapedii: treść jest w divie mw-parser-output
        # (często jest to też klasa .mw-content-ltr)
        content_div = self.soup.find('div', class_='mw-parser-output')
        
        if not content_div:
            # Fallback dla standardowych MediaWiki, jeśli klasa jest inna
            content_div = self.soup.find('div', id='mw-content-text')

        if not content_div:
            return "Error: Could not find article content structure."

        # Szukamy pierwszego paragrafu <p>
        # Uwaga: Czasami pierwszy <p> jest pusty lub techniczny, 
        # więc szukamy pierwszego, który ma sensowny tekst.
        paragraphs = content_div.find_all('p', recursive=False)
        
        for p in paragraphs:
            text = p.get_text().strip()
            if text:
                return text
        
        return "Info: No summary paragraph found."