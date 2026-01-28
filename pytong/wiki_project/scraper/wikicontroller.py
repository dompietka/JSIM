import argparse
import sys
from scraper.wikiarticle import WikiArticle
from scraper.wikifetcher import WikiFetcher

class WikiController:
    def __init__(self, base_url):
        self.scraper = WikiFetcher(base_url=base_url)

    def run(self):
        parser = argparse.ArgumentParser(description="WikiController - manage wiki scraping tasks.")
        
        parser.add_argument('--summary', type=str, help='get first paragraph summary of the article for the given phrase')
        
        args = parser.parse_args()
        
        if args.summary:
            phrase = args.summary
            html_content = self.scraper.scrape(phrase)
            if html_content:
                article = WikiArticle(html_content)
                summary = article.get_summary()
                print(f"Summary for '{phrase}':\n{summary}")
            else:
                print(f"Could not retrieve content for '{phrase}'.")
       