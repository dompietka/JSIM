#include "common.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>

using namespace std;

// ─── Argument parsing ─────────────────────────────────────────────────

struct ServerConfig {
    string pawn_row;   // from -r
    uint8_t bitmask_pawn_row[32];
    int pawn_row_len; // derived from pawn_row
    int max_pawn;     // derived from pawn_row_len
    string address;    // from -a
    uint16_t    port;       // from -p
    int         timeout;    // from -t
};

static ServerConfig parse_args(int argc, char* argv[]) {
    ServerConfig cfg{};
    bool has_r = false, has_a = false, has_p = false, has_t = false;
    if (argc < 9) {
        cerr << "Usage: " << argv[0] << " -r <pawn_row> -a <address> -p <port> -t <timeout>\n";
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "-r" && i + 1 < argc) {
            if (has_r) {cerr << "Duplicate -r: using first value\n"; i++; continue; }
            cfg.pawn_row = argv[++i];
            cfg.pawn_row_len = cfg.pawn_row.size();
            has_r = true;
        } else if (string(argv[i]) == "-a" && i + 1 < argc) {
            if (has_a) { cerr << "Duplicate -a: using first value\n"; i++; continue; }
            cfg.address = argv[++i];
            has_a = true;
        } else if (string(argv[i]) == "-p" && i + 1 < argc) {
            if (has_p) { cerr << "Duplicate -p: using first value\n"; i++; continue; }
            char* end;
            long v = strtol(argv[++i], &end, 10);
            if (*end != '\0' || v < 0 || v > 65535) {
                cerr << "Invalid port\n"; exit(1);
            }
            cfg.port = (uint16_t)v;
            has_p = true;
        } else if (string(argv[i]) == "-t" && i + 1 < argc) {
            if (has_t) { cerr << "Duplicate -t: using first value\n"; i++; continue; }
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

    if (!has_r || !has_a || !has_p || !has_t) {
        cerr << "Missing required arguments (-r -a -p -t)\n";
        exit(1);
    }

    // Convert pawn_row string to bitmask format for easier processing later
    for (int i = 0; i < (int)cfg.pawn_row.size(); i++) {
        if (cfg.pawn_row[i] == '1')
            pawn_set(cfg.bitmask_pawn_row, i);
    }

    cfg.max_pawn = cfg.pawn_row_len - 1;
    return cfg;
}

// ─── Game structures ─────────────────────────────────────────────────

struct Game {
    uint32_t game_id;
    uint32_t player_a_id;
    uint32_t player_b_id;       // 0 = no player B yet
    uint8_t  status;
    uint8_t  max_pawn;
    uint8_t  pawn_row[32];      // only first pawn_row_bytes(max_pawn) are meaningful

    timespec   last_active_a;     // last valid msg from player A for this game
    timespec   last_active_b;     // last valid msg from player B for this game
    timespec   finished_at;       // when status became WIN_A/WIN_B, 0 if ongoing
};

static vector<Game> games;
static int waiting_game_idx = -1;  // index into games of the one WAITING_FOR_OPPONENT game, -1 if none
static uint32_t next_game_id = 0;  // incrementing counter for unique game_ids

// Returns pointer to game, or nullptr if not found
static Game* find_game(uint32_t game_id) {
    for (auto& g : games)
        if (g.game_id == game_id)
            return &g;
    return nullptr;
}

// Returns true if game exists AND player is participant
static bool game_exists_with_player(uint32_t game_id, uint32_t player_id) {
    Game* g = find_game(game_id);
    if (!g) return false;
    return g->player_a_id == player_id || g->player_b_id == player_id;
}

static Game make_game(uint32_t player_a_id, const ServerConfig& cfg) {
    Game g{};
    g.game_id      = next_game_id++;
    g.player_a_id  = player_a_id;
    g.player_b_id  = 0;
    g.status       = STATUS_WAITING;
    g.max_pawn     = cfg.max_pawn;
    memcpy(g.pawn_row, cfg.bitmask_pawn_row, sizeof(g.pawn_row));
    g.last_active_a = now_ts();
    g.last_active_b = {0, 0};
    g.finished_at   = {0, 0};
    return g;
}

// ─── Pawn row validation ──────────────────────────────────────────────

static void validate_pawn_row(const string& row) {
    int len = row.size();
    if (len < 1 || len > 256) {
        cerr << "pawn_row length must be 1-256\n"; exit(1);
    }
    for (char c : row) {
        if (c != '0' && c != '1') {
            cerr << "pawn_row must contain only 0 and 1\n"; exit(1);
        }
    }
    if (row.front() != '1' || row.back() != '1') {
        cerr << "pawn_row must start and end with 1\n"; exit(1);
    }
}

// ─── Message sending ──────────────────────────────────────────────

static void send_game_state(int sock, const sockaddr_in& to, const Game& g) {
    GameState gs{};
    gs.game_id     = htonl(g.game_id);
    gs.player_a_id = htonl(g.player_a_id);
    gs.player_b_id = htonl(g.player_b_id);
    gs.status      = g.status;
    gs.max_pawn    = g.max_pawn;
    memcpy(gs.pawn_row, g.pawn_row, pawn_row_bytes(g.max_pawn));

    int size = 14 + pawn_row_bytes(g.max_pawn);
    sendto(sock, &gs, size, 0, (sockaddr*)&to, sizeof(to));
}

static void send_wrong_msg(int sock, const sockaddr_in& to,
                           const uint8_t* buf, ssize_t n, uint8_t error_index) {
    MsgWrongMsg wm{};
    memcpy(wm.echo, buf, min((ssize_t)12, n));
    wm.status      = STATUS_WRONG_MSG;
    wm.error_index = error_index;
    sendto(sock, &wm, sizeof(wm), 0, (sockaddr*)&to, sizeof(to));
}

// ─── Message verification ──────────────────────────────────────────────

bool verify_message(const uint8_t* buf, ssize_t n,
                            ParsedMsg& out, uint8_t& out_error_index) {

    // ── byte 0: msg_type ─────────────────────────────────────────────
    if (n < 1) {
        out_error_index = 0;
        return false;
    }
    uint8_t msg_type = buf[0];
    if (msg_type > MSG_GIVE_UP) {
        out_error_index = 0;
        return false;
    }

    // ── check correct total length for this msg_type ─────────────────
    // should be correct because of client verification, but check anyway
    ssize_t expected_size;
    switch (msg_type) {
        case MSG_JOIN:       expected_size = MSG_JOIN_SIZE;       break;
        case MSG_MOVE_1:     expected_size = MSG_MOVE_1_SIZE;     break;
        case MSG_MOVE_2:     expected_size = MSG_MOVE_2_SIZE;     break;
        case MSG_KEEP_ALIVE: expected_size = MSG_KEEP_ALIVE_SIZE; break;
        case MSG_GIVE_UP:    expected_size = MSG_GIVE_UP_SIZE;    break;
        default:             out_error_index = 0; return false;
    }

    if (n != expected_size) {
        // point to first byte we couldn't interpret — either too short or too long
        out_error_index = (uint8_t)min((ssize_t)255, min(n, expected_size));
        return false;
    }

    // ── bytes 1-4: player_id ─────────────────────────────────────────
    uint32_t player_id;
    memcpy(&player_id, buf + 1, 4);
    player_id = ntohl(player_id);

    if (player_id == 0) {
        out_error_index = 1;
        return false;
    }

    // ── MSG_JOIN ends here ───────────────────────────────────────────
    if (msg_type == MSG_JOIN) {
        out.msg_type  = msg_type;
        out.player_id = player_id;
        out.game_id   = 0;
        out.pawn      = 0;
        return true;
    }

    // ── bytes 5-8: game_id ───────────────────────────────────────────
    uint32_t game_id;
    memcpy(&game_id, buf + 5, 4);
    game_id = ntohl(game_id);

    // game must exist and player must be a participant
    if (!game_exists_with_player(game_id, player_id)) {
        out_error_index = 5;  // first byte of game_id field
        return false;
    }

    // ── MSG_KEEP_ALIVE and MSG_GIVE_UP end here ──────────────────────
    if (msg_type == MSG_KEEP_ALIVE || msg_type == MSG_GIVE_UP) {
        out.msg_type  = msg_type;
        out.player_id = player_id;
        out.game_id   = game_id;
        out.pawn      = 0;
        return true;
    }

    // ── byte 9: pawn ─────────────────────────────────────────────────
    // Note: invalid pawn value makes the message valid but the move illegal
    uint8_t pawn = buf[9];

    out.msg_type  = msg_type;
    out.player_id = player_id;
    out.game_id   = game_id;
    out.pawn      = pawn;
    return true;
}

// ─── Message handling ──────────────────────────────────────────────

static void handle_message(int sock, const sockaddr_in& client_addr,
                            const ParsedMsg& msg, const ServerConfig& cfg, timespec& now) {
    // ── MSG_JOIN ─────────────────────────────────────────────────────
    if (msg.msg_type == MSG_JOIN) {
        if (waiting_game_idx == -1) {
            // no waiting game — create one
            Game g = make_game(msg.player_id, cfg);
            try {
                games.push_back(g);
                waiting_game_idx = (int)games.size() - 1;
                send_game_state(sock, client_addr, games[waiting_game_idx]);
            } catch (const std::bad_alloc&) {
                // ignore MSG_JOIN if can't create game
            }
            
        } else {
            // join existing waiting game as player B
            Game& g = games[waiting_game_idx];

            g.player_b_id   = msg.player_id;
            g.status        = STATUS_TURN_B;
            g.last_active_b = now;
            waiting_game_idx = -1;
            send_game_state(sock, client_addr, g);
        }
        return;
    }

    // ── all other messages need a valid game ──────────────────────────
    Game* g = find_game(msg.game_id);
    if (!g) return;  // shouldn't happen — verify_message already checked

    // update last active timestamp
    if (msg.player_id == g->player_a_id) g->last_active_a = now;
    if (msg.player_id == g->player_b_id) g->last_active_b = now;

    // ── MSG_KEEP_ALIVE ───────────────────────────────────────────────
    if (msg.msg_type == MSG_KEEP_ALIVE) {
        send_game_state(sock, client_addr, *g);
        return;
    }

    // only the player whose turn it is can do one of the following
    bool is_my_turn = ((g->status == STATUS_TURN_A && msg.player_id == g->player_a_id) || (g->status == STATUS_TURN_B && msg.player_id == g->player_b_id));

    if (!is_my_turn) {
        // illegal — not your turn, state unchanged
        send_game_state(sock, client_addr, *g);
        return;    
    }

    // ── MSG_GIVE_UP ──────────────────────────────────────────────────
    if (msg.msg_type == MSG_GIVE_UP) {
        g->status      = g->status == STATUS_TURN_A ? STATUS_WIN_B : STATUS_WIN_A;
        g->finished_at = now;
        send_game_state(sock, client_addr, *g);
        return;
    }


    // ── MSG_MOVE_1 and MSG_MOVE_2 ───────────────────────────────────

    uint8_t pawn = msg.pawn;

    if (msg.msg_type == MSG_MOVE_1) {
        // pawn must exist
        if (pawn > g->max_pawn || !pawn_is_set(g->pawn_row, pawn)) {
            send_game_state(sock, client_addr, *g);  // illegal, unchanged
            return;
        }
        pawn_clear(g->pawn_row, pawn);

    } else {  // MSG_MOVE_2
        // pawn and pawn+1 must both exist and be adjacent
        if (pawn > g->max_pawn || pawn + 1 > g->max_pawn ||
            !pawn_is_set(g->pawn_row, pawn) ||
            !pawn_is_set(g->pawn_row, pawn + 1)) {
            send_game_state(sock, client_addr, *g);  // illegal, unchanged
            return;
        }
        pawn_clear(g->pawn_row, pawn);
        pawn_clear(g->pawn_row, pawn + 1);
    }

    // check if any pawns remain
    bool any_left = false;
    for (int i = 0; i <= g->max_pawn; i++) {
        if (pawn_is_set(g->pawn_row, i)) { any_left = true; break; }
    }

    if (!any_left) {
        // whoever just moved wins
        g->status      = g->status == STATUS_TURN_A ? STATUS_WIN_A : STATUS_WIN_B;
        g->finished_at = now;
    } else {
        // swap turns
        g->status = g->status == STATUS_TURN_A ? STATUS_TURN_B : STATUS_TURN_A;
    }

    send_game_state(sock, client_addr, *g);
}

// ─── Timeout handling ──────────────────────────────────────────────
void check_all_game_timeouts(int server_timeout, timespec& now) {

    for (int i = 0; i < (int)games.size(); i++) {
        Game& g = games[i];

        // finished games — keep for server_timeout seconds then delete
        if (g.status == STATUS_WIN_A || g.status == STATUS_WIN_B) {
            if (g.finished_at.tv_sec != 0 &&
                elapsed(g.finished_at, now) >= server_timeout &&
                elapsed(g.last_active_a, now) >= server_timeout &&
                elapsed(g.last_active_b, now) >= server_timeout) {
                if (waiting_game_idx == i)        waiting_game_idx = -1;
                else if (waiting_game_idx > i)    waiting_game_idx--;
                games.erase(games.begin() + i);
                i--;
            }
            continue;
        }

        // waiting game — only player A exists
        if (g.status == STATUS_WAITING) {
            if (elapsed(g.last_active_a, now) >= server_timeout) {
                waiting_game_idx = -1;
                games.erase(games.begin() + i);
                i--;
            }
            continue;
        }

        // active game — check both players
        if (elapsed(g.last_active_a, now) >= server_timeout && elapsed(g.last_active_b, now) >= server_timeout) {
            if (elapsed(g.last_active_a, now) >= elapsed(g.last_active_b, now) ) {
                 g.status      = STATUS_WIN_B;
            }
            else {
                 g.status      = STATUS_WIN_A;
            }
            g.finished_at = now;
           
        } else if (elapsed(g.last_active_b, now) >= server_timeout) {
            g.status      = STATUS_WIN_A;
            g.finished_at = now;
        } else if (elapsed(g.last_active_a, now) >= server_timeout) {
            g.status      = STATUS_WIN_B;
            g.finished_at = now;
        }

    }
}




// ─── Main ─────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    ServerConfig cfg = parse_args(argc, argv);
    validate_pawn_row(cfg.pawn_row);

    // ── Create UDP socket ────────────────────────────────────────────
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "socket: " << strerror(errno) << "\n";
        return 1;
    }

    // ── Bind to address and port ─────────────────────────────────────
    sockaddr_in server_addr{};
    if (!parse_address(cfg.address, cfg.port, server_addr)) {
        cerr << "Invalid address: " << cfg.address << "\n";
        return 1;
    }

    if (bind(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "bind: " << strerror(errno) << "\n";
        return 1;
    }

    // ── Main loop ────────────────────────────────────────────────────
    uint8_t buf[1024];
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    while (true) {
        ssize_t n = recvfrom(sock, buf, sizeof(buf), 0,
                             (sockaddr*)&client_addr, &client_len);

        timespec now = now_ts();
        //timeout check before processing message to ensure that 
        //there is no scenario where a game that should be timed out 
        //gets a message processed and becomes active again
        check_all_game_timeouts(cfg.timeout, now);
        ParsedMsg msg{};
        uint8_t error_index = 0;
        if (!verify_message(buf, n, msg, error_index)) {
            send_wrong_msg(sock, client_addr, buf, n, error_index);
            continue;
        }
        handle_message(sock, client_addr, msg, cfg, now);
    }
}