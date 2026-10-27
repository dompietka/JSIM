#include <iostream>
#include <string>
#include <cstdlib>
#include <getopt.h>
#include <vector>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include <arpa/inet.h>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <strings.h>
#include <cerrno>
#include <climits>
#include <poll.h>
#include <algorithm>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <regex>
#include <map>

using namespace std;

// Core data structures.
struct Url {
    string scheme;
    string host;
    string path;
    int port;
    bool is_ipv6 = false;
};

// Logger with configurable verbosity levels.
class Logger {
public:
    Logger(int verbosity)
    {
        this->verbosity = verbosity;
    }

    void communication(const string& message)
    {
        log(1, message);
    }

    void critical(const string& message)
    {
        log(2, message);
    }

    void warning(const string& message)
    {
        log(3, message);
    }

    void debug(const string& message)
    {
        log(4, message);
    }

    void timestamp() {
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y.%m.%d %H.%M.%S", timeinfo);
        log(1, string(buffer));
    }

    void metadata(const string& message)
    {
        log(0, message);
    }
private:
    int verbosity;

    void log(int level, const string& message)
    {
        if (level <= verbosity) {
            cerr << message << '\n';
        }
    }
};

struct Config {
    string url;
    Url current_url;
    Url original_url;

    bool metadata = false;

    int timeout = 5000;
    int verbosity = 2;

    int ip_version = 0; // 0 = default, 4 = IPv4, 6 = IPv6

    map<string, map<string, string>> cookies;

};

struct ResolvedAddress {
    sockaddr_storage address;
    socklen_t length;
};

struct StreamState {
    int metaint = 0;
    int audio_remaining = 0;
    int metadata_remaining = 0;
    string metadata;
    bool reading_metadata_length = false;
};

// HTTP header, redirect, and cookie helpers.
bool is_redirect(int status_code)
{
    return status_code == 301 ||
        status_code == 302 ||
        status_code == 303 ||
        status_code == 307 ||
        status_code == 308 ||
        status_code == 300 ||
        status_code == 399;
}

// Searches the HTTP header block for a case-insensitive header name and returns its trimmed value.
bool get_header_value(
    const string& headers,
    const string& header_name,
    string& value)
{
    size_t position = 0;

    while (position < headers.size()) {

        size_t line_end = headers.find("\r\n", position);

        if (line_end == string::npos) {
            line_end = headers.size();
        }

        string line = headers.substr(
                position,
                line_end - position
                );

        size_t colon = line.find(':');

        if (colon != string::npos) {

            string name = line.substr(0, colon);

            if (strcasecmp(
                        name.c_str(),
                        header_name.c_str()) == 0) {

                string header_value =
                    line.substr(colon + 1);

                size_t first =
                    header_value.find_first_not_of(" \t");

                if (first == string::npos) {
                    value.clear();
                } else {

                    size_t last =
                        header_value.find_last_not_of(" \t");

                    value = header_value.substr(
                            first,
                            last - first + 1
                            );
                }

                return true;
            }
        }

        if (line_end == headers.size()) {
            break;
        }

        position = line_end + 2;
    }

    return false;
}

// Parses a Set-Cookie header, determines its domain, and stores the cookie in the cookie map.
bool parse_set_cookie(
    const string& set_cookie,
    map<string, map<string, string>>& cookies,
    const string& host,
    Logger& logger)
{
    string domain;
    string cookie_name;
    string cookie_value;

    // The cookie name-value pair appears before the first ';'.
    size_t semicolon = set_cookie.find(';');
    string cookie_part = set_cookie.substr(0, semicolon);

    size_t equal_pos = cookie_part.find('=');

    if (equal_pos == string::npos) {
        logger.debug("Invalid Set-Cookie header: " + set_cookie);
        return false;
    }

    cookie_name = cookie_part.substr(0, equal_pos);
    cookie_value = cookie_part.substr(equal_pos + 1);

    // Use the current host when the cookie does not specify a Domain attribute.
    size_t domain_pos = set_cookie.find("Domain=");

    if (domain_pos == string::npos) {
        domain = host;
    }
    else {
        domain_pos += strlen("Domain=");

        size_t domain_end = set_cookie.find(';', domain_pos);

        if (domain_end == string::npos) {
            domain = set_cookie.substr(domain_pos);
        }
        else {
            domain = set_cookie.substr(
                    domain_pos,
                    domain_end - domain_pos
                    );
        }
    }

    logger.debug(
            "Found cookie: name=" + cookie_name +
            ", value=" + cookie_value +
            ", domain=" + domain
            );

    cookies[domain][cookie_name] = cookie_value;

    return true;
}

