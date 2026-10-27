function delay(ms: number): Promise<void> {
    return new Promise((resolve) => setTimeout(resolve, ms));
}

// async/await makes Promise code read top-to-bottom:
async function countdown(element: HTMLElement): Promise<void> {
    for (let i = 5; i >= 0; i--) {
        element.textContent = `${i}…`;
        await delay(1000);
    }
    element.textContent = "Go!";
}

interface Todo {
    userId:    number;
    id:        number;
    title:     string;
    completed: boolean;
}

async function fetchTodo(id: number): Promise<Todo> {
    const res = await fetch(`https://jsonplaceholder.typicode.com/todos/${id}`);
    if (!res.ok) {
        throw new Error(`HTTP ${res.status}`);
    }
    const data: Todo = await res.json();
    return data;
}

function fetchWithTimeout<T>(url: string, ms: number): Promise<T> {
    const timeout = new Promise<never>((_, reject) =>
        setTimeout(() => reject(new Error("Timeout")), ms)
    );

    const request = fetch(url).then(res => res.json());

    return Promise.race([request, timeout]);
}

export async function fetchParallel() {
    const start = performance.now();

    const promises = [];

    for (let i = 1; i <= 10; i++) {
        promises.push(
            fetch(`https://jsonplaceholder.typicode.com/todos/${i}`)
                .then(res => res.json())
        );
    }

    const todos = await Promise.all(promises);

    const end = performance.now();

    return { todos, time: end - start };
}

export async function fetchSequential() {
    const start = performance.now();

    const todos: Todo[] = [];

    for (let i = 1; i <= 10; i++) {
        const todo = await fetch(
            `https://jsonplaceholder.typicode.com/todos/${i}`
        ).then(res => res.json());

        todos.push(todo);
    }

    const end = performance.now();

    return { todos, time: end - start };
}

export { delay, countdown, fetchTodo, Todo, fetchWithTimeout };
