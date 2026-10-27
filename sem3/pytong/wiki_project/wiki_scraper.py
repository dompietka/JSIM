import argparse
from scraper.wikicontroller import WikiController

# Stała konfiguracja dla uncyclopedii
UNCYCLOPEDIA = "https://uncyclopedia.com/wiki/"


def main(argss=None):
    parser = argparse.ArgumentParser(
        description="WikiScraper - data extraction and analysis from wikis."
    )

    parser.add_argument(
        "--url",
        type=str,
        help="base URL of the wiki to scrape (default: uncyclopedia)",
        default=UNCYCLOPEDIA,
    )
    modes = parser.add_mutually_exclusive_group(required=True)

    modes.add_argument(
        "--summary",
        metavar='"PHRASE"',
        type=str,
        help="get first paragraph summary of the article for the given phrase",
    )
    modes.add_argument(
        "--table",
        metavar='"PHRASE"',
        type=str,
        help="extract table data from the article for the given phrase and save as CSV",
    )
    modes.add_argument(
        "--count-words",
        metavar='"PHRASE"',
        type=str,
        help="Count words in article and update word-counts.json",
    )
    modes.add_argument(
        "--analyze-relative-word-frequency",
        action="store_true",
        help="Analyze relative word frequency vs wiki language",
    )
    modes.add_argument(
        "--auto-count-words",
        metavar='"PHRASE"',
        type=str,
        help="Automatically crawl linked articles and count words",
    )

    parser.add_argument(
        "--number", type=int, default=1, help="Table number (starting from 1)"
    )
    parser.add_argument(
        "--first-row-is-header",
        action="store_true",
        help="Treat first row of table as header",
    )
    parser.add_argument(
        "--first-column-is-not-index",
        action="store_true",
        help="Do not treat first column of table as index",
    )

    parser.add_argument(
        "--mode",
        choices=["article", "language"],
        help="Sorting mode for frequency analysis",
    )
    parser.add_argument(
        "--count", type=int, help="Number of words to include in analysis"
    )
    parser.add_argument("--chart", type=str, help="Path to save bar chart image")

    parser.add_argument("--depth", type=int, help="Crawling depth")
    parser.add_argument(
        "--wait", type=float, default=1.0, help="Seconds to wait between requests"
    )

    parser.add_argument(
        "--use-local-file",
        metavar="FILE_PATH",
        type=str,
        help="Use local HTML file for testing instead of making HTTP requests",
    )
    parser.add_argument(
        "--language",
        type=str,
        help="Language code for frequency analysis (default: 'en')",
    )

    parser.add_argument(
        "--clear-json",
        action="store_true",
        help="Clear word-counts.json before counting words",
    )
    
    parser.add_argument(
        "--json-path",
        type=str,
        default="word-counts.json",
        help="Path to the JSON file for storing word counts (default: word-counts.json)",
    )

    args = parser.parse_args(argss)
    app = WikiController(args)
    
    if argss is None:  # wywołanie z terminala
        print(app.run())
        print(
        f"Wyjście programu na licencji CC-BY-SA stworzone na podstawie artykułu dostępnego na stronie {args.url.replace(' ', '_')}"
    )
    return app.run()



if __name__ == "__main__":
    main()