// Extracts all Set-Cookie headers from an HTTP response and stores the parsed cookies.
bool get_cookies(
    const string& headers,
    const string& host,
    map<string, map<string, string>>& cookies,
    Logger& logger)
{
    size_t position = 0;

    while (position < headers.size()) {

        size_t line_end = headers.find("\r\n", position);

        if (line_end == string::npos) {
            line_end = headers.size();
        }

        string line = headers.substr(
                position,
                line_end - position
                );

        size_t colon = line.find(':');

        if (colon != string::npos) {

            string name = line.substr(0, colon);

            if (strcasecmp(name.c_str(), "Set-Cookie") == 0) {

                string set_cookie = line.substr(colon + 1);

                size_t first =
                    set_cookie.find_first_not_of(" \t");

                if (first != string::npos) {
                    set_cookie = set_cookie.substr(first);
                }

                if (!parse_set_cookie(
                            set_cookie,
                            cookies,
                            host, logger
                            )) return false;
            }
        }

        if (line_end == headers.size()) {
            break;
        }

        position = line_end + 2;
    }

    return true;
}

// Converts a resolved IPv4 or IPv6 socket address into a printable address-and-port string.
string print_address(const ResolvedAddress& address)
{
    char ip[INET6_ADDRSTRLEN];
    int port;

    if (address.address.ss_family == AF_INET) {
        const sockaddr_in* addr =
            reinterpret_cast<const sockaddr_in*>(&address.address);

        inet_ntop(
                AF_INET,
                &addr->sin_addr,
                ip,
                sizeof(ip)
                );

        port = ntohs(addr->sin_port);

        return string(ip) + ":" + to_string(port);
    }
    else {
        const sockaddr_in6* addr =
            reinterpret_cast<const sockaddr_in6*>(&address.address);

        inet_ntop(
                AF_INET6,
                &addr->sin6_addr,
                ip,
                sizeof(ip)
                );

        port = ntohs(addr->sin6_port);

        return "[" + string(ip) + "]:" + to_string(port);
    }
}

// Resolve the current host to usable IPv4/IPv6 socket addresses.
// Resolves the current host to usable IPv4/IPv6 socket addresses while respecting the selected IP version.
bool resolve(
    Config& config,
    vector<ResolvedAddress>& addresses,
    Logger& logger)
{
    addresses.clear();

    logger.timestamp();
    logger.communication("resolving name " + config.current_url.host);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    string port_string = to_string(config.current_url.port);

    struct addrinfo* result = nullptr;

    if (getaddrinfo(
                config.current_url.host.c_str(),
                port_string.c_str(),
                &hints,
                &result
                )) {
        logger.critical("Failed to resolve host: " + config.current_url.host);
        return false;
    }

    for (
            struct addrinfo* current = result;
            current != nullptr;
            current = current->ai_next
        ) {
        if (current->ai_family != AF_INET &&
                current->ai_family != AF_INET6) {
            continue;
        }

        if (config.ip_version == 4 &&
                current->ai_family != AF_INET) {
            continue;
        }

        if (config.ip_version == 6 &&
                current->ai_family != AF_INET6) {
            continue;
        }

        ResolvedAddress address;

        address.length = current->ai_addrlen;

        memcpy(
                &address.address,
                current->ai_addr,
                current->ai_addrlen
              );

        addresses.push_back(address);
    }
    logger.debug("Resolved " + to_string(addresses.size()) + " addresses for host: " + config.current_url.host);
    logger.debug("IP version used: " + to_string(config.ip_version));
    freeaddrinfo(result);
    return !addresses.empty();
}

