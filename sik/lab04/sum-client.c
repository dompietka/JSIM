#include <endian.h>
#include <inttypes.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "sum-common.h"
#include "err.h"
#include "common.h"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fatal("usage: %s <host> <port> <n> <k>\n", argv[0]);
    }

    char const *host = argv[1];
    uint16_t port = read_port(argv[2]);
    int n = atoi(argv[3]);  
    int k = atoi(argv[4]);  

    struct sockaddr_in server_address = get_server_address(host, port);

    // Create a socket.
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        syserr("cannot create a socket");
    }

    // Connect to the server.
    if (connect(socket_fd, (struct sockaddr *) &server_address,
                (socklen_t) sizeof(server_address)) < 0) {
        syserr("cannot connect to the server");
    }

    char *buffer = malloc(k);
    if (!buffer) {
        syserr("malloc failed");
    }


    for (int i = 0; i < n; i++) {
        memset(buffer, 'A', k);

        ssize_t written_length = writen(socket_fd, buffer, k);
        if (written_length < 0) {
            syserr("writen");
        }
        else if (written_length != k) {
            fatal("incomplete writen");
        }

        printf("sent packet %d (%d bytes)\n", i + 1, k);
    }

    free(buffer);
    close(socket_fd);
    return 0;
}
