// src/utils.ts

// A generic identity function — T is inferred from the argument.
function identity<T>(value: T): T {
    return value;
}

// Returns first element or undefined
function first<T>(arr: T[]): T | undefined {
    return arr[0];
}

// Groups items by a string key
function groupBy<T>(
    items: T[],
    keyFn: (item: T) => string
): Record<string, T[]> {

    const groups: Record<string, T[]> = {};

    for (const item of items) {

        const key = keyFn(item);

        if (!groups[key]) {
            groups[key] = [];
        }

        groups[key].push(item);
    }

    return groups;
}

function pipe<T>(
    ...fns: Array<(arg: T) => T>
): (arg: T) => T {

    return (arg: T) => {
        return fns.reduce(
            (value, fn) => fn(value),
            arg
        );
    };
}

function memoize<A extends PropertyKey, R>(
    fn: (arg: A) => Promise<R>
): (arg: A) => Promise<R> {

    const cache: Record<PropertyKey, R> = {};

    return async (arg: A) => {

        // cache HIT
        if (arg in cache) {
            return cache[arg];
        }

        // cache MISS
        const result = await fn(arg);

        cache[arg] = result;

        return result;
    };
}

export { identity, first, groupBy, pipe, memoize };