// Owns the TCP socket and, for HTTPS, the associated TLS state.
class Connection {
public:
    Connection() : sock(-1), ctx(nullptr), ssl(nullptr), secure(false) {}

    ~Connection() {
        close();
    }

    // Establishes a TCP connection and enables TLS when the URL uses HTTPS.
    bool connect(
        const vector<ResolvedAddress>& addresses,
        const Url& url,
        Logger& logger)
    {
        if (!connect_tcp(addresses, logger)) {
            return false;
        }

        if (url.scheme == "http") {
            secure = false;
            return true;
        }

        if (url.scheme == "https") {
            return setup_tls(url.host, logger);
        }

        return false;
    }

    // Sends the complete buffer over the active plain TCP or TLS connection, retrying when required.
    bool send_all(const string& data, Logger& logger)
    {
        size_t total_sent = 0;

        while (total_sent < data.size()) {
            ssize_t sent;
            if (secure) {
                sent = SSL_write(
                        ssl,
                        data.data() + total_sent,
                        data.size() - total_sent
                        );

                if (sent <= 0) {
                    int error = SSL_get_error(ssl, sent);
                    if (error == SSL_ERROR_WANT_WRITE || error == SSL_ERROR_WANT_READ) {
                        logger.warning("SSL_write wants to read/write, retrying...");
                        continue;
                    }
                    logger.critical(
                            "Failed to send data over TLS: " + string(ERR_error_string(error, nullptr))
                            );
                    return false;
                }
            } else {
                sent = send(
                        sock,
                        data.data() + total_sent,
                        data.size() - total_sent,
                        0
                        );
                if (sent < 0) {
                    if (errno == EINTR) {
                        continue;
                    }

                    logger.critical(
                            "Failed to send data: " + string(strerror(errno))
                            );
                    return false;
                }

                if (sent == 0) {
                    logger.critical("Connection closed while sending data.");
                    return false;
                }
            }

            total_sent += sent;
        }
        logger.debug("Sent whole message successfully.");
        return true;

    }
    // Receives data through TCP or TLS and maps retryable TLS/read conditions to the caller's return convention.
    ssize_t receive(
        char* buffer,
        size_t size,
        Logger& logger)
    {
        if (!secure) {
            ssize_t received = ::recv(sock, buffer, size, 0);

            logger.debug(
                    "recv returned: " + to_string(received) +
                    ", errno: " + to_string(errno)
                    );

            if (received < 0 && errno == EINTR) {
                return -2;
            }

            return received;
        }

        int received = SSL_read(ssl, buffer, size);

        if (received > 0) {
            return received;
        }

        int error = SSL_get_error(ssl, received);

        switch (error) {
            case SSL_ERROR_ZERO_RETURN:
                return 0;

            case SSL_ERROR_WANT_READ:
                return -2;

            case SSL_ERROR_SYSCALL:
            case SSL_ERROR_SSL:
            default:
                logger.critical("TLS receive failed.");
                return -1;
        }
    }
    // Releases TLS resources and closes the underlying socket, leaving the connection in a reset state.
    void close() {
        if (ssl != nullptr) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            ssl = nullptr;
        }
        if (ctx != nullptr) {
            SSL_CTX_free(ctx);
            ctx = nullptr;
        }
        if (sock != -1) {
            ::close(sock);
            sock = -1;
        }
        secure = false;
    }

    int get_socket() const {
        return sock;
    }

