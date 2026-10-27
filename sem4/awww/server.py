import socket
import threading
import time
import os
from urllib.parse import unquote_plus

semafor = threading.Lock()
counter = 0

MIME_TYPES = {
    ".html": "text/html",
    ".css": "text/css",
    ".js": "application/javascript",
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".ico": "image/x-icon",
}


def read_file(path):
    public_root = os.path.abspath("public")
    abs_path = os.path.abspath(os.path.join("public", path.lstrip("/")))

    # 🔒 Security: reject paths that escape public/
    if not abs_path.startswith(public_root + os.sep):
        return b"<h1>403 Forbidden</h1>", "403 Forbidden", "text/html"

    ext = os.path.splitext(abs_path)[1].lower()
    mime = MIME_TYPES.get(ext, "application/octet-stream")

    try:
        with open(abs_path, "rb") as f:  # binary mode works for text AND images
            return f.read(), "200 OK", mime
    except FileNotFoundError:
        return b"<h1>404 Not Found</h1>", "404 Not Found", "text/html"


def start_server():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(("localhost", 8000))

    server_socket.listen(5)

    print("Server running on http://localhost:8000 ...")
    while True:
        client_connection, client_address = server_socket.accept()
        client_thread = threading.Thread(
            target=handle_client, args=(client_connection,)
        )
        client_thread.start()


def handle_client(client_connection):
    print(f"Connection received!")
    print(f"abbcc")
    global counter
    request_data = client_connection.recv(1024).decode("utf-8")
    path = parse_request(request_data)

    if path == "/submit":
        data = parse_post_body(request_data)
        response = handle_submit(data)
        status = "200 ok"
        print("whyyyyy")
    else:
        path = parse_request(request_data)
        content, status, mime = read_file(path if path != "/" else "/index.html")
        response = generate_response(content, status, mime)

    if "ok" in status or "200" in status:
        with semafor:
            counter += 1
            i = counter
        print(f"visited {i} times")
    client_connection.sendall(response)
    client_connection.close()


def parse_request(request_data):
    if not request_data:
        return ""
    lines = request_data.splitlines()
    first_line = lines[0]
    parts = first_line.split(" ")
    path = parts[1]
    return path


# def generate_response(content, status_code="200 OK"):
#    body = content.encode('utf-8')   # encode first!

#    header = f"HTTP/1.1 {status_code}\r\n"
#    header += "Content-Type: text/html\r\n"
#    header += f"Content-Length: {len(body)}\r\n"
#    header += "\r\n"

#    return header.encode('utf-8') + body


def generate_response(content, status, mime="text/html"):
    if isinstance(content, str):
        content = content.encode()
    response_line = f"HTTP/1.1 {status}\r\n"
    response_headers = f"Content-Type: {mime}\r\nContent-Length: {len(content)}\r\n\r\n"
    return response_line.encode() + response_headers.encode() + content


def parse_post_body(request_data):
    parts = request_data.split("\r\n\r\n", 1)
    if len(parts) < 2:
        return {}
    body = parts[1]
    pairs = body.split("&")
    result = {}
    for pair in pairs:
        if "=" in pair:
            key, value = pair.split("=", 1)
            result[unquote_plus(key)] = unquote_plus(value)

    return result


def handle_submit(data):
    with open(".txt", "w", encoding="utf-8") as f:
        for klucz, wartosc in data.items():
            f.write(f"{klucz}: {wartosc}\n")

    content, status, mime = read_file("/odp.html")
    response = generate_response(content, status, mime)
    value_to_insert = next(iter(data.values())).encode("utf-8")
    response = response.replace(b"{YN}", value_to_insert)
    return response


if __name__ == "__main__":
    start_server()
