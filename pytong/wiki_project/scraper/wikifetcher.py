import requests

class WikiFetcher:
    def __init__(self, base_url, phrase, use_local_file=False):
        self.base_url = base_url
        self.phrase = phrase
        self.use_local_file = use_local_file #czytanie z pliku lokalnego, do testowania offline

    def _get_url(self): #zamienia spacje na podlogi
        safe_phrase = self.phrase.replace(" ", "_")
        return f"{self.base_url}{safe_phrase}"

    def get_html(self, local_file_path=None):
        html_content = "" #string

        if self.use_local_file:
            if not local_file_path:
                raise ValueError("Local file path must be provided in local mode.")
            try:
                with open(local_file_path, 'r', encoding='utf-8') as f:
                    html_content = f.read()
            except FileNotFoundError:
                print(f"Error: File {local_file_path} not found.")
                return None 
        else:
            url = self._get_url()
            try:
                headers = {
                    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
                    "AppleWebKit/537.36 (KHTML, like Gecko) "
                    "Chrome/119.0 Safari/537.36",
                    "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
                    "Accept-Language": "en-US,en;q=0.5",
                    "Referer": "https://lotr.fandom.com/",
                    "Connection": "keep-alive"
}

                response = requests.get(url, headers=headers) #wysyla zapytanie HTTP GET 
                
                if response.status_code == 404:
                    print(f"Error: Article '{self.phrase}' not found (404).")
                    return None
                
                response.raise_for_status() #inne bledy HTTP
                html_content = response.text
                  
            except requests.RequestException as e:
                print(f"Network error: {e}")
                return None
        return html_content