private:
    int sock;
    SSL_CTX* ctx;
    SSL* ssl;
    bool secure;

    // Tries the resolved addresses in order until a TCP connection succeeds or all candidates fail.
    bool connect_tcp(
        const vector<ResolvedAddress>& addresses,
        Logger& logger)
    {
        for (const auto& target : addresses) {
            sock = socket(target.address.ss_family, SOCK_STREAM, 0);
            if (sock < 0) {
                logger.warning("Failed to create socket for address " + print_address(target) + ": " + strerror(errno) + ". Skipping to next address.");
                continue;
            }

            logger.communication("connecting to server " + print_address(target) + "...");
            if (::connect(sock, reinterpret_cast<const sockaddr*>(&target.address), target.length) == 0) {
                return true;
            }

            ::close(sock);
            sock = -1;
            logger.warning("Failed to connect to address " + print_address(target) + ": " + strerror(errno) + ". Skipping to next address.");

        }

        logger.critical("Failed to connect to any resolved address.");
        return false;
    }

    // Configures TLS, verifies the server certificate and hostname, and completes the TLS handshake.
    bool setup_tls(const string& hostname, Logger& logger)
    {
        ctx = SSL_CTX_new(TLS_client_method());

        if (ctx == nullptr) {
            logger.critical("Failed to create TLS context.");
            ERR_print_errors_fp(stderr);
            return false;
        }

        if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
            logger.critical("Failed to load system CA certificates.");
            ERR_print_errors_fp(stderr);
            SSL_CTX_free(ctx);
            ctx = nullptr;
            return false;
        }

        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);

        ssl = SSL_new(ctx);

        if (ssl == nullptr) {
            logger.critical("Failed to create TLS connection.");
            ERR_print_errors_fp(stderr);
            SSL_CTX_free(ctx);
            ctx = nullptr;
            return false;
        }

        if (SSL_set_fd(ssl, sock) != 1) {
            logger.critical("Failed to attach socket to TLS connection.");
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            ssl = nullptr;
            SSL_CTX_free(ctx);
            ctx = nullptr;
            return false;
        }

        if (SSL_set_tlsext_host_name(ssl, hostname.c_str()) != 1) {
            logger.critical("Failed to set TLS server name.");
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            ssl = nullptr;
            SSL_CTX_free(ctx);
            ctx = nullptr;
            return false;
        }

        if (SSL_set1_host(ssl, hostname.c_str()) != 1) {
            logger.critical("Failed to set TLS hostname verification.");
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            ssl = nullptr;
            SSL_CTX_free(ctx);
            ctx = nullptr;
            return false;
        }

        logger.debug("Starting TLS handshake...");

        if (SSL_connect(ssl) != 1) {
            logger.critical("TLS handshake failed.");
            SSL_free(ssl);
            ssl = nullptr;
            SSL_CTX_free(ctx);
            ctx = nullptr;
            return false;
        }

        long verify_result = SSL_get_verify_result(ssl);

        if (verify_result != X509_V_OK) {
            logger.critical(
                    "TLS certificate verification failed: " +
                    string(X509_verify_cert_error_string(verify_result))
                    );

            SSL_shutdown(ssl);
            SSL_free(ssl);
            ssl = nullptr;

            SSL_CTX_free(ctx);
            ctx = nullptr;

            return false;
        }

        secure = true;

        logger.debug("TLS connection established.");
        logger.debug(
                "TLS version: " +
                string(SSL_get_version(ssl))
                );

        logger.debug(
                "TLS cipher: " +
                string(SSL_get_cipher(ssl))
                );

        return true;
    }

};

// Command-line and URL parsing helpers.
// Parses a complete decimal integer string and verifies that the result is within the requested range.
static bool parse_integer(
    const string& text,
    int min_value,
    int max_value,
    int& result)
{
    try {
        size_t position = 0;

        int value = stoi(text, &position);

        if (position != text.size()) {
            return false;
        }

        if (value < min_value || value > max_value) {
            return false;
        }

        result = value;
        return true;
    }
    catch (...) {
        return false;
    }
}

