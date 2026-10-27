#include "common.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <sstream>


using namespace std;

// ─── Argument parsing ─────────────────────────────────────────────────

struct ClientConfig {
    string address;    // from -a
    uint16_t    port;       // from -p
    string message;    // from -m
    int         timeout;    // from -t
};

static ClientConfig parse_args(int argc, char* argv[]) {
    ClientConfig cfg{};
    bool has_a = false, has_p = false, has_m = false, has_t = false;

    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "-a" && i + 1 < argc) {
            if (has_a) { cerr << "Duplicate -a: using first value\n"; i++; continue; }
            cfg.address = argv[++i];
            has_a = true;
        } else if (string(argv[i]) == "-p" && i + 1 < argc) {
            if (has_p) {cerr << "Duplicate -p: using first value\n"; i++; continue; }
            char* end;
            long v = strtol(argv[++i], &end, 10);
            if (*end != '\0' || v < 1 || v > 65535) {  // client: port 1-65535, no zero
                cerr << "Invalid port\n"; exit(1);
            }
            cfg.port = (uint16_t)v;
            has_p = true;
        } else if (string(argv[i]) == "-m" && i + 1 < argc) {
            if (has_m) {cerr << "Duplicate -m: using first value\n"; i++; continue; }
            cfg.message = argv[++i];
            has_m = true;
        } else if (string(argv[i]) == "-t" && i + 1 < argc) {
            if (has_t) {cerr << "Duplicate -t: using first value\n"; i++; continue; }
            char* end;
            long v = strtol(argv[++i], &end, 10);
            if (*end != '\0' || v < 1 || v > 99) {
                cerr << "Invalid timeout\n"; exit(1);
            }
            cfg.timeout = (int)v;
            has_t = true;
        } else {
            cerr << "Unknown argument: " << argv[i] << "\n";
            exit(1);
        }
    }

    if (!has_a || !has_p || !has_m || !has_t) {
        cerr << "Missing required arguments (-a -p -m -t)\n";
        exit(1);
    }

    return cfg;
}

// ─── Message parsing (-m field) ───────────────────────────────────────
// Format: slash-separated decimal values, e.g. "0/42" or "1/42/7/3"

static vector<uint32_t> parse_message_fields(const string& m) {
    vector<uint32_t> fields;
    string token;
    istringstream ss(m);

    while (getline(ss, token, '/')) {
        char* end;
        unsigned long v = strtoul(token.c_str(), &end, 10);
        if (*end != '\0') {
            cerr << "Invalid message field: " << token << "\n";
            exit(1);
        }
        fields.push_back((uint32_t)v);
    }

    return fields;
}

// Builds raw bytes to send based on msg_type (first field)
// Returns number of bytes written into buf
static ssize_t build_message(const vector<uint32_t>& fields, uint8_t* buf) {
    //buffsize is always the same: 64
    if (fields.empty()) {
        cerr << "Empty message\n";
        exit(1);
    }

    uint8_t msg_type = (uint8_t)fields[0];

    switch (msg_type) {
        case MSG_JOIN: {
            if (fields.size() != 2) { cerr << "MSG_JOIN needs 2 fields\n"; exit(1); }
            MsgJoin msg{};
            msg.msg_type  = MSG_JOIN;
            msg.player_id = htonl(fields[1]);
            if (fields[1] == 0) { cerr << "player_id must be nonzero\n"; exit(1); }
            memcpy(buf, &msg, sizeof(msg));
            return sizeof(msg);
        }
        case MSG_MOVE_1: {
            if (fields.size() != 4) { cerr << "MSG_MOVE_1 needs 4 fields\n"; exit(1); }
            MsgMove1 msg{};
            msg.msg_type  = MSG_MOVE_1;
            msg.player_id = htonl(fields[1]);
            msg.game_id   = htonl(fields[2]);
            msg.pawn      = (uint8_t)fields[3];
            memcpy(buf, &msg, sizeof(msg));
            return sizeof(msg);
        }
        case MSG_MOVE_2: {
            if (fields.size() != 4) { cerr << "MSG_MOVE_2 needs 4 fields\n"; exit(1); }
            MsgMove2 msg{};
            msg.msg_type  = MSG_MOVE_2;
            msg.player_id = htonl(fields[1]);
            msg.game_id   = htonl(fields[2]);
            msg.pawn      = (uint8_t)fields[3];
            memcpy(buf, &msg, sizeof(msg));
            return sizeof(msg);
        }
        case MSG_KEEP_ALIVE: {
            if (fields.size() != 3) { cerr << "MSG_KEEP_ALIVE needs 3 fields\n"; exit(1); }
            MsgKeepAlive msg{};
            msg.msg_type  = MSG_KEEP_ALIVE;
            msg.player_id = htonl(fields[1]);
            msg.game_id   = htonl(fields[2]);
            memcpy(buf, &msg, sizeof(msg));
            return sizeof(msg);
        }
        case MSG_GIVE_UP: {
            if (fields.size() != 3) { cerr << "MSG_GIVE_UP needs 3 fields\n"; exit(1); }
            MsgGiveUp msg{};
            msg.msg_type  = MSG_GIVE_UP;
            msg.player_id = htonl(fields[1]);
            msg.game_id   = htonl(fields[2]);
            memcpy(buf, &msg, sizeof(msg));
            return sizeof(msg);
        }
        default:
            cerr << "Unknown msg_type: " << (int)msg_type << "\n";
            exit(1);
    }
}

