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

// Funkcja pomocnicza do parsowania liczb (n i k)
static size_t read_nr(char const *string) {
    char *endptr;
    errno = 0;
    unsigned long value = strtoul(string, &endptr, 10);
    if (errno != 0 || *endptr != 0) {
        fatal("%s is not a valid number: %s", string);
    }
    return (size_t) value;
}

static uint16_t read_port(char const *string) {
    char *endptr;
    errno = 0;
    unsigned long port = strtoul(string, &endptr, 10);
    if (errno != 0 || *endptr != 0 || port == 0 || port > UINT16_MAX) {
        fatal("%s is not a valid port number", string);
    }
    return (uint16_t) port;
}

static struct sockaddr_in get_server_address(char const *host, uint16_t port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *address_result;
    int errcode = getaddrinfo(host, NULL, &hints, &address_result);
    if (errcode != 0) {
        fatal("getaddrinfo: %s", gai_strerror(errcode));
    }

    struct sockaddr_in send_address;
    send_address.sin_family = AF_INET;
    send_address.sin_addr.s_addr = 
            ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr;
    send_address.sin_port = htons(port);

    freeaddrinfo(address_result);
    return send_address;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fatal("usage: %s <host> <port> <n_packets> <packet_size>\n", argv[0]);
    }

    char const *host = argv[1];
    uint16_t port = read_port(argv[2]);

    size_t n_packets = read_nr(argv[3]);
    size_t packet_size = read_nr(argv[4]);

    struct sockaddr_in server_address = get_server_address(host, port);

    // Create a socket.
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        syserr("cannot create a socket");
    }

    unsigned char *buffer = malloc(packet_size);
    if (!buffer) {
        syserr("malloc");
    }
    printf("Sending %zu packets of size %zu to %s: %" PRIu16 "\n", n_packets, packet_size, host, port);

    for (size_t i = 0; i < n_packets; i++) {
        char x = (char) rand() % 256;
        memset(buffer, x, packet_size);
        int send_flags = 0;

        socklen_t address_length = (socklen_t) sizeof(server_address);
        ssize_t sent_length = sendto(socket_fd, buffer, packet_size, send_flags,
                                     (struct sockaddr *) &server_address, address_length);
            syserr("sendto");
        if ((size_t) sent_length != packet_size) {
            fprintf(stderr, "Warning: incomplete packet sent\n");
        }
    }

    printf("Finished sending.\n");

    free(buffer);
    close(socket_fd);
    return 0;
}