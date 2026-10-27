#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "err.h"

#define MAX_BUFFER_SIZE 10000 

static uint16_t read_port(char const *string) {
    char *endptr;
    errno = 0;
    unsigned long port = strtoul(string, &endptr, 10);
    if (errno != 0 || *endptr != 0 || port == 0 || port > UINT16_MAX) {
        fatal("%s is not a valid port number", string);
    }
    return (uint16_t) port;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fatal("usage: %s <port>", argv[0]);
    }

    uint16_t port = read_port(argv[1]);

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        syserr("cannot create a socket");
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *) &server_address, (socklen_t) sizeof(server_address)) < 0) {
        syserr("bind");
    }

    printf("listening on port %" PRIu16 "\n", port);

    static char buffer[MAX_BUFFER_SIZE];
    ssize_t received_length;

    while (1) { 
        memset(buffer, 0, sizeof(buffer)); // Clean the buffer.
        
        int flags = 0;
        struct sockaddr_in client_address;
        socklen_t address_length = (socklen_t) sizeof(client_address);

        received_length = recvfrom(socket_fd, buffer, MAX_BUFFER_SIZE, 0,
                                           (struct sockaddr *) &client_address, &address_length);
        
        if (received_length < 0) {
            syserr("recvfrom");
        }
        printf("Received %zd bytes\n", received_length);
        if (received_length == 0) {
            printf("closing connection\n");
            break;
        }
    }

    close(socket_fd);
    return 0;
}