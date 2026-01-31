import json
from collections import deque
import time
import requests
from scraper.wikiarticle import WikiArticle

class WikiBfs:
    def __init__(self, fetcher, phrase, depth, wait):
        self.fetcher = fetcher
        self.phrase = phrase
        self.depth = depth
        self.wait = wait
        self.visited = set()
        self.queue = deque()

    def bfs_count(self):
        self.queue.append((self.phrase, 0))  # (phrase, current_depth)
        self.visited.add(self.phrase)
      
        while self.queue:
            current_phrase, current_depth = self.queue.popleft()
            print(f"Visiting: {current_phrase} at depth {current_depth}")

            html_content = self.fetcher.get_html(current_phrase)
            if not html_content:
                print(f"Could not retrieve content for '{current_phrase}'. Skipping.")
                continue

            article = WikiArticle(html_content, phrase=current_phrase)
            article.count_words()

            if current_depth < self.depth:
                content_div = article._get_content_div()
                if content_div:
                    links = article._get_links_from_content(content_div)
                    for link in links:
                        if link not in self.visited:
                            self.visited.add(link)
                            self.queue.append((link, current_depth + 1))

            time.sleep(self.wait) 
        return "BFS word counting completed."