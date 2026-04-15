from scraper.wikiarticle import WikiArticle
from scraper.wikifetcher import WikiFetcher
from scraper.wikianalyzer import analyze_relative_word_frequency
from scraper.wikibfs import WikiBfs


class WikiController:
    def __init__(self, args):
        self.args = args
        self.phrase = (
            args.summary or args.table or args.count_words or args.auto_count_words
        )

        if self.args.use_local_file is not None:
            self.use_local_file = True
            self.fetcher = WikiFetcher(base_url=args.url, use_local_file=True)
        else:
            self.use_local_file = False
            self.fetcher = WikiFetcher(base_url=args.url)

    def run(self):
        if self.args.clear_json:
            with open(self.args.json_path, "w") as f:
                f.write("{}")  # Clear the file content
            print(f"{self.args.json_path} cleared.")

        

        DISPATCH = {
            "summary": lambda: article.get_summary(),
            "table": lambda: article.get_table(
                phrase=self.phrase,
                table_number=self.args.number,
                first_row_is_header=self.args.first_row_is_header,
                first_column_is_not_index=self.args.first_column_is_not_index,
            ),
            "count_words": lambda: article.count_words(json_file=self.args.json_path),
            "auto_count_words": lambda: WikiBfs(
                fetcher=self.fetcher,
                phrase=self.phrase,
                depth=self.args.depth,
                wait=self.args.wait,
            ).bfs_count(),
        }

        for name, value in vars(self.args).items():
            if name in DISPATCH and value is not None:
                if self.args.use_local_file:
                    html_content = self.fetcher.get_html(
                        phrase=self.phrase, local_file_path=self.args.use_local_file
                    )
                else:
                    html_content = self.fetcher.get_html(phrase=self.phrase)

                if html_content:
                    article = WikiArticle(html_content, phrase=self.phrase)
                else:
                    return(f"Could not retrieve content for '{self.phrase}'.")
                print(f"tryb: {name}, fraza: {value}")
                res = DISPATCH[name]()
                break
            if name == "analyze_relative_word_frequency" and value:
                res = analyze_relative_word_frequency(self.args)
                break

        return(res)
