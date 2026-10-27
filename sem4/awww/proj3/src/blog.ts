interface Category {
    id:   number;
    name: string;
    slug: string;
}

interface Post {
    id:       number;
    title:    string;
    slug:     string;
    body:     string;
    pubDate:  string;
    category: Category | null;   // a field that is either a Category or nothing — union types are introduced in Phase 4
}

interface Comment {
    id:      number;
    author:  string;
    body:    string;
    created: string;
}

// TODO: Write a function summarise(post: Post): string that returns
// "<title> (<category name or 'Uncategorised'>) — <first 50 chars of body>..."

function summarise(post: Post): string {
    const categoryName = post.category?.name ?? "Uncategorised";

    const shortBody =
        post.body.length > 50
            ? post.body.slice(0, 50) + "..."
            : post.body;

    return `${post.title} (${categoryName}) — ${shortBody}`;
}

// TODO: Write a function filterByCategory(posts: Post[], categoryName: string): Post[]
// that returns only posts whose category.name matches (case-insensitive).

function filterByCategory(posts: Post[], categoryName: string): Post[] {
    return posts.filter(post => {
        if (!post.category) return false;
        return post.category.name.toLowerCase() === categoryName.toLowerCase();
    });
}

export function sortPosts(
    posts: Post[],
    by: "title" | "date" | "category"
): Post[] {
    const copy = [...posts]; // NIE mutujemy oryginału

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

export { Post, Comment, Category, summarise, filterByCategory };

export function validatePost(
    data: unknown
): { ok: true; post: Post } | { ok: false; reason: string } {

    if (typeof data !== "object" || data === null) {
        return {
            ok: false,
            reason: "Data is not an object"
        };
    }

    const obj = data as Record<string, unknown>;

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

    // category validation
    if (obj.category !== null) {

        if (
            typeof obj.category !== "object" ||
            obj.category === null
        ) {
            return {
                ok: false,
                reason: "Invalid category"
            };
        }

        const category = obj.category as Record<string, unknown>;

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
    post: obj as unknown as Post
};
}