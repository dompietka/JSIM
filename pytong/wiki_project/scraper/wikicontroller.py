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
                self.fetcher = WikiFetcher(base_url=args.url, phrase=self.phrase, use_local_file=True)
            else:
                self.use_local_file = False
                self.fetcher = WikiFetcher(base_url=args.url, phrase=self.phrase)

    def run(self):
        if self.args.use_local_file:
            html_content = self.fetcher.get_html(local_file_path=self.args.use_local_file)
        else:
            html_content = self.fetcher.get_html()
            
        if html_content:
            article = WikiArticle(html_content)
        else:
            print(f"Could not retrieve content for '{self.phrase}'.")
            
        DISPATCH = {
            "summary": lambda: article.get_summary(),
            "table": lambda: article.get_table(phrase=self.phrase, table_number=self.args.number, first_row_is_header=self.args.first_row_is_header, first_column_is_not_index=self.args.first_column_is_not_index),
            "count_words": lambda: article.count_words(),
            "analyze_relative_word_frequency": lambda: article.analyze_relative_word_frequency(),
            "auto_count_words": lambda: article.auto_count_words()
        }

        for name, value in vars(self.args).items():
            if name in DISPATCH and value is not None:
                print(f"tryb: {name}, fraza: {value}")
                res = DISPATCH[name]()
                break
        
        print(res)     
                
