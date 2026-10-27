import requests


class WikiFetcher:
    def __init__(self, base_url, use_local_file=False):
        self.base_url = base_url
        self.use_local_file = (
            use_local_file  # czytanie z pliku lokalnego, do testowania offline
        )

    def _get_url(self, phrase):  # zamienia spacje na podlogi i skleja z baza URL
        safe_phrase = phrase.replace(" ", "_")
        return f"{self.base_url}{safe_phrase}"

    def get_html(self, phrase, local_file_path=None):
        html_content = ""  # string

        if self.use_local_file:
            if not local_file_path:
                raise ValueError("Local file path must be provided in local mode.")
            try:
                with open(local_file_path, "r", encoding="utf-8") as f:
                    html_content = f.read()
            except FileNotFoundError:
                print(f"Error: File {local_file_path} not found.")
                return None
        else:
            url = self._get_url(phrase)
            try:
                headers = {"User-Agent": "uni-project-wiki-scraper "}

                response = requests.get(
                    url, headers=headers
                )  # wysyla zapytanie HTTP GET

                if response.status_code == 404:
                    print(f"Error: Article '{phrase}' not found (404).")
                    return None

                response.raise_for_status()  # inne bledy HTTP
                html_content = response.text

            except requests.RequestException as e:
                print(f"Network error: {e}")
                return None
        return html_content