// Parses command-line options, validates their values, and writes the resulting settings to Config.
bool parse_arguments(
    int argc,
    char* argv[],
    Config& config)
{
    bool url_given = false;
    int option;

    bool force_ipv4 = false;
    bool force_ipv6 = false;

    while ((option = getopt(argc, argv, "u:mt:v:46q")) != -1) {
        switch (option) {

            case 'u':
                if (url_given) {
                    cerr << "argument -u specified more than once\n";
                    return false;
                }

                config.url = optarg;
                url_given = true;
                break;

            case 'm':
                config.metadata = true;
                break;

            case 't':
                if (!parse_integer(optarg, 100, 100000, config.timeout)) {
                    cerr << "invalid timeout\n";
                    return false;
                }
                break;

            case '4':
                force_ipv4 = true;
                break;

            case '6':
                force_ipv6 = true;
                break;

            case 'v':
                if (!parse_integer(optarg, 0, 4, config.verbosity)) {
                    cerr << "invalid verbosity\n";
                    return false;
                }
                break;

            case 'q':
                config.verbosity = 0;
                break;

            case '?':
                return false;

            default:
                return false;
        }
    }

    if ((force_ipv4 && force_ipv6) || (!force_ipv4 && !force_ipv6)) {
        config.ip_version = 0;
    }
    else if (force_ipv4) {
        config.ip_version = 4;
    }
    else if (force_ipv6) {
        config.ip_version = 6;
    }

    if (!url_given) {
        cerr << "missing required argument: -u\n";
        return false;
    }

    if (optind != argc) {
        cerr << "unexpected argument: " << argv[optind] << '\n';
        return false;
    }

    return true;
}

// Parses an HTTP/HTTPS URL into scheme, host, port, path, and IPv6 information, applying default ports when needed.
bool parse_url(const string& url, Url& result)
{
    size_t scheme_end = url.find("://");
    string url_rest = url;

    if (scheme_end == string::npos) {
        return false;
    }

    result.scheme = url.substr(0, scheme_end);

    if (result.scheme != "http" && result.scheme != "https") {
        return false;
    }

    if (result.scheme == "http") {
        result.port = 80;
    } else {
        result.port = 443;
    }

    // Split the authority from the path or query.
    url_rest = url_rest.substr(scheme_end + 3);
    size_t path_start = url_rest.find_first_of("/?");
    string host_port_str;

    if (path_start == string::npos) {
        host_port_str = url_rest;
        result.path = "/";
    } else {
        host_port_str = url_rest.substr(0, path_start);
        if (url_rest[path_start] == '/') {
            result.path = url_rest.substr(path_start);
        } else {
            // HTTP request targets that start with a query still need a leading '/'.
            result.path = "/" + url_rest.substr(path_start);
        }
    }

    // Bracketed IPv6 host, optionally followed by an explicit port.
    if (host_port_str[0] == '[') {

        size_t closing_bracket = host_port_str.find(']');

        if (closing_bracket == string::npos) {
            cerr << "Invalid IPv6 address: missing ']'\n";
            return false;
        }

        result.host = host_port_str.substr(
                1,
                closing_bracket - 1
                );

        if (result.host.empty()) {
            cerr << "Missing IPv6 address\n";
            return false;
        }
        result.is_ipv6 = true;
        if (closing_bracket + 1 < host_port_str.size()) {

            if (host_port_str[closing_bracket + 1] != ':') {
                cerr << "Invalid IPv6 host/port format\n";
                return false;
            }

            string port_str =
                host_port_str.substr(closing_bracket + 2);

            if (port_str.empty()) {
                cerr << "Missing port\n";
                return false;
            }

            if (!parse_integer(
                        port_str,
                        1,
                        65535,
                        result.port)) {

                cerr << "Invalid port: " << port_str << '\n';
                return false;
            }
        }
    } else {

        size_t colon_pos = host_port_str.rfind(':');
        if (colon_pos != string::npos) {
            result.host = host_port_str.substr(0, colon_pos);
            string port_str = host_port_str.substr(colon_pos + 1);
            if (!parse_integer(port_str, 1, 65535, result.port)) {
                cerr << "Invalid port: " << port_str << '\n';
                return false;
            }
        } else {
            result.host = host_port_str;

        }
    }
    if (result.host.empty()) {
        cerr << "Missing host in URL\n";
        return false;
    }

    return true;
}

// Resolves an absolute or relative HTTP redirect location against the current URL.
bool resolve_redirect_url(
    const Url& current_url,
    const string& location,
    Url& result)
{
    if (location.find("://") != string::npos) {
        return parse_url(location, result);
    }

    result.scheme = current_url.scheme;
    result.host = current_url.host;
    result.port = current_url.port;

    if (!location.empty() && location[0] == '/') {
        result.path = location;
        return true;
    }

    size_t slash = current_url.path.rfind('/');

    if (slash == string::npos) {
        result.path = "/" + location;
    } else {
        result.path = current_url.path.substr(0, slash + 1) + location;
    }

    return true;
}

