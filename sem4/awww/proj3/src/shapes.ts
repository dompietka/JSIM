// src/shapes.ts

interface Circle {
    kind: "circle";
    radius: number;
}

interface Rectangle {
    kind: "rectangle";
    width: number;
    height: number;
}

interface Triangle {
    kind: "triangle";
    base: number;
    height: number;
}

type Shape = Circle | Rectangle | Triangle;

// =========================
// AREA
// =========================

function area(shape: Shape): number {
    switch (shape.kind) {
        case "circle":
            return Math.PI * shape.radius * shape.radius;

        case "rectangle":
            return shape.width * shape.height;

        case "triangle":
            return (shape.base * shape.height) / 2;

        default:
            return 0;
    }
}

// =========================
// SVG HELPER
// =========================

function svgEl(
    tag: string,
    attrs: Record<string, string | number>
): SVGElement {
    const el = document.createElementNS(
        "http://www.w3.org/2000/svg",
        tag
    );

    for (const [k, v] of Object.entries(attrs)) {
        el.setAttribute(k, String(v));
    }

    return el as SVGElement;
}

// =========================
// RENDER SHAPE
// =========================

function renderShape(shape: Shape): SVGElement {
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

// =========================
// SHAPES DATA
// =========================

const shapes: Shape[] = [
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

// =========================
// RENDER GALLERY
// =========================

export function renderShapesGallery(): void {
    const output = document.getElementById("output")!;

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

    output.appendChild(container);
}