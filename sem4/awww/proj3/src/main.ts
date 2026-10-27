const output = document.getElementById("output")!;

const postsContainer = document.createElement("div");
postsContainer.id = "posts";
output.appendChild(postsContainer);

// A helper to display results in the page instead of the console.
function show(label: string, value: unknown): void {
    const line = document.createElement("p");
    line.textContent = `${label}: ${String(value)}`;
    output.appendChild(line);
}

function greet(name: string, times: number): string {
    return `Hello, ${name}! `.repeat(times).trim();
}

function clamp(value: number, min: number, max: number): number {
    return Math.min(Math.max(value, min), max);
}

show("greet", greet("World", 2));
show("clamp(15, 0, 10)", clamp(15, 0, 10));
show("clamp(-5, 0, 10)", clamp(-5, 0, 10));

function formatDuration(totalSeconds: number): string {
    if (totalSeconds < 60) {
        return `${totalSeconds}s`;
    }

    const hours = Math.floor(totalSeconds / 3600);
    const minutes = Math.floor((totalSeconds % 3600) / 60);
    const seconds = totalSeconds % 60;

    if (totalSeconds < 3600) {
        return `${minutes}m ${seconds}s`;
    }

    return `${hours}h ${minutes}m ${seconds}s`;
}

// TESTY (też w main.ts)
const testCases: Array<[number, string]> = [
    [0, "0s"], [5, "5s"], [62, "1m 2s"],
    [3661, "1h 1m 1s"], [86400, "24h 0m 0s"],
];

const table = document.createElement("table");
table.innerHTML = "<tr><th>Input</th><th>Expected</th><th>Got</th><th>✓</th></tr>";

for (const [input, expected] of testCases) {
    const got = formatDuration(input);
    const pass = got === expected;

    const row = document.createElement("tr");
    row.innerHTML = `<td>${input}</td><td>${expected}</td><td>${got}</td>
                     <td>${pass ? "✅" : "❌"}</td>`;

    if (!pass) row.classList.add("error");

    table.appendChild(row);
}

document.getElementById("output")!.appendChild(table);

import { delay, countdown } from "./async";

const counter = document.createElement("h2");
document.getElementById("output")!.appendChild(counter);
countdown(counter);

import { fetchTodo, Todo } from "./async";

async function renderTodos(): Promise<void> {
    const container = document.createElement("div");
    container.innerHTML = "<h3>Todos from API</h3>";
    document.getElementById("output")!.appendChild(container);

    for (const id of [1, 2, 3]) {
        const todo = await fetchTodo(id);
        const p = document.createElement("p");
        p.textContent = `${todo.completed ? "✅" : "⬜"} ${todo.title}`;
        container.appendChild(p);
    }
}

renderTodos();

import { fetchWithTimeout } from "./async";



async function run() {
    const tests = [
        5000, // powinno działać
        1     // powinno timeout
    ];

    for (const ms of tests) {
        const p = document.createElement("p");

        try {
            const todo = await fetchWithTimeout<Todo>(
                "https://jsonplaceholder.typicode.com/todos/1",
                ms
            );

            p.textContent = `ms=${ms} ✅ Succeeded: ${todo.title}`;
        } catch {
            p.textContent = `ms=${ms} ❌ Timed out`;
        }

        output.appendChild(p);
    }
}

run();

import { fetchParallel, fetchSequential } from "./async";


async function run2() {
    const parallel = await fetchParallel();
    const sequential = await fetchSequential();

    const box = document.createElement("div");

    const list = document.createElement("ul");

    parallel.todos.forEach(todo => {
        const li = document.createElement("li");
        li.textContent = todo.title;
        list.appendChild(li);
    });

    box.appendChild(list);

    const info = document.createElement("p");
    info.innerHTML = `
        ⚡ Parallel: ${parallel.time.toFixed(2)} ms <br>
        🐌 Sequential: ${sequential.time.toFixed(2)} ms
    `;

    box.appendChild(info);
    output.appendChild(box);
}

run2();

import { Post, summarise, filterByCategory, sortPosts } from "./blog";

const posts: Post[] = [
    {
        id: 1, title: "Hello TypeScript", slug: "hello-ts",
        body: "TypeScript is JavaScript with types. It compiles to plain JS.",
        pubDate: "2025-01-01",
        category: { id: 1, name: "Tech", slug: "tech" },
    },
    {
        id: 2, title: "CSS Grid", slug: "css-grid",
        body: "CSS Grid is a two-dimensional layout system for the web.",
        pubDate: "2025-01-15",
        category: { id: 2, name: "Frontend", slug: "frontend" },
    },
    {
        id: 3, title: "Django REST", slug: "django-rest",
        body: "Build a REST API with Django and serve JSON to any client.",
        pubDate: "2025-02-01",
        category: { id: 1, name: "Tech", slug: "tech" },
    },
];