// Checks whether a stored cookie domain applies to the current host, including matching subdomains.
bool cookie_matches_host(const string& host, const string& cookie_domain)
{
    if (host == cookie_domain) {
        return true;
    }

    if (host.size() > cookie_domain.size() &&
            host.compare(
                host.size() - cookie_domain.size(),
                cookie_domain.size(),
                cookie_domain
                ) == 0 &&
            host[host.size() - cookie_domain.size() - 1] == '.') {
        return true;
    }

    return false;
}

// HTTP request and response helpers.
// Builds the HTTP GET request, including host, metadata preference, and cookies valid for the current host.
string build_http_request(const Config& config)
{
    string request;

    request += "GET " + config.current_url.path + " HTTP/1.1\r\n";
    if (!config.current_url.is_ipv6) request += "Host: " + config.current_url.host + "\r\n";
    else request += "Host: [" + config.current_url.host + "]\r\n";

    request += "Connection: Keep-Alive\r\n";

    if (config.metadata) {
        request += "Icy-MetaData: 1\r\n";
    }

    for (auto const& [domain, cookie_names] : config.cookies) {
        if (cookie_matches_host(config.current_url.host, domain)) {
            for (auto const& [cookie_name, cookie_val] : cookie_names)
                request += "Cookie: " + cookie_name + "=" + cookie_val + "\r\n";
        }
    }

    request += "\r\n";

    return request;
}

// Reads from the connection until the complete HTTP header block is available and records where it ends.
bool receive_headers(
    Connection& connection,
    string& data,
    size_t& header_end,
    Logger& logger)
{
    char buffer[4096];

    data.clear();

    while (true) {
        ssize_t received = connection.receive(buffer, sizeof(buffer), logger);

        if (received <= 0) {
            return false;
        }

        data.append(buffer, received);

        size_t position = data.find("\r\n\r\n");

        if (position != string::npos) {
            header_end = position;
            return true;
        }
    }
}

// Extracts and validates the three-digit HTTP status code from the response status line.
bool get_status_code(const string& headers, int& status_code)
{
    size_t line_end = headers.find("\r\n");

    if (line_end == string::npos) {
        return false;
    }

    string status_line = headers.substr(0, line_end);

    if (status_line.size() < 10) {
        return false;
    }

    size_t space = status_line.find(' ');

    if (space == string::npos) {
        return false;
    }

    if (space + 4 > status_line.size()) {
        return false;
    }

    string code_string = status_line.substr(space + 1, 3);

    if (!parse_integer(code_string, 100, 599, status_code)) {
        return false;
    }

    return true;
}

// Streaming state machine and event loop.
enum class StreamResult {
    Success,
    Timeout,
    Error
};

// Processes a received stream chunk, writing audio to stdout and separating ICY metadata according to the stream state.
StreamResult stream_data(
    const char* data,
    size_t size,
    StreamState& state,
    Logger& logger)
{
    size_t position = 0;

    while (position < size) {

        if (state.metaint == 0) {
            cout.write(data + position, size - position);
            cout.flush();
            return StreamResult::Success;
        }

        if (state.audio_remaining > 0) {
            size_t bytes = min(
                    static_cast<size_t>(state.audio_remaining),
                    size - position
                    );

            cout.write(data + position, bytes);
            cout.flush();

            position += bytes;
            state.audio_remaining -= bytes;

            continue;
        }

        if (!state.reading_metadata_length) {
            state.reading_metadata_length = true;

            unsigned char length =
                static_cast<unsigned char>(data[position]);

            position++;

            state.metadata_remaining = length * 16;
            state.metadata.clear();

            if (state.metadata_remaining == 0) {
                state.audio_remaining = state.metaint;
                state.reading_metadata_length = false;
            }

            continue;
        }

        size_t bytes = min(
                static_cast<size_t>(state.metadata_remaining),
                size - position
                );

        state.metadata.append(data + position, bytes);

        position += bytes;
        state.metadata_remaining -= bytes;

        if (state.metadata_remaining == 0) {
            if (!state.metadata.empty()) {

                while (!state.metadata.empty() &&
                        state.metadata.back() == '\0') {
                    state.metadata.pop_back();
                }

                if (!state.metadata.empty()) {
                    logger.metadata(state.metadata);
                }
            }

            state.audio_remaining = state.metaint;
            state.reading_metadata_length = false;
            state.metadata.clear();
        }
    }

    return StreamResult::Success;
}

