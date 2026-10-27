#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <time.h>


using namespace std;


// ─── Time management ─────────────────────────────────────────────────

timespec now_ts();

// returns elapsed seconds between two timespecs as a double
double elapsed(const timespec& from, const timespec& to);

// ─── Protocol constants ───────────────────────────────────────────────

// Client message types
constexpr uint8_t MSG_JOIN       = 0;
constexpr uint8_t MSG_MOVE_1     = 1;
constexpr uint8_t MSG_MOVE_2     = 2;
constexpr uint8_t MSG_KEEP_ALIVE = 3;
constexpr uint8_t MSG_GIVE_UP    = 4;

// Game status values
constexpr uint8_t STATUS_WAITING = 0;
constexpr uint8_t STATUS_TURN_A  = 1;
constexpr uint8_t STATUS_TURN_B  = 2;
constexpr uint8_t STATUS_WIN_A   = 3;
constexpr uint8_t STATUS_WIN_B   = 4;

constexpr uint8_t STATUS_WRONG_MSG = 255;

constexpr int MAX_PAWN_COUNT = 256;  // max pawn_row string length from -r
constexpr int PAWN_ROW_BYTES = 32;   // ceil(256/8)


// Message sizes 
static constexpr ssize_t MSG_JOIN_SIZE       = 5;  // 1 + 4
static constexpr ssize_t MSG_MOVE_1_SIZE     = 10; // 1 + 4 + 4 + 1
static constexpr ssize_t MSG_MOVE_2_SIZE     = 10; // 1 + 4 + 4 + 1
static constexpr ssize_t MSG_KEEP_ALIVE_SIZE = 9;  // 1 + 4 + 4
static constexpr ssize_t MSG_GIVE_UP_SIZE    = 9;  // 1 + 4 + 4



// ─── Wire format structs ──────────────────────────────────────────────
// All multibyte fields are in NETWORK byte order when on the wire.
// Use the helper functions below to read/write them safely.

// Game state sent by server → client
struct __attribute__((packed)) GameState {
    uint32_t game_id;
    uint32_t player_a_id;
    uint32_t player_b_id;
    uint8_t  status;
    uint8_t  max_pawn;
    uint8_t  pawn_row[PAWN_ROW_BYTES];  // only first floor(max_pawn/8)+1 bytes are meaningful
};

// Client → server messages

struct __attribute__((packed)) MsgJoin {
    uint8_t  msg_type;   // 0
    uint32_t player_id;
};

struct __attribute__((packed)) MsgMove1 {
    uint8_t  msg_type;   // 1
    uint32_t player_id;
    uint32_t game_id;
    uint8_t  pawn;
};

struct __attribute__((packed)) MsgMove2 {
    uint8_t  msg_type;   // 2
    uint32_t player_id;
    uint32_t game_id;
    uint8_t  pawn;
};

struct __attribute__((packed)) MsgKeepAlive {
    uint8_t  msg_type;   // 3
    uint32_t player_id;
    uint32_t game_id;
};

struct __attribute__((packed)) MsgGiveUp {
    uint8_t  msg_type;   // 4
    uint32_t player_id;
    uint32_t game_id;
};

// Server → client error message
struct __attribute__((packed)) MsgWrongMsg {
    uint8_t  echo[12];       // first 12 bytes of bad client message, zero-padded
    uint8_t  status;         // 255
    uint8_t  error_index;    // index of first byte server couldn't interpret
};

struct ParsedMsg {
    uint8_t  msg_type;
    uint32_t player_id;
    uint32_t game_id;   // 0 for MSG_JOIN
    uint8_t  pawn;      // 0 for non-move messages
};

// ─── Pawn row helpers ─────────────────────────────────────────────────

// Returns true if pawn N is set in pawn_row
bool pawn_is_set(const uint8_t* pawn_row, uint8_t n);

// Sets pawn N in pawn_row
void pawn_set(uint8_t* pawn_row, uint8_t n);

// Clears pawn N in pawn_row
void pawn_clear(uint8_t* pawn_row, uint8_t n);

// Returns number of bytes needed for pawn_row given max_pawn
int pawn_row_bytes(uint8_t max_pawn);

// ─── Network helpers ──────────────────────────────────────────────────

// Parse address + port into sockaddr_in, returns false on failure
bool parse_address(const std::string& addr, uint16_t port, sockaddr_in& out);