// Render each post as a card
function renderPostCard(post: Post): HTMLElement {
    const card = document.createElement("article");
    card.innerHTML = `<h3>${post.title}</h3><p>${summarise(post)}</p>`;
    return card;
}




const select = document.createElement("select");
select.innerHTML = `
  <option value="title">Title</option>
  <option value="date">Date</option>
  <option value="category">Category</option>
`;

document.getElementById("output")!.prepend(select);

function render(posts: Post[]) {
    postsContainer.innerHTML = "";

    for (const post of posts) {
        postsContainer.appendChild(renderPostCard(post));
    }
}

select.addEventListener("change", () => {
    const value = select.value as "title" | "date" | "category";

    const sorted = sortPosts(posts, value);
    render(sorted);
});

render(posts);

import { renderShapesGallery } from "./shapes";

renderShapesGallery();

import { validatePost } from "./blog";

const textarea = document.createElement("textarea");
textarea.rows = 12;
textarea.cols = 50;

textarea.value = `{
  "id": 1,
  "title": "Test Post",
  "slug": "test-post",
  "body": "Hello from JSON",
  "pubDate": "2025-01-01",
  "category": {
    "id": 1,
    "name": "Tech",
    "slug": "tech"
  }
}`;

const button = document.createElement("button");
button.textContent = "Validate";

const result = document.createElement("div");

document.getElementById("output")!.appendChild(textarea);
document.getElementById("output")!.appendChild(button);
document.getElementById("output")!.appendChild(result);

button.addEventListener("click", () => {

    result.innerHTML = "";

    try {
        const parsed = JSON.parse(textarea.value);

        const validation = validatePost(parsed);

        if (validation.ok) {

            const success = document.createElement("p");
            success.textContent = "✅ Valid Post";

            result.appendChild(success);

            result.appendChild(
                renderPostCard(validation.post)
            );

        } else {

            const fail = document.createElement("p");
            fail.textContent =
                `❌ Invalid: ${validation.reason}`;

            result.appendChild(fail);
        }

    } catch {
        const fail = document.createElement("p");
        fail.textContent = "❌ Invalid JSON";

        result.appendChild(fail);
    }
});

import { pipe } from "./utils";

const processPosts = pipe<Post[]>(

    // only posts with category
    posts => posts.filter(
        p => p.category !== null
    ),

    // only newer posts
    posts => posts.filter(
        p => p.pubDate >= "2025-01-10"
    ),

    // sort alphabetically
    posts => posts.sort(
        (a, b) => a.title.localeCompare(b.title)
    ),

    // limit to 5
    posts => posts.slice(0, 5)
);

const processed = processPosts(posts);

// ======================
// RENDER
// ======================

const section = document.createElement("div");

section.innerHTML = `
    <h2>Pipeline Demo</h2>
`;

const inputBox = document.createElement("div");
inputBox.innerHTML = `
    <h3>Input</h3>
`;

for (const post of posts) {
    const p = document.createElement("p");
    p.textContent = post.title;

    inputBox.appendChild(p);
}

const outputBox = document.createElement("div");
outputBox.innerHTML = `
    <h3>Output</h3>
`;

for (const post of processed) {
    const p = document.createElement("p");
    p.textContent = post.title;

    outputBox.appendChild(p);
}

section.appendChild(inputBox);
section.appendChild(outputBox);

document.getElementById("output")!
    .appendChild(section);

import { memoize } from "./utils";

const cachedFetch = memoize(fetchTodo);

async function testMemoize() {

    const section = document.createElement("div");

    section.innerHTML = `
        <h2>Memoization Demo</h2>
    `;

    // ======================
    // FIRST CALL
    // ======================

    const start1 = performance.now();

    const todo1 = await cachedFetch(1);

    const end1 = performance.now();

    const p1 = document.createElement("p");

    p1.textContent = `
        Cache MISS (${(end1 - start1).toFixed(2)} ms):
        ${todo1.title}
    `;

    section.appendChild(p1);

    // ======================
    // SECOND CALL
    // ======================

    const start2 = performance.now();

    const todo2 = await cachedFetch(1);

    const end2 = performance.now();

    const p2 = document.createElement("p");

    p2.textContent = `
        Cache HIT (${(end2 - start2).toFixed(2)} ms):
        ${todo2.title}
    `;

    section.appendChild(p2);

    document
        .getElementById("output")!
        .appendChild(section);
}

testMemoize();