// Runs the streaming loop, handles socket/stdin events and timeouts, and passes received data to the stream parser.
StreamResult start_streaming(
    const string& initial_audio,
    Connection& connection,
    Logger& logger,
    bool metadata,
    int metaint,
    const Config& config)
{
    StreamState state;
    state.metaint = metaint;
    state.audio_remaining = metaint;

    if (!metadata) {
        state.metaint = 0;
    }
    logger.debug("Starting audio streaming...");

    if (stream_data(initial_audio.data(), initial_audio.size(), state, logger) == StreamResult::Error) {
        logger.critical("Failed to process initial audio data.");
        return StreamResult::Error;
    }

    struct pollfd fds[2];

    fds[0].fd = connection.get_socket();
    fds[0].events = POLLIN;

    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    char buffer[8192];
    char input_buffer[1000];
    string leftover;

    while (true) {
        int result = poll(fds, 2, config.timeout);

        if (result < 0) {
            if (errno == EINTR)
                continue;

            logger.critical("poll() failed.");
            return StreamResult::Error;
        }

        if (result == 0) {
            logger.debug("Connection timeout.");
            return StreamResult::Timeout;
        }

        if (fds[1].revents & POLLIN) {
            ssize_t received = ::read(STDIN_FILENO, input_buffer, sizeof(input_buffer));

            if (received > 0) {
                string input = leftover + string(input_buffer, received);

                if (input.find("quit\n") != string::npos) {
                    connection.close();
                    logger.debug("User requested to quit. Closing connection.");
                    return StreamResult::Success;
                }

                // Keep a short suffix so "quit\n" can be detected across read boundaries.
                if (input.size() > 10) {
                    leftover = input.substr(input.size() - 10);
                } else {
                    leftover = input;
                }
            }
        }

        logger.debug(
                "poll result=" + to_string(result) +
                ", socket revents=" + to_string(fds[0].revents) +
                ", stdin revents=" + to_string(fds[1].revents)
                );

        if (fds[0].revents & POLLIN) {
            ssize_t received = connection.receive(buffer, sizeof(buffer), logger);

            if (received == 0) {
                logger.communication("Server closed the connection.");
                return StreamResult::Success;
            }

            if (received == -1) {
                return StreamResult::Error;
            }

            if (received == -2) {
                continue;
            }

            if (stream_data(
                        buffer,
                        received,
                        state,
                        logger) == StreamResult::Error) {
                return StreamResult::Error;
            }
        }

        if (fds[0].revents & (POLLERR | POLLNVAL)) {
            logger.critical("Socket error.");
            return StreamResult::Error;
        }

        if (fds[0].revents & POLLHUP) {
            logger.communication("Server closed the connection.");
            return StreamResult::Success;
        }

        if (fds[1].revents & POLLHUP) {
            fds[1].fd = -1;
        }
    }

    return StreamResult::Success;
}

