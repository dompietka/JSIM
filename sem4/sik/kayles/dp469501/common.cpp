#include "common.hpp"
#include <sstream>
#include <iostream>
#include <netdb.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netdb.h>

using namespace std;

timespec now_ts() {
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}

// returns elapsed seconds between two timespecs as a double
double elapsed(const timespec& from, const timespec& to) {
    return (to.tv_sec - from.tv_sec) + (to.tv_nsec - from.tv_nsec) / 1e9;
}

bool pawn_is_set(const uint8_t* pawn_row, uint8_t n) {
    return (pawn_row[n / 8] >> (7 - (n % 8))) & 1;
}

// Sets pawn N in pawn_row
void pawn_set(uint8_t* pawn_row, uint8_t n) {
    pawn_row[n / 8] |= (1 << (7 - (n % 8)));
}

// Clears pawn N in pawn_row
void pawn_clear(uint8_t* pawn_row, uint8_t n) {
    pawn_row[n / 8] &= ~(1 << (7 - (n % 8)));
}

// Returns number of bytes needed for pawn_row given max_pawn
int pawn_row_bytes(uint8_t max_pawn) {
    return max_pawn / 8 + 1;
}

bool parse_address(const std::string& addr, uint16_t port, sockaddr_in& out) {
    struct addrinfo hints{};
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    struct addrinfo* res = nullptr;
    int err = getaddrinfo(addr.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (err != 0) {
        cerr << "getaddrinfo: " << gai_strerror(err) << "\n";
        return false;
    }

    memcpy(&out, res->ai_addr, sizeof(out));
    freeaddrinfo(res);
    return true;
}


