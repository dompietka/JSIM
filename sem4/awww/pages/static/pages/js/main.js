"use strict";
(() => {
  var __getOwnPropNames = Object.getOwnPropertyNames;
  var __esm = (fn, res) => function __init() {
    return fn && (res = (0, fn[__getOwnPropNames(fn)[0]])(fn = 0)), res;
  };
  var __commonJS = (cb, mod) => function __require() {
    return mod || (0, cb[__getOwnPropNames(cb)[0]])((mod = { exports: {} }).exports, mod), mod.exports;
  };

  // pages/static/pages/ts/api.ts
  function getCsrfToken() {
    const match = document.cookie.match(/csrftoken=([^;]+)/);
    return match ? match[1] : "";
  }
  async function apiFetch(method, path, body) {
    try {
      const options = {
        method,
        headers: {
          "Content-Type": "application/json",
          "X-CSRFToken": getCsrfToken()
        }
      };
      if (body !== void 0) {
        options.body = JSON.stringify(body);
      }
      const res = await fetch(path, options);
      if (!res.ok) {
        let message = `HTTP ${res.status}`;
        try {
          const err = await res.json();
          message = err.error ?? message;
        } catch {
        }
        return { ok: false, error: message, status: res.status };
      }
      if (res.status === 204) {
        return { ok: true, data: {} };
      }
      const data = await res.json();
      return { ok: true, data };
    } catch (err) {
      return { ok: false, error: String(err) };
    }
  }
  var BlogApiClient, api;
  var init_api = __esm({
    "pages/static/pages/ts/api.ts"() {
      "use strict";
      BlogApiClient = class {
        async getPosts(search) {
          const qs = search ? `?search=${encodeURIComponent(search)}` : "";
          return apiFetch("GET", `/api/osts/${qs}`);
        }
        async getPost(id) {
          return apiFetch("GET", `/api/osts/${id}/`);
        }
        async deleteComment(postId, commentId) {
          return apiFetch("DELETE", `/api/posts/${postId}/comments/${commentId}/`);
        }
        async getComments(postId) {
          return apiFetch("GET", `/api/posts/${postId}/comments/`);
        }
        async createComment(postId, payload) {
          return apiFetch("POST", `/api/posts/${postId}/comments/`, payload);
        }
        async updatePost(id, payload) {
          return apiFetch("PATCH", `/api/posts/${id}/`, payload);
        }
      };
      api = new BlogApiClient();
    }
  });

  // pages/static/pages/ts/main.ts
  var require_main = __commonJS({
    "pages/static/pages/ts/main.ts"() {
      init_api();
      var sortSelect = getElement("sort-select");
      console.log("Hello, World!");
      function getElement(id) {
        const el = document.getElementById(id);
        if (el === null) {
          throw new Error(`Element #${id} not found`);
        }
        return el;
      }
      function escapeRegExp(str) {
        return str.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
      }
      function highlightMatch(text, query) {
        if (!query) return text;
        const safeQuery = escapeRegExp(query);
        const regex = new RegExp(safeQuery, "gi");
        console.log("Highlighting matches for query:", query);
        return text.replace(regex, (match) => `<mark>${match}</mark>`);
      }
      var searchInput = getElement("search-input");
      var postList = getElement("post-list");
      var statusBar = getElement("status-bar");
      async function fetchPosts(query = "") {
        const url = query ? `/api/posts/?search=${encodeURIComponent(query)}` : "/api/posts/";
        const res = await fetch(url);
        if (!res.ok) {
          throw new Error(`API error: ${res.status}`);
        }
        const data = await res.json();
        return data.posts;
      }
      var state = {
        posts: [],
        query: "",
        status: "idle",
        error: "",
        sortBy: "date",
        focusedIndex: -1,
        expanded: /* @__PURE__ */ new Set()
      };
      function sortPosts(posts, by) {
        const sorted = [...posts];
        if (by === "title") {
          return sorted.sort((a, b) => a.title.localeCompare(b.title));
        }
        if (by === "date") {
          return sorted.sort(
            (a, b) => new Date(b.pub_date).getTime() - new Date(a.pub_date).getTime()
          );
        }
        if (by === "category") {
          return sorted.sort((a, b) => {
            const ca = a.category ?? "zzz";
            const cb = b.category ?? "zzz";
            return ca.localeCompare(cb);
          });
        }
        return sorted;
      }
      function renderCommentForm(postId, container) {
        const form2 = document.createElement("form");
        form2.addEventListener("submit", async (e) => {
          e.preventDefault();
          const authorInput2 = document.getElementById("author");
          const bodyInput = document.getElementById("body");
          document.querySelectorAll(".error").forEach((el) => el.remove());
          const validation = validateComment(authorInput2.value, bodyInput.value);
          if (!validation.valid) {
            if (validation.errors.author) {
              showError("author", validation.errors.author);
            }
            if (validation.errors.body) {
              showError("body", validation.errors.body);
            }
            return;
          }
          await api.createComment(postId, {
            author: authorInput2.value,
            body: bodyInput.value
          });
        });
        const authorInput = document.createElement("input");
        authorInput.type = "text";
        authorInput.placeholder = "Your name";
        authorInput.required = true;
        const bodyTextarea = document.createElement("textarea");
        bodyTextarea.placeholder = "Your comment";
        bodyTextarea.required = true;
        bodyTextarea.rows = 3;
        const submitBtn = document.createElement("button");
        submitBtn.type = "submit";
        submitBtn.textContent = "Post Comment";
        const feedback = document.createElement("p");
        form2.append(authorInput, bodyTextarea, submitBtn, feedback);
        form2.addEventListener("submit", async (event) => {
          event.preventDefault();
          submitBtn.disabled = true;
          feedback.textContent = "Posting\u2026";
          const result = await api.createComment(postId, {
            author: authorInput.value.trim(),
            body: bodyTextarea.value.trim()
          });
          if (result.ok) {
            feedback.textContent = "Comment posted!";
            form2.reset();
          } else {
            feedback.textContent = `Error: ${result.error}`;
          }
          submitBtn.disabled = false;
        });
        container.appendChild(form2);
      }
      function renderPost(post, query) {
        const article = document.createElement("article");
        let pollTimer = null;
        let timeTimer = null;
        let secondsSinceUpdate = 0;
        const heading = document.createElement("h2");
        heading.innerHTML = highlightMatch(post.title, query);
        const excerpt = document.createElement("p");
        const text = post.body.slice(0, 120) + "\u2026";
        excerpt.innerHTML = highlightMatch(text, query);
        const fullBody = document.createElement("p");
        fullBody.textContent = post.body;
        fullBody.hidden = true;
        const toggle = document.createElement("button");
        toggle.textContent = "Read more";
        let commentsLoaded = false;
        const commentsContainer = document.createElement("div");
        commentsContainer.className = "comments";
        const timestamp = document.createElement("div");
        timestamp.className = "timestamp";
        commentsContainer.appendChild(timestamp);
        function renderComments(comments) {
          commentsContainer.innerHTML = "";
          commentsContainer.appendChild(timestamp);
          for (const comment of comments) {
            const el = document.createElement("div");
            el.className = "comment";
            const text2 = document.createElement("span");
            text2.innerHTML = `<strong>${comment.author}</strong>: ${comment.body}`;
            const delBtn = document.createElement("button");
            delBtn.textContent = "Delete";
            el.append(text2, delBtn);
            commentsContainer.appendChild(el);
            delBtn.addEventListener("click", async () => {
              const original = el;
              const parent = el.parentElement;
              const nextSibling = el.nextSibling;
              const deletingText = document.createElement("span");
              deletingText.textContent = "Deleting\u2026";
              delBtn.replaceWith(deletingText);
              await new Promise((resolve) => setTimeout(resolve, 2e3));
              el.remove();
              const result = await api.deleteComment(post.id, comment.id);
              if (!result.ok) {
                deletingText.replaceWith(delBtn);
                if (nextSibling) {
                  parent.insertBefore(original, nextSibling);
                } else {
                  parent.appendChild(original);
                }
                alert("Failed to delete comment");
              }
            });
          }
          secondsSinceUpdate = 0;
          timestamp.textContent = "Just now";
        }
        async function fetchAndUpdate() {
          if (document.hidden) return;
          const result = await api.getComments(post.id);
          if (result.ok) {
            renderComments(result.data.comments);
          }
        }
        toggle.addEventListener("click", async () => {
          const isExpanded = !fullBody.hidden;
          fullBody.hidden = isExpanded;
          excerpt.hidden = !isExpanded;
          toggle.textContent = isExpanded ? "Read more" : "Show less";
          if (isExpanded) {
            if (pollTimer) clearInterval(pollTimer);
            if (timeTimer) clearInterval(timeTimer);
            pollTimer = null;
            timeTimer = null;
            const existing = article.querySelector(".comments");
            if (existing) existing.remove();
            commentsLoaded = false;
            return;
          }
          if (!commentsLoaded) {
            commentsLoaded = true;
            article.insertBefore(commentsContainer, article.lastElementChild);
            fetchAndUpdate();
            pollTimer = setInterval(fetchAndUpdate, 1e4);
            timeTimer = setInterval(() => {
              secondsSinceUpdate++;
              timestamp.textContent = `Last updated: ${secondsSinceUpdate}s ago`;
            }, 1e3);
            const loading = document.createElement("p");
            loading.textContent = "Loading comments...";
            article.appendChild(loading);
          }
        });
        document.addEventListener("visibilitychange", () => {
          if (document.hidden) {
            if (pollTimer) clearInterval(pollTimer);
            if (timeTimer) clearInterval(timeTimer);
          } else {
            if (!fullBody.hidden) {
              fetchAndUpdate();
              pollTimer = setInterval(fetchAndUpdate, 1e4);
            }
          }
        });
        article.tabIndex = 0;
        article.addEventListener("keydown", (e) => {
          if (e.key === "Enter") {
            toggle.click();
          }
        });
        article.append(heading, excerpt, fullBody, toggle);
        renderCommentForm(post.id, article);
        return article;
      }
      function render() {
        postList.innerHTML = "";
        if (state.status === "loading") {
          statusBar.textContent = "Loading\u2026";
          return;
        }
        if (state.status === "error") {
          statusBar.textContent = `Error: ${state.error}`;
          return;
        }
        const sorted = sortPosts(state.posts, state.sortBy);
        statusBar.textContent = `${sorted.length} post(s) found`;
        console.log("Rendering posts with query:", state.query);
        for (let i = 0; i < sorted.length; i++) {
          const postEl = renderPost(sorted[i], state.query);
          const isFocused = i === state.focusedIndex;
          const isExpanded = state.expanded.has(i);
          if (isFocused) {
            postEl.classList.add("focused");
            postEl.scrollIntoView({ block: "nearest" });
          }
          const excerpt = postEl.querySelectorAll("p")[0];
          const full = postEl.querySelectorAll("p")[1];
          const button = postEl.querySelector("button");
          full.hidden = !isExpanded;
          excerpt.hidden = isExpanded;
          button.textContent = isExpanded ? "Show less" : "Read more";
          postList.appendChild(postEl);
        }
      }
      async function loadPosts(query = "") {
        state = { ...state, status: "loading", query };
        render();
        try {
          const posts = await fetchPosts(query);
          state = { ...state, posts, status: "success" };
        } catch (err) {
          state = { ...state, status: "error", error: String(err) };
        }
        render();
      }
      console.log("Initial load of posts");
      var prefs = loadPrefs();
      if (prefs) {
        state.query = prefs.query;
        state.sortBy = prefs.sortBy;
        searchInput.value = prefs.query;
        sortSelect.value = prefs.sortBy;
      }
      loadPosts(state.query);
      function debounce(fn, delay) {
        let timer;
        return (...args) => {
          clearTimeout(timer);
          timer = setTimeout(() => fn(...args), delay);
        };
      }
      var debouncedLoad = debounce((query) => {
        state.query = query;
        savePrefs({ query: state.query, sortBy: state.sortBy });
        loadPosts(query);
      }, 300);
      searchInput.addEventListener("input", (e) => {
        const value = e.target.value.trim();
        debouncedLoad(value);
      });
      sortSelect.addEventListener("change", (e) => {
        const value = e.target.value;
        state.sortBy = value;
        savePrefs({
          query: state.query,
          sortBy: state.sortBy
        });
        render();
      });
      document.addEventListener("keydown", (e) => {
        const max = state.posts.length - 1;
        switch (e.key) {
          case "ArrowDown":
            state.focusedIndex = state.focusedIndex < max ? state.focusedIndex + 1 : 0;
            render();
            break;
          case "ArrowUp":
            state.focusedIndex = state.focusedIndex > 0 ? state.focusedIndex - 1 : max;
            render();
            break;
          case "Enter": {
            if (state.focusedIndex === -1) return;
            const idx = state.focusedIndex;
            if (state.expanded.has(idx)) {
              state.expanded.delete(idx);
            } else {
              state.expanded.add(idx);
            }
            render();
            break;
          }
          case "Escape":
            state.expanded.clear();
            state.focusedIndex = -1;
            render();
            break;
        }
      });
      function isPersistedState(data) {
        if (typeof data !== "object" || data === null) return false;
        const obj = data;
        const validQuery = typeof obj.query === "string";
        const validSortBy = obj.sortBy === "title" || obj.sortBy === "date" || obj.sortBy === "category";
        return validQuery && validSortBy;
      }
      function savePrefs(prefs2) {
        localStorage.setItem("blog-prefs", JSON.stringify(prefs2));
      }
      function loadPrefs() {
        const raw = localStorage.getItem("blog-prefs");
        if (!raw) return null;
        try {
          const parsed = JSON.parse(raw);
          if (isPersistedState(parsed)) {
            return parsed;
          }
          return null;
        } catch {
          return null;
        }
      }
      var form = document.getElementById("comment-form");
      if (!form) {
        throw new Error("Form not found");
      }
      function setStatus(message) {
        const el = document.getElementById("status-bar");
        if (!el) return;
        el.textContent = message;
      }
      function wait(ms) {
        return new Promise((resolve) => setTimeout(resolve, ms));
      }
      async function withRetry(fn, retries, delayMs, setStatus2) {
        let attempt = 1;
        while (true) {
          statusBar.textContent = `Attempt ${attempt}/${attempt + retries}\u2026`;
          const result = await fn();
          if (result.ok) {
            return result;
          }
          if (retries === 0) {
            statusBar.textContent = `Failed after all retries`;
            return result;
          }
          statusBar.textContent = `Retry ${attempt}/${attempt + retries} (waiting ${delayMs}ms)\u2026`;
          await wait(delayMs);
          retries--;
          delayMs *= 2;
          attempt++;
        }
      }
      function validateComment(author, body) {
        const errors = {};
        if (author.length < 2 || author.length > 50) {
          errors.author = "Author must be 2\u201350 characters.";
        }
        if (body.length < 10 || body.length > 500) {
          errors.body = "Comment must be 10\u2013500 characters.";
        }
        if (Object.keys(errors).length > 0) {
          return { valid: false, errors };
        }
        return { valid: true };
      }
      function showError(inputId, message) {
        const input = document.getElementById(inputId);
        if (!input) return;
        const old = input.parentElement?.querySelector(".error");
        if (old) old.remove();
        const span = document.createElement("span");
        span.className = "error";
        span.textContent = message;
        input.insertAdjacentElement("afterend", span);
      }
      async function loadAndRender(query) {
        const result = await withRetry(
          () => api.getPosts(query),
          3,
          2e3,
          setStatus
        );
        if (!result.ok) {
          statusBar.textContent = `Error: ${result.error}`;
          return;
        }
        const { posts } = result.data;
        statusBar.textContent = `${posts.length} post(s)`;
        postList.innerHTML = "";
        for (const post of posts) {
          postList.appendChild(renderPost(post, query ?? ""));
        }
      }
      loadAndRender();
    }
  });
  require_main();
})();
//# sourceMappingURL=main.js.map
