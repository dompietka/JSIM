const sortSelect = getElement<HTMLSelectElement>("sort-select");

console.log("Hello, World!");
// A helper that throws if the element is missing
// better than silently failing at runtime.
function getElement<T extends HTMLElement>(id: string): T {
    const el = document.getElementById(id);
    if (el === null) {
        throw new Error(`Element #${id} not found`);
    }
    return el as T;
}

type ValidationResult =
    | { valid: true }
    | { valid: false; errors: Record<string, string> };

import { api, Result, Comment } from "./api";





function escapeRegExp(str: string): string {
    return str.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

function highlightMatch(text: string, query: string): string {
    if (!query) return text;

    const safeQuery = escapeRegExp(query);
    const regex = new RegExp(safeQuery, "gi");
    console.log("Highlighting matches for query:", query);

    return text.replace(regex, (match) => `<mark>${match}</mark>`);
}

// Use getElement to grab references to:
// - #search-input (HTMLInputElement)
// - #post-list (HTMLDivElement)
// - #status-bar (HTMLParagraphElement)

const searchInput = getElement<HTMLInputElement>("search-input");
const postList = getElement<HTMLDivElement>("post-list");
const statusBar = getElement<HTMLParagraphElement>("status-bar");

interface Post {
    id:       number;
    title:    string;
    slug:     string;
    body:     string;
    pub_date: string;    // ISO string from Django
    category: string | null;
}


interface PostsResponse {
    posts: Post[];
}

async function fetchPosts(query: string = ""): Promise<Post[]> {
    const url = query
        ? `/api/posts/?search=${encodeURIComponent(query)}`
        : "/api/posts/";
    const res = await fetch(url);
    if (!res.ok) {
        throw new Error(`API error: ${res.status}`);
    }
    const data: PostsResponse = await res.json();
    return data.posts;
}


type PageStatus = "idle" | "loading" | "success" | "error";
type SortField  = "title" | "date" | "category";

interface AppState {
    posts:   Post[];
    query:   string;
    status:  PageStatus;
    error:   string;
    sortBy:  SortField;
    focusedIndex: number; // For keyboard navigation
     expanded: Set<number>; // tracks which posts are expanded
}

let state: AppState = {
    posts:  [],
    query:  "",
    status: "idle",
    error:  "",
    sortBy: "date",
    focusedIndex: -1,
    expanded: new Set()
};

function sortPosts(posts: Post[], by: SortField): Post[] {
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

function renderCommentForm(postId: number, container: HTMLElement): void {
    const form = document.createElement("form");
    form.addEventListener("submit", async (e) => {
    e.preventDefault();

    const authorInput = document.getElementById("author") as HTMLInputElement;
    const bodyInput = document.getElementById("body") as HTMLTextAreaElement;

    // usuń stare błędy
    document.querySelectorAll(".error").forEach(el => el.remove());

    const validation = validateComment(authorInput.value, bodyInput.value);

    if (!validation.valid) {
        if (validation.errors.author) {
            showError("author", validation.errors.author);
        }

        if (validation.errors.body) {
            showError("body", validation.errors.body);
        }

        return; // ❌ NIE wysyłamy requestu
    }

    // ✅ dopiero teraz API call
    await api.createComment(postId, {
        author: authorInput.value,
        body: bodyInput.value
    });
});

    const authorInput = document.createElement("input");
    authorInput.type        = "text";
    authorInput.placeholder = "Your name";
    authorInput.required    = true;

    const bodyTextarea = document.createElement("textarea");
    bodyTextarea.placeholder = "Your comment";
    bodyTextarea.required    = true;
    bodyTextarea.rows        = 3;

    const submitBtn = document.createElement("button");
    submitBtn.type        = "submit";
    submitBtn.textContent = "Post Comment";

    const feedback = document.createElement("p");

    form.append(authorInput, bodyTextarea, submitBtn, feedback);

    form.addEventListener("submit", async (event: Event) => {
        event.preventDefault();
        submitBtn.disabled = true;
        feedback.textContent = "Posting…";

        const result = await api.createComment(postId, {
            author: authorInput.value.trim(),
            body:   bodyTextarea.value.trim(),
        });

        if (result.ok) {
            feedback.textContent = "Comment posted!";
            form.reset();
            // TODO: Append a new <div> to container displaying result.data.author
            //       and result.data.body — no page reload needed.
        } else {
            feedback.textContent = `Error: ${result.error}`;
        }

        submitBtn.disabled = false;
    });

    container.appendChild(form);
}


function renderPost(post: Post, query: string): HTMLElement {
    const article = document.createElement("article");

    let pollTimer: ReturnType<typeof setInterval> | null = null;
    let timeTimer: ReturnType<typeof setInterval> | null = null;
    let secondsSinceUpdate = 0;

    const heading = document.createElement("h2");
    heading.innerHTML = highlightMatch(post.title, query);

    const excerpt = document.createElement("p");
    const text = post.body.slice(0, 120) + "…";
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

    function renderComments(comments: Comment[]) {
        commentsContainer.innerHTML = "";
        commentsContainer.appendChild(timestamp);

        for (const comment of comments) {
            const el = document.createElement("div");
            el.className = "comment";

            const text = document.createElement("span");
            text.innerHTML = `<strong>${comment.author}</strong>: ${comment.body}`;

            const delBtn = document.createElement("button");
            delBtn.textContent = "Delete";

            el.append(text, delBtn);
            commentsContainer.appendChild(el);

            delBtn.addEventListener("click", async () => {
                        const original = el;
                        const parent = el.parentElement!;
                        const nextSibling = el.nextSibling;

                        // 1. optimistic UI
                        const deletingText = document.createElement("span");
                        deletingText.textContent = "Deleting…";

                        delBtn.replaceWith(deletingText);

                        await new Promise(resolve => setTimeout(resolve, 2000));
                        el.remove();

                        // 2. API call
                        const result = await api.deleteComment(post.id, comment.id);

                        // 3. rollback on failure
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

        // COLLAPSE → usuń komentarze
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

        // EXPAND → load only once
        if (!commentsLoaded) {
            commentsLoaded = true;

            article.insertBefore(commentsContainer, article.lastElementChild);

            fetchAndUpdate();

            pollTimer = setInterval(fetchAndUpdate, 10000);

            timeTimer = setInterval(() => {
                secondsSinceUpdate++;
                timestamp.textContent = `Last updated: ${secondsSinceUpdate}s ago`;
            }, 1000);

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
            pollTimer = setInterval(fetchAndUpdate, 10000);
        }
    }
});
    // Keyboard support: Enter toggles expand/collapse
    article.tabIndex = 0;
    article.addEventListener("keydown", (e: KeyboardEvent) => {
        if (e.key === "Enter") {
            toggle.click();
        }
    });

    article.append(heading, excerpt, fullBody, toggle);
    renderCommentForm(post.id, article);

    return article;
}



function render(): void {
    postList.innerHTML = "";

    if (state.status === "loading") {
        statusBar.textContent = "Loading…";
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
    const button = postEl.querySelector("button")!;

    full.hidden = !isExpanded;
    excerpt.hidden = isExpanded;
    button.textContent = isExpanded ? "Show less" : "Read more";

    postList.appendChild(postEl);
}
}

async function loadPosts(query: string = ""): Promise<void> {
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

const prefs = loadPrefs();

if (prefs) {
    state.query = prefs.query;
    state.sortBy = prefs.sortBy;
    searchInput.value = prefs.query;
    sortSelect.value = prefs.sortBy;
}

loadPosts(state.query);

function togglePost(index: number) {
    if (state.expanded.has(index)) {
        state.expanded.delete(index);
    } else {
        state.expanded.add(index);
    }
}

function debounce(fn: (...args: any[]) => void, delay: number) {
    let timer: ReturnType<typeof setTimeout>;
    return (...args: any[]) => {
        clearTimeout(timer);
        timer = setTimeout(() => fn(...args), delay);
    };
}

const debouncedLoad = debounce((query: string) => {
    state.query = query;
    savePrefs({ query: state.query, sortBy: state.sortBy });
    loadPosts(query);
}, 300);

searchInput.addEventListener("input", (e: Event) => {
    const value = (e.target as HTMLInputElement).value.trim();
    debouncedLoad(value);
}); 


sortSelect.addEventListener("change", (e: Event) => {
    const value = (e.target as HTMLSelectElement).value as SortField;

    state.sortBy = value;

    savePrefs({
        query: state.query,
        sortBy: state.sortBy
    });

    render();
});

document.addEventListener("keydown", (e: KeyboardEvent) => {
    const max = state.posts.length - 1;

    switch (e.key) {
        case "ArrowDown":
            state.focusedIndex =
                state.focusedIndex < max ? state.focusedIndex + 1 : 0;
            render();
            break;

        case "ArrowUp":
            state.focusedIndex =
                state.focusedIndex > 0 ? state.focusedIndex - 1 : max;
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

interface PersistedState {
    query: string;
    sortBy: SortField;
}

function isPersistedState(data: unknown): data is PersistedState {
    if (typeof data !== "object" || data === null) return false;

    const obj = data as Record<string, unknown>;

    const validQuery = typeof obj.query === "string";

    const validSortBy =
        obj.sortBy === "title" ||
        obj.sortBy === "date" ||
        obj.sortBy === "category";

    return validQuery && validSortBy;
}

function savePrefs(prefs: PersistedState): void {
    localStorage.setItem("blog-prefs", JSON.stringify(prefs));
}

function loadPrefs(): PersistedState | null {
    const raw = localStorage.getItem("blog-prefs");
    if (!raw) return null;

    try {
        const parsed: unknown = JSON.parse(raw);

        if (isPersistedState(parsed)) {
            return parsed;
        }

        return null;
    } catch {
        return null;
    }
}

const form = document.getElementById("comment-form");

if (!form) {
    throw new Error("Form not found");
}

function setStatus(message: string) {
    const el = document.getElementById("status-bar");

    if (!el) return;

    el.textContent = message;
}

function wait(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function withRetry<T>(
    fn: () => Promise<Result<T>>,
    retries: number,
    delayMs: number,
    setStatus: (message: string) => void
): Promise<Result<T>> {

    let attempt = 1;

    while (true) {
        statusBar.textContent = `Attempt ${attempt}/${attempt + retries}…`;

        const result = await fn();

        if (result.ok) {
            return result;
        }

        if (retries === 0) {
            statusBar.textContent = `Failed after all retries`;
            return result;
        }

        statusBar.textContent =
            `Retry ${attempt}/${attempt + retries} (waiting ${delayMs}ms)…`;

        await wait(delayMs);

        retries--;
        delayMs *= 2;
        attempt++;
    }
}

function validateComment(author: string, body: string): ValidationResult {
    const errors: Record<string, string> = {};

    if (author.length < 2 || author.length > 50) {
        errors.author = "Author must be 2–50 characters.";
    }

    if (body.length < 10 || body.length > 500) {
        errors.body = "Comment must be 10–500 characters.";
    }

    if (Object.keys(errors).length > 0) {
        return { valid: false, errors };
    }

    return { valid: true };
}

function showError(inputId: string, message: string) {
    const input = document.getElementById(inputId);
    if (!input) return;

    // usuń stary błąd
    const old = input.parentElement?.querySelector(".error");
    if (old) old.remove();

    const span = document.createElement("span");
    span.className = "error";
    span.textContent = message;

    input.insertAdjacentElement("afterend", span);
}



async function loadAndRender(query?: string): Promise<void> {
    const result = await withRetry(
    () => api.getPosts(query),
    3,
    2000, setStatus
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