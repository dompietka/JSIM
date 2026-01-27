# wiki_scraper.py
import argparse
import sys
from scraper.wikiscraper import WikiScraper

# Stała konfiguracja dla Bulbapedii
BULBAPEDIA_URL = "https://bulbapedia.bulbagarden.net/wiki/"

def main():
    parser = argparse.ArgumentParser(description="WikiScraper - pobieranie i analiza danych z Wiki.")
    
    # Argument: --summary
    parser.add_argument('--summary', type=str, help='Pobiera i wyświetla pierwszy paragraf artykułu dla podanej frazy.')
    
    # Tutaj w przyszłości dodamy kolejne argumenty (--table, --count-words itp.)

    args = parser.parse_args()

    # Inicjalizacja środowiska
    scraper = WikiScraper(base_url=BULBAPEDIA_URL)

    if args.summary:
        phrase = args.summary
        print(f"Fetching summary for: {phrase}...")
        
        article = scraper.scrape(phrase)
        
        if article:
            summary = article.get_summary()
            print("-" * 40)
            print(summary)
            print("-" * 40)
            
            # Licencja (wymóg zadania)
            print(f"Wyjście programu na licencji BY-NC-SA stworzone na podstawie artykułu dostępnego na stronie {BULBAPEDIA_URL}{phrase.replace(' ', '_')}")
    
    else:
        parser.print_help()

if __name__ == "__main__":
    main()