// ─── Human-readable output ───────────────────────────────────────────

string game_state_to_string(const GameState& gs) {
    ostringstream oss;

    oss << "game_id=" << gs.game_id
        << " player_a=" << gs.player_a_id
        << " player_b=" << (gs.player_b_id == 0 ? string("(none)") : to_string(gs.player_b_id));

    const char* status_str[] = {
        "WAITING_FOR_OPPONENT", "TURN_A", "TURN_B", "WIN_A", "WIN_B"
    };
    oss << " status=";
    if (gs.status < 5)
        oss << status_str[gs.status];
    else
        oss << (int)gs.status;

    oss << " max_pawn=" << (int)gs.max_pawn;
    oss << " pawns=";

    for (int i = 0; i <= gs.max_pawn; i++) {
        oss << (pawn_is_set(gs.pawn_row, i) ? '1' : '0');
        // space every 8 for readability
        if ((i + 1) % 8 == 0 && i < gs.max_pawn)
            oss << ' ';
    }

    return oss.str();
}

// ─── Wrong message output ─────────────────────────────────────────

string wrong_msg_to_string(const MsgWrongMsg& wm) {
    ostringstream oss;
    oss << "ERROR: server could not interpret byte at index "
        << (int)wm.error_index
        << " | echo: ";
    for (int i = 0; i < 12; i++)
        oss << hex << (int)wm.echo[i] << ' ';
    return oss.str();
}

// ─── Response decoding ────────────────────────────────────────────────

static void decode_message(const uint8_t* buf, ssize_t n) {

    if (n < 14) {
        cout << "Response too short: cannot translate to readable format: " << n << " bytes\n";
        exit(0);
    }

    uint8_t status = buf[12];

    if (status == STATUS_WRONG_MSG) {
        // ── MSG_WRONG_MSG: exactly 14 bytes ──────────────────────────
        if (n != 14) {
            cout << "Unexpected MSG_WRONG_MSG length: cannot translate to readable format: " << n << "\n";
            exit(0);
        }
        MsgWrongMsg wm{};
        memcpy(&wm, buf, sizeof(wm));
        cout << wrong_msg_to_string(wm) << "\n";

    } else if (status <= 4) {
        // ── MSG_GAME_STATE ────────────────────────────────────────────
        GameState gs{};
        memcpy(&gs, buf, min((ssize_t)sizeof(gs), n));
        gs.game_id     = ntohl(gs.game_id);
        gs.player_a_id = ntohl(gs.player_a_id);
        gs.player_b_id = ntohl(gs.player_b_id);

        // validate received length matches expected pawn_row size
        int expected = 12 + 1 + 1 + pawn_row_bytes(gs.max_pawn); 
        if (n != expected) {
            cout << "Unexpected GameState length: cannot translate to readable format: got " << n
                      << " expected " << expected << "\n";
            exit(0);
        }

        cout << game_state_to_string(gs) << "\n";
    }
    else {
        // wrong message status
        cout << "Unexpected message status: cannot translate to readable format: " << (int)status << "\n";
        exit(0);
    }
}

// ─── Main ─────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    ClientConfig cfg = parse_args(argc, argv);

    // ── Parse -m into raw bytes ──────────────────────────────────────
    auto fields = parse_message_fields(cfg.message);
    uint8_t sendbuf[64];
    ssize_t sendlen = build_message(fields, sendbuf);

    // ── Create UDP socket ────────────────────────────────────────────
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "socket: " << strerror(errno) << "\n";
        return 1;
    }

    // ── Resolve server address ───────────────────────────────────────
    sockaddr_in server_addr{};
    if (!parse_address(cfg.address, cfg.port, server_addr)) {
        cerr << "Could not resolve address: " << cfg.address << "\n";
        return 1;
    }

    // ── Set receive timeout ──────────────────────────────────────────
    struct timeval tv{ .tv_sec = cfg.timeout, .tv_usec = 0 };
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        cerr << "setsockopt: " << strerror(errno) << "\n";
        return 1;
    }

    // ── Send message ─────────────────────────────────────────────────
    ssize_t sent = sendto(sock, sendbuf, sendlen, 0,
                          (sockaddr*)&server_addr, sizeof(server_addr));
    if (sent < 0) {
        cerr << "sendto: " << strerror(errno) << "\n";
        return 1;
    }

    // ── Wait for response ────────────────────────────────────────────
    uint8_t recvbuf[1024];
    sockaddr_in from{};
    socklen_t fromlen = sizeof(from);

    ssize_t n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,
                         (sockaddr*)&from, &fromlen);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cout << "No response from server (timeout)\n";
            return 0;
        }
        cerr << "recvfrom: " << strerror(errno) << "\n";
        return 1;
    }

    // double-check response is from the server we sent to
    if (from.sin_addr.s_addr != server_addr.sin_addr.s_addr ||
        from.sin_port         != server_addr.sin_port) {
        cerr << "Response from unexpected address, ignoring\n";
        exit(1);
    }

    decode_message(recvbuf, n);

    return 0;
}