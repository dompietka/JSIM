import argparse
import sys
from scraper.wikiarticle import WikiArticle
from scraper.wikifetcher import WikiFetcher

class WikiController:
    def __init__(self, args):
            self.args = args
            self.phrase = args.summary or args.table or args.count_words or args.auto_count_words
            if self.args.use_local_file is not None:
                self.use_local_file = True
                self.scraper = WikiFetcher(base_url=args.url, phrase=self.phrase, use_local_file=True)
            else:
                self.use_local_file = False
                self.scraper = WikiFetcher(base_url=args.url, phrase=self.phrase)

    def run(self):
        if self.args.use_local_file:
            html_content = self.scraper.scrape(local_file_path=self.args.use_local_file)
        else:
            html_content = self.scraper.scrape()
        if html_content:
            article = WikiArticle(html_content)
        else:
            print(f"Could not retrieve content for '{html_content}'.")

        for arg, value in vars(self.args).items():
            print(f"{arg}: {value}")