// Main HTTP workflow: resolve, connect, handle redirects, and stream data.
// Coordinates the complete HTTP session: resolution, connection, redirects, cookies, response handling, streaming, and reconnects.
bool handle_communication(Config& config, Logger& logger)
{
    int redirect_count = 0;
    while (true) {
        vector<ResolvedAddress> addresses;
        if (!resolve(config, addresses, logger)) {
            return false;
        }
        Connection connection;

        if (!connection.connect(addresses, config.current_url, logger)) {
            return false;
        }

        logger.debug("Successfully connected to server.");

        string request = build_http_request(config);

        if (!connection.send_all(request, logger)) {
            connection.close();
            return false;
        }

        logger.communication(request);

        string response;
        size_t header_end;

        if (!receive_headers(connection, response, header_end, logger)) {
            connection.close();
            return false;
        }

        string headers = response.substr(0, header_end);
        logger.communication(headers + "\r\n\r\n");

        int status_code;

        if (!get_status_code(headers, status_code)) {
            connection.close();
            logger.critical("Invalid HTTP response");
            return false;
        }

        logger.debug("HTTP status code: " + to_string(status_code));

        logger.debug("Checking for Set-Cookie header...");

        if (!get_cookies(headers, config.current_url.host, config.cookies, logger)){
            logger.warning("Couldn't parse cookies correctly");
        }

        if (status_code == 200) {

            string initial_audio = response.substr(header_end + 4);

            if (initial_audio.empty()) {
                logger.debug("No initial audio data received.");
            }

            logger.debug(
                    "Final connection established, ready to stream audio."
                    );

            int metaint = 0;
            string metaint_value;

            if (get_header_value(headers, "icy-metaint", metaint_value)) {
                if (!parse_integer(metaint_value, 1, INT_MAX, metaint)) {
                    logger.critical("Invalid icy-metaint.");
                    connection.close();
                    return false;
                }

            }

            if (config.metadata) {
                logger.debug("Metadata is enabled. Starting streaming with metadata.");
                logger.debug("ICY metadata interval: " + to_string(metaint));
            } else {
                logger.debug("Metadata is disabled. Starting streaming without metadata.");
            }

            StreamResult res = start_streaming(initial_audio, connection, logger, config.metadata, metaint, config);
            if (res == StreamResult::Error) {
                connection.close();
                return false;
            }
            if (res == StreamResult::Timeout) {
                logger.communication("data receiving timeout.");
                connection.close();
                config.cookies.clear();
                logger.debug("Attempting to reconnect...");
                config.current_url = config.original_url;
                redirect_count = 0;
                continue;
            }
            return true;
        }

        else if (is_redirect(status_code)) {

            string new_address;

            if (!get_header_value(
                        headers,
                        "Location",
                        new_address)) {

                logger.critical(
                        "Failed to extract redirect location from headers."
                        );

                connection.close();
                return false;
            }

            if (new_address.empty())
                return false;

            logger.debug(
                    "Redirect location: " + new_address
                    );

            Url redirected_url;

            if (!resolve_redirect_url(
                        config.current_url,
                        new_address,
                        redirected_url)) {

                logger.critical(
                        "Failed to resolve redirect URL: " +
                        new_address
                        );

                connection.close();
                return false;
            }

            config.current_url = redirected_url;
            connection.close();
            redirect_count++;

            if (redirect_count > 10) {
                logger.warning("Too many HTTP redirects:" + to_string(redirect_count));
            }

            continue;
        }

        else if (status_code >= 300 && status_code < 400) {
            logger.warning("3xx HTTP response status, but not redirect - ignoring");
            return true;
        }

        else if (status_code >= 400 && status_code < 500) {

            logger.critical(
                    "HTTP client error: " +
                    to_string(status_code)
                    );

            connection.close();
            return false;
        }

        else if (status_code >= 500 && status_code < 600) {

            logger.critical(
                    "HTTP server error: " +
                    to_string(status_code)
                    );

            connection.close();
            return false;
        }

        else {

            logger.critical(
                    "Unexpected HTTP status: " +
                    to_string(status_code)
                    );

            connection.close();
            return false;
        }
    }
}

// Parses program input, initializes the starting URL and logger, and runs the HTTP streaming client.
int main(int argc, char* argv[])
{
    Config config;

    if (!parse_arguments(argc, argv, config)) {
        return EXIT_FAILURE;
    }
    Logger logger(config.verbosity);
    Url parsed_url;
    if (!parse_url(config.url, parsed_url)) {
        cerr << "Failed to parse URL: " << config.url << '\n';
        return EXIT_FAILURE;
    }

    config.current_url = parsed_url;
    config.original_url = parsed_url;

    if (!handle_communication(config, logger)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
