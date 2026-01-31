import sys
import os
from wiki_scraper import main as wiki_scraper_main


def main():
    try:
        text = wiki_scraper_main(
            [
                "--use-local-file",
                "data/arwen_owtrta.html",
                "--url",
                "https://lotr.fandom.com/wiki/",
                "--summary",
                "arwen",
            ]
        )
        assert text.startswith("Arwen"), "Tekst nie zaczyna się od 'Arwen'"
        assert text.endswith("She was often called Arwen Undómiel or \"Evenstar\"."), "Tekst nie kończy się oczekiwanym zdaniem"


        print("TEST PASSED: Tekst został poprawnie wyodrębniony.")
        sys.exit(0)  

    except Exception as e:
        print(f"TEST FAILED: {e}")
        sys.exit(1)  

if __name__ == "__main__":
    main()
