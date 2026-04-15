#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <endian.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#include "sum-common.h"
#include "err.h"
#include "common.h"

#define QUEUE_LENGTH  5
#define BUFFER_SIZE 100000

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fatal("usage: %s <port>", argv[0]);
    }

    uint16_t port = read_port(argv[1]);

    // Ignore SIGPIPE signals, so they are delivered as normal errors.
    signal(SIGPIPE, SIG_IGN);

    // Create a socket.
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        syserr("cannot create a socket");
    }

    // Bind the socket to a concrete address.
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // Listening on all interfaces.
    server_address.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *) &server_address, (socklen_t) sizeof server_address) < 0) {
        syserr("bind");
    }

    // Switch the socket to listening.
    if (listen(socket_fd, QUEUE_LENGTH) < 0) {
        syserr("listen");
    }

    // Find out what port the server is actually listening on.
    socklen_t lenght = (socklen_t) sizeof server_address;
    if (getsockname(socket_fd, (struct sockaddr *) &server_address, &lenght) < 0) {
        syserr("getsockname");
    }

    printf("listening on port %" PRIu16 "\n", ntohs(server_address.sin_port));

    for (;;) {
        struct sockaddr_in client_address;

        int client_fd = accept(socket_fd, (struct sockaddr *) &client_address,
                               &((socklen_t) {sizeof(client_address)}));
        if (client_fd < 0) {
            syserr("accept");
        }

        char const *client_ip = inet_ntoa(client_address.sin_addr);
        uint16_t client_port = ntohs(client_address.sin_port);
        printf("accepted connection from %s:%" PRIu16 "\n", client_ip, client_port);

        char buffer[BUFFER_SIZE];
        ssize_t read_length;
        size_t total_bytes = 0;

        // czas startu
        struct timespec start, now;
        clock_gettime(CLOCK_MONOTONIC, &start);

        for (;;) {
            read_length = read(client_fd, buffer, BUFFER_SIZE);

            if (read_length < 0) {
                if (errno == EINTR)
                    continue;
                syserr("read");
            }
            else if (read_length == 0) {
                printf("connection closed\n");
                break;
            }

            total_bytes += read_length;

            // aktualny czas
            clock_gettime(CLOCK_MONOTONIC, &now);

            double elapsed =
                (now.tv_sec - start.tv_sec) +
                (now.tv_nsec - start.tv_nsec) / 1e9;

            printf("last read: %zd bytes | total: %zu bytes | time: %.3f s\n",
                   read_length, total_bytes, elapsed);
        }

        close(client_fd);
    }

    close(socket_fd);
    return 0;
}
