"use strict";
(() => {
  // src/async.ts
  function delay(ms) {
    return new Promise((resolve) => setTimeout(resolve, ms));
  }
  async function countdown(element) {
    for (let i = 5; i >= 0; i--) {
      element.textContent = `${i}\u2026`;
      await delay(1e3);
    }
    element.textContent = "Go!";
  }
  async function fetchTodo(id) {
    const res = await fetch(`https://jsonplaceholder.typicode.com/todos/${id}`);
    if (!res.ok) {
      throw new Error(`HTTP ${res.status}`);
    }
    const data = await res.json();
    return data;
  }
  function fetchWithTimeout(url, ms) {
    const timeout = new Promise(
      (_, reject) => setTimeout(() => reject(new Error("Timeout")), ms)
    );
    const request = fetch(url).then((res) => res.json());
    return Promise.race([request, timeout]);
  }
  async function fetchParallel() {
    const start = performance.now();
    const promises = [];
    for (let i = 1; i <= 10; i++) {
      promises.push(
        fetch(`https://jsonplaceholder.typicode.com/todos/${i}`).then((res) => res.json())
      );
    }
    const todos = await Promise.all(promises);
    const end = performance.now();
    return { todos, time: end - start };
  }
  async function fetchSequential() {
    const start = performance.now();
    const todos = [];
    for (let i = 1; i <= 10; i++) {
      const todo = await fetch(
        `https://jsonplaceholder.typicode.com/todos/${i}`
      ).then((res) => res.json());
      todos.push(todo);
    }
    const end = performance.now();
    return { todos, time: end - start };
  }

  // src/blog.ts
  function summarise(post) {
    const categoryName = post.category?.name ?? "Uncategorised";
    const shortBody = post.body.length > 50 ? post.body.slice(0, 50) + "..." : post.body;
    return `${post.title} (${categoryName}) \u2014 ${shortBody}`;
  }
  function sortPosts(posts2, by) {
    const copy = [...posts2];
    return copy.sort((a, b) => {
      if (by === "title") {
        return a.title.localeCompare(b.title);
      }
      if (by === "date") {
        return new Date(b.pubDate).getTime() - new Date(a.pubDate).getTime();
      }
      if (by === "category") {
        const aCat = a.category?.name ?? "zzz";
        const bCat = b.category?.name ?? "zzz";
        return aCat.localeCompare(bCat);
      }
      return 0;
    });
  }
  function validatePost(data) {
    if (typeof data !== "object" || data === null) {
      return {
        ok: false,
        reason: "Data is not an object"
      };
    }
    const obj = data;
    if (typeof obj.id !== "number") {
      return {
        ok: false,
        reason: "Invalid id"
      };
    }
    if (typeof obj.title !== "string") {
      return {
        ok: false,
        reason: "Invalid title"
      };
    }
    if (typeof obj.slug !== "string") {
      return {
        ok: false,
        reason: "Invalid slug"
      };
    }
    if (typeof obj.body !== "string") {
      return {
        ok: false,
        reason: "Invalid body"
      };
    }
    if (typeof obj.pubDate !== "string") {
      return {
        ok: false,
        reason: "Invalid pubDate"
      };
    }
    if (obj.category !== null) {
      if (typeof obj.category !== "object" || obj.category === null) {
        return {
          ok: false,
          reason: "Invalid category"
        };
      }
      const category = obj.category;
      if (typeof category.id !== "number") {
        return {
          ok: false,
          reason: "Invalid category.id"
        };
      }
      if (typeof category.name !== "string") {
        return {
          ok: false,
          reason: "Invalid category.name"
        };
      }
      if (typeof category.slug !== "string") {
        return {
          ok: false,
          reason: "Invalid category.slug"
        };
      }
    }
    return {
      ok: true,
      post: obj
    };
  }

  // src/shapes.ts
  function area(shape) {
    switch (shape.kind) {
      case "circle":
        return Math.PI * shape.radius * shape.radius;
      case "rectangle":
        return shape.width * shape.height;
      case "triangle":
        return shape.base * shape.height / 2;
      default:
        return 0;
    }
  }
  function svgEl(tag, attrs) {
    const el = document.createElementNS(
      "http://www.w3.org/2000/svg",
      tag
    );
    for (const [k, v] of Object.entries(attrs)) {
      el.setAttribute(k, String(v));
    }
    return el;
  }
  function renderShape(shape) {
    const svg = document.createElementNS(
      "http://www.w3.org/2000/svg",
      "svg"
    );
    svg.setAttribute("width", "120");
    svg.setAttribute("height", "120");
    svg.setAttribute("viewBox", "0 0 120 120");
    switch (shape.kind) {
      case "circle":
        svg.appendChild(
          svgEl("circle", {
            cx: 60,
            cy: 60,
            r: shape.radius,
            fill: "steelblue"
          })
        );
        break;
      case "rectangle":
        svg.appendChild(
          svgEl("rect", {
            x: 10,
            y: 10,
            width: shape.width,
            height: shape.height,
            fill: "coral"
          })
        );
        break;
      case "triangle":
        svg.appendChild(
          svgEl("polygon", {
            points: `10,110 ${shape.base},110 ${shape.base / 2},10`,
            fill: "seagreen"
          })
        );
        break;
    }
    return svg;
  }
  var shapes = [
    {
      kind: "circle",
      radius: 40
    },
    {
      kind: "rectangle",
      width: 80,
      height: 60
    },
    {
      kind: "triangle",
      base: 100,
      height: 80
    }
  ];
  function renderShapesGallery() {
    const output2 = document.getElementById("output");
    const container = document.createElement("div");
    for (const shape of shapes) {
      const box = document.createElement("div");
      const svg = renderShape(shape);
      const p = document.createElement("p");
      p.textContent = `Area: ${area(shape).toFixed(2)}`;
      box.appendChild(svg);
      box.appendChild(p);
      container.appendChild(box);
    }
    output2.appendChild(container);
  }

  // src/utils.ts
  function pipe(...fns) {
    return (arg) => {
      return fns.reduce(
        (value, fn) => fn(value),
        arg
      );
    };
  }
  function memoize(fn) {
    const cache = {};
    return async (arg) => {
      if (arg in cache) {
        return cache[arg];
      }
      const result2 = await fn(arg);
      cache[arg] = result2;
      return result2;
    };
  }

  // src/main.ts
  var output = document.getElementById("output");
  var postsContainer = document.createElement("div");
  postsContainer.id = "posts";
  output.appendChild(postsContainer);
  function show(label, value) {
    const line = document.createElement("p");
    line.textContent = `${label}: ${String(value)}`;
    output.appendChild(line);
  }
  function greet(name, times) {
    return `Hello, ${name}! `.repeat(times).trim();
  }
  function clamp(value, min, max) {
    return Math.min(Math.max(value, min), max);
  }
  show("greet", greet("World", 2));
  show("clamp(15, 0, 10)", clamp(15, 0, 10));
  show("clamp(-5, 0, 10)", clamp(-5, 0, 10));
  function formatDuration(totalSeconds) {
    if (totalSeconds < 60) {
      return `${totalSeconds}s`;
    }
    const hours = Math.floor(totalSeconds / 3600);
    const minutes = Math.floor(totalSeconds % 3600 / 60);
    const seconds = totalSeconds % 60;
    if (totalSeconds < 3600) {
      return `${minutes}m ${seconds}s`;
    }
    return `${hours}h ${minutes}m ${seconds}s`;
  }
  var testCases = [
    [0, "0s"],
    [5, "5s"],
    [62, "1m 2s"],
    [3661, "1h 1m 1s"],
    [86400, "24h 0m 0s"]
  ];
  var table = document.createElement("table");
  table.innerHTML = "<tr><th>Input</th><th>Expected</th><th>Got</th><th>\u2713</th></tr>";
  for (const [input, expected] of testCases) {
    const got = formatDuration(input);
    const pass = got === expected;
    const row = document.createElement("tr");
    row.innerHTML = `<td>${input}</td><td>${expected}</td><td>${got}</td>
                     <td>${pass ? "\u2705" : "\u274C"}</td>`;
    if (!pass) row.classList.add("error");
    table.appendChild(row);
  }
  document.getElementById("output").appendChild(table);
  var counter = document.createElement("h2");
  document.getElementById("output").appendChild(counter);
  countdown(counter);
  async function renderTodos() {
    const container = document.createElement("div");
    container.innerHTML = "<h3>Todos from API</h3>";
    document.getElementById("output").appendChild(container);
    for (const id of [1, 2, 3]) {
      const todo = await fetchTodo(id);
      const p = document.createElement("p");
      p.textContent = `${todo.completed ? "\u2705" : "\u2B1C"} ${todo.title}`;
      container.appendChild(p);
    }
  }
  renderTodos();
  async function run() {
    const tests = [
      5e3,
      // powinno działać
      1
      // powinno timeout
    ];
    for (const ms of tests) {
      const p = document.createElement("p");
      try {
        const todo = await fetchWithTimeout(
          "https://jsonplaceholder.typicode.com/todos/1",
          ms
        );
        p.textContent = `ms=${ms} \u2705 Succeeded: ${todo.title}`;
      } catch {
        p.textContent = `ms=${ms} \u274C Timed out`;
      }
      output.appendChild(p);
    }
  }
  run();
  async function run2() {
    const parallel = await fetchParallel();
    const sequential = await fetchSequential();
    const box = document.createElement("div");
    const list = document.createElement("ul");
    parallel.todos.forEach((todo) => {
      const li = document.createElement("li");
      li.textContent = todo.title;
      list.appendChild(li);
    });
    box.appendChild(list);
    const info = document.createElement("p");
    info.innerHTML = `
        \u26A1 Parallel: ${parallel.time.toFixed(2)} ms <br>
        \u{1F40C} Sequential: ${sequential.time.toFixed(2)} ms
    `;
    box.appendChild(info);
    output.appendChild(box);
  }
  run2();
  var posts = [
    {
      id: 1,
      title: "Hello TypeScript",
      slug: "hello-ts",
      body: "TypeScript is JavaScript with types. It compiles to plain JS.",
      pubDate: "2025-01-01",
      category: { id: 1, name: "Tech", slug: "tech" }
    },
    {
      id: 2,
      title: "CSS Grid",
      slug: "css-grid",
      body: "CSS Grid is a two-dimensional layout system for the web.",
      pubDate: "2025-01-15",
      category: { id: 2, name: "Frontend", slug: "frontend" }
    },
    {
      id: 3,
      title: "Django REST",
      slug: "django-rest",
      body: "Build a REST API with Django and serve JSON to any client.",
      pubDate: "2025-02-01",
      category: { id: 1, name: "Tech", slug: "tech" }
    }
  ];
  function renderPostCard(post) {
    const card = document.createElement("article");
    card.innerHTML = `<h3>${post.title}</h3><p>${summarise(post)}</p>`;
    return card;
  }
  var select = document.createElement("select");
  select.innerHTML = `
  <option value="title">Title</option>
  <option value="date">Date</option>
  <option value="category">Category</option>
`;
  document.getElementById("output").prepend(select);
  function render(posts2) {
    postsContainer.innerHTML = "";
    for (const post of posts2) {
      postsContainer.appendChild(renderPostCard(post));
    }
  }
  select.addEventListener("change", () => {
    const value = select.value;
    const sorted = sortPosts(posts, value);
    render(sorted);
  });
  render(posts);
  renderShapesGallery();
  var textarea = document.createElement("textarea");
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
  var button = document.createElement("button");
  button.textContent = "Validate";
  var result = document.createElement("div");
  document.getElementById("output").appendChild(textarea);
  document.getElementById("output").appendChild(button);
  document.getElementById("output").appendChild(result);
  button.addEventListener("click", () => {
    result.innerHTML = "";
    try {
      const parsed = JSON.parse(textarea.value);
      const validation = validatePost(parsed);
      if (validation.ok) {
        const success = document.createElement("p");
        success.textContent = "\u2705 Valid Post";
        result.appendChild(success);
        result.appendChild(
          renderPostCard(validation.post)
        );
      } else {
        const fail = document.createElement("p");
        fail.textContent = `\u274C Invalid: ${validation.reason}`;
        result.appendChild(fail);
      }
    } catch {
      const fail = document.createElement("p");
      fail.textContent = "\u274C Invalid JSON";
      result.appendChild(fail);
    }
  });
  var processPosts = pipe(
    // only posts with category
    (posts2) => posts2.filter(
      (p) => p.category !== null
    ),
    // only newer posts
    (posts2) => posts2.filter(
      (p) => p.pubDate >= "2025-01-10"
    ),
    // sort alphabetically
    (posts2) => posts2.sort(
      (a, b) => a.title.localeCompare(b.title)
    ),
    // limit to 5
    (posts2) => posts2.slice(0, 5)
  );
  var processed = processPosts(posts);
  var section = document.createElement("div");
  section.innerHTML = `
    <h2>Pipeline Demo</h2>
`;
  var inputBox = document.createElement("div");
  inputBox.innerHTML = `
    <h3>Input</h3>
`;
  for (const post of posts) {
    const p = document.createElement("p");
    p.textContent = post.title;
    inputBox.appendChild(p);
  }
  var outputBox = document.createElement("div");
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
  document.getElementById("output").appendChild(section);
  var cachedFetch = memoize(fetchTodo);
  async function testMemoize() {
    const section2 = document.createElement("div");
    section2.innerHTML = `
        <h2>Memoization Demo</h2>
    `;
    const start1 = performance.now();
    const todo1 = await cachedFetch(1);
    const end1 = performance.now();
    const p1 = document.createElement("p");
    p1.textContent = `
        Cache MISS (${(end1 - start1).toFixed(2)} ms):
        ${todo1.title}
    `;
    section2.appendChild(p1);
    const start2 = performance.now();
    const todo2 = await cachedFetch(1);
    const end2 = performance.now();
    const p2 = document.createElement("p");
    p2.textContent = `
        Cache HIT (${(end2 - start2).toFixed(2)} ms):
        ${todo2.title}
    `;
    section2.appendChild(p2);
    document.getElementById("output").appendChild(section2);
  }
  testMemoize();
})();
//# sourceMappingURL=main.js.map
