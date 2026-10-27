// src/status.ts

// An enum compiles to a real JavaScript object.
enum PostStatus {
    Draft = "draft",
    Published = "published",
    Archived = "archived",
}

// A union type: the value must be one of these strings.
type HttpMethod = "GET" | "POST" | "PUT" | "PATCH" | "DELETE";

interface ApiRequest {
    method: HttpMethod;
    path: string;
    body?: unknown;
}

// Example Post interface for the type guard
interface Post {
    title: string;
    slug: string;
}

// A type guard narrows a wide type to a specific one.
function isPost(value: unknown): value is Post {
    return (
        typeof value === "object" &&
        value !== null &&
        "title" in value &&
        "slug" in value
    );
}

// Returns a readable description of the request
function describeRequest(req: ApiRequest): string {
    if (req.body !== undefined) {
        return `${req.method} ${req.path} (has body)`;
    }

    return `${req.method} ${req.path}`;
}

export {
    PostStatus,
    HttpMethod,
    ApiRequest,
    isPost,
    describeRequest
};