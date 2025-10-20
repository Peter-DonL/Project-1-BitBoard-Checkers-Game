/* bitboard_checkers.c
 *
 * CS3503 - BitBoard Checkers Project
 *  - Enter moves as: <from> <to>  e.g. "b3 c4"
 *  - Alternate formats accepted: "b3-c4" or "b3xd4".
 *  - Multi-jumps: after a capturing jump, you will be prompted to continue the jump if possible.
 *
 *
 * Compile:
 *   gcc -std=c11 -O2 -o bitboard_checkers bitboard_checkers.c
 *
 * Run:
 *   ./bitboard_checkers
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
   Phase 1: Bit manipulation API

/* 32-bit variants */
unsigned int SetBit32(unsigned int value, int position) {
    if (position < 0 || position >= 32) return value;
    return value | (1u << position);
}
unsigned int ClearBit32(unsigned int value, int position) {
    if (position < 0 || position >= 32) return value;
    return value & ~(1u << position);
}
unsigned int ToggleBit32(unsigned int value, int position) {
    if (position < 0 || position >= 32) return value;
    return value ^ (1u << position);
}
int GetBit32(unsigned int value, int position) {
    if (position < 0 || position >= 32) return 0;
    return (value >> position) & 1u;
}
int CountBits32(unsigned int value) {
    /* Simple Kernighan's algorithm */
    int count = 0;
    while (value) { value &= (value - 1); count++; }
    return count;
}
unsigned int ShiftLeft32(unsigned int value, int positions) {
    if (positions < 0) return ShiftLeft32(value, 0);
    if (positions >= 32) return 0u;
    return value << positions;
}
unsigned int ShiftRight32(unsigned int value, int positions) {
    if (positions < 0) return ShiftRight32(value, 0);
    if (positions >= 32) return 0u;
    return value >> positions;
}
void PrintBinary32(unsigned int value) {
    for (int i = 31; i >= 0; --i) {
        putchar(((value >> i) & 1u) ? '1' : '0');
        if (i % 4 == 0 && i != 0) putchar(' ');
    }
    putchar('\n');
}
void PrintHex32(unsigned int value) {
    printf("0x%X\n", value);
}

/* 64-bit variants */
unsigned long long SetBit64(unsigned long long value, int position) {
    if (position < 0 || position >= 64) return value;
    return value | (1ULL << position);
}
unsigned long long ClearBit64(unsigned long long value, int position) {
    if (position < 0 || position >= 64) return value;
    return value & ~(1ULL << position);
}
unsigned long long ToggleBit64(unsigned long long value, int position) {
    if (position < 0 || position >= 64) return value;
    return value ^ (1ULL << position);
}
int GetBit64(unsigned long long value, int position) {
    if (position < 0 || position >= 64) return 0;
    return (int)((value >> position) & 1ULL);
}
int CountBits64(unsigned long long value) {
    int count = 0;
    while (value) { value &= (value - 1ULL); count++; }
    return count;
}
unsigned long long ShiftLeft64(unsigned long long value, int positions) {
    if (positions < 0) return ShiftLeft64(value, 0);
    if (positions >= 64) return 0ULL;
    return value << positions;
}
unsigned long long ShiftRight64(unsigned long long value, int positions) {
    if (positions < 0) return ShiftRight64(value, 0);
    if (positions >= 64) return 0ULL;
    return value >> positions;
}
void PrintBinary64(unsigned long long value) {
    for (int i = 63; i >= 0; --i) {
        putchar(((value >> i) & 1ULL) ? '1' : '0');
        if (i % 4 == 0 && i != 0) putchar(' ');
    }
    putchar('\n');
}
void PrintHex64(unsigned long long value) {
    printf("0x%llX\n", (unsigned long long)value);
}

/*

   Phase 2: Checkers game (bitboard)

*/

typedef struct {
    unsigned long long red_man;
    unsigned long long red_king;
    unsigned long long blk_man;
    unsigned long long blk_king;
    int turn;
} GameState;

/* Helpers for coordinate conversions */
int coord_to_index(const char *sq) {
    if (!sq || strlen(sq) < 2) return -1;
    char file = tolower(sq[0]);
    char rank_c = sq[1];
    if (file < 'a' || file > 'h') return -1;
    if (rank_c < '1' || rank_c > '8') return -1;
    int col = file - 'a';
    int row = rank_c - '1';
    return row * 8 + col;
}
void index_to_coord(int idx, char *out) {
    if (!out) return;
    if (idx < 0 || idx >= 64) { out[0] = '?'; out[1] = '\0'; return; }
    int row = idx / 8;
    int col = idx % 8;
    out[0] = 'a' + col;
    out[1] = '1' + row;
    out[2] = '\0';
}

unsigned long long all_red(GameState *g) {
    return g->red_man | g->red_king;
}
unsigned long long all_black(GameState *g) {
    return g->blk_man | g->blk_king;
}
unsigned long long all_pieces(GameState *g) {
    return all_red(g) | all_black(g);
}

int is_occupied(GameState *g, int idx) {
    if (idx < 0 || idx >= 64) return 0;
    return GetBit64(all_pieces(g), idx);
}
int is_red_piece(GameState *g, int idx) {
    return GetBit64(all_red(g), idx);
}
int is_black_piece(GameState *g, int idx) {
    return GetBit64(all_black(g), idx);
}
int is_king(GameState *g, int idx) {
    return GetBit64(g->red_king | g->blk_king, idx);
}

void init_game(GameState *g) {
    g->red_man = 0ULL;
    g->red_king = 0ULL;
    g->blk_man = 0ULL;
    g->blk_king = 0ULL;
    g->turn = 0;

    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 8; ++c) {
            if ((r + c) % 2 == 1) {
                int idx = r * 8 + c;
                g->red_man = SetBit64(g->red_man, idx);
            }
        }
    }
    for (int r = 5; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if ((r + c) % 2 == 1) {
                int idx = r * 8 + c;
                g->blk_man = SetBit64(g->blk_man, idx);
            }
        }
    }
}


void print_board(GameState *g) {
    printf("\n    a b c d e f g h\n   -----------------\n");
    for (int r = 0; r < 8; ++r) {
        printf("%d | ", r + 1);
        for (int c = 0; c < 8; ++c) {
            int idx = r * 8 + c;
            char ch;
            if (is_red_piece(g, idx)) {
                if (GetBit64(g->red_king, idx)) ch = 'R';
                else ch = 'r';
            } else if (is_black_piece(g, idx)) {
                if (GetBit64(g->blk_king, idx)) ch = 'B';
                else ch = 'b';
            } else {
                if ((r + c) % 2 == 1) ch = '#'; else ch = '.';
            }
            printf("%c ", ch);
        }
        printf("\n");
    }
    printf("   -----------------\n");
    printf("    a b c d e f g h\n\n");
}

int on_board(int r, int c) {
    return (r >= 0 && r < 8 && c >= 0 && c < 8);
}


int is_playable_square(int r, int c) {
    return ((r + c) % 2 == 1);
}

int player_has_capture(GameState *g, int player /*0=red,1=black*/) {
    unsigned long long men = (player == 0) ? g->red_man : g->blk_man;
    unsigned long long kings = (player == 0) ? g->red_king : g->blk_king;
    unsigned long long opponent = (player == 0) ? all_black(g) : all_red(g);
    unsigned long long occupied = all_pieces(g);

    for (int idx = 0; idx < 64; ++idx) {
        if (GetBit64(men | kings, idx) == 0) continue;
        int r = idx / 8, c = idx % 8;
        int directions[4][2] = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
        for (int d = 0; d < 4; ++d) {
            int dr = directions[d][0], dc = directions[d][1];
            if (GetBit64(men, idx)) {
                if (player == 0 && dr < 0) continue;
                if (player == 1 && dr > 0) continue;
            }
            int midr = r + dr, midc = c + dc;
            int endr = r + 2*dr, endc = c + 2*dc;
            if (!on_board(midr, midc) || !on_board(endr, endc)) continue;
            if (!is_playable_square(midr, midc) || !is_playable_square(endr, endc)) continue;
            int midi = midr*8 + midc, endi = endr*8 + endc;
            if (GetBit64(opponent, midi) && !GetBit64(occupied, endi)) {
                return 1;
            }
        }
    }
    return 0;
}

int validate_simple_move(GameState *g, int player, int from_idx, int to_idx, int *is_capture) {
    *is_capture = 0;
    if (from_idx < 0 || to_idx < 0 || from_idx >=64 || to_idx>=64) return 0;
    if (from_idx == to_idx) return 0;
    if (is_occupied(g, to_idx)) return 0;
    unsigned long long men = (player==0)? g->red_man : g->blk_man;
    unsigned long long kings = (player==0)? g->red_king : g->blk_king;
    if (!GetBit64(men | kings, from_idx)) return 0; /* no player's piece at from */
    int fr = from_idx/8, fc = from_idx%8, tr = to_idx/8, tc = to_idx%8;
    if (!is_playable_square(tr,tc) || !is_playable_square(fr,fc)) return 0;
    int dr = tr - fr, dc = tc - fc;
    if (abs(dr) == 1 && abs(dc) == 1) {
        if (GetBit64(men, from_idx)) {
            if (player == 0 && dr != 1) return 0; /* red moves down (+1 row) */
            if (player == 1 && dr != -1) return 0; /* black moves up (-1 row) */
        }
        return 1;
    }
    if (abs(dr) == 2 && abs(dc) == 2) {
        int midr = fr + dr/2, midc = fc + dc/2;
        int mid = midr*8 + midc;
        unsigned long long opponent = (player==0)? all_black(g) : all_red(g);
        if (GetBit64(opponent, mid)) {
            if (GetBit64(men, from_idx)) {
                if (player == 0 && dr != 2) return 0;
                if (player == 1 && dr != -2) return 0;
            }
            *is_capture = 1;
            return 1;
        }
    }
    return 0;
}

int execute_move(GameState *g, int player, int from_idx, int to_idx) {
    if (from_idx < 0 || to_idx < 0 || from_idx >=64 || to_idx>=64) return -1;
    unsigned long long *my_man = (player==0) ? &g->red_man : &g->blk_man;
    unsigned long long *my_king = (player==0) ? &g->red_king : &g->blk_king;
    unsigned long long *opp_man = (player==0) ? &g->blk_man : &g->red_man;
    unsigned long long *opp_king = (player==0) ? &g->blk_king : &g->red_king;
    unsigned long long occupied = all_pieces(g);

    int is_cap = 0;
    if (!validate_simple_move(g, player, from_idx, to_idx, &is_cap)) return -1;

    int was_king = GetBit64(*my_king, from_idx);
    if (GetBit64(*my_man, from_idx)) {
        *my_man = ClearBit64(*my_man, from_idx);
    } else if (was_king) {
        *my_king = ClearBit64(*my_king, from_idx);
    } else {
        return -1;
    }

    if (is_cap) {
        int fr = from_idx / 8, fc = from_idx % 8, tr = to_idx/8, tc = to_idx%8;
        int midr = (fr + tr) / 2, midc = (fc + tc) / 2;
        int mid = midr*8 + midc;
        if (GetBit64(*opp_man, mid)) *opp_man = ClearBit64(*opp_man, mid);
        else if (GetBit64(*opp_king, mid)) *opp_king = ClearBit64(*opp_king, mid);
        else {
        }
    }


    int tr = to_idx / 8;
    int promote = 0;
    if (!was_king) {
        if (player == 0 && tr == 7) promote = 1;
        if (player == 1 && tr == 0) promote = 1;
    }
    if (was_king || promote) {
        *my_king = SetBit64(*my_king, to_idx);
    } else {
        *my_man = SetBit64(*my_man, to_idx);
    }

    if (is_cap) {
        unsigned long long opponent = all_pieces(g) ^ (all_red(g) | all_black(g)); /* placeholder; we'll compute */
        int r = to_idx / 8, c = to_idx % 8;
        unsigned long long mypieces = (player==0) ? g->red_man | g->red_king : g->blk_man | g->blk_king;
        unsigned long long opponentBB = (player==0) ? all_black(g) : all_red(g);
        int directions[4][2] = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
        for (int d = 0; d < 4; ++d) {
            int dr = directions[d][0], dc = directions[d][1];
            if (!GetBit64(*my_king, to_idx)) {
                if (player == 0 && dr < 0) continue;
                if (player == 1 && dr > 0) continue;
            }
            int midr = r + dr, midc = c + dc;
            int endr = r + 2*dr, endc = c + 2*dc;
            if (!on_board(midr, midc) || !on_board(endr, endc)) continue;
            if (!is_playable_square(midr, midc) || !is_playable_square(endr, endc)) continue;
            int mid = midr*8 + midc, end = endr*8 + endc;
            if (GetBit64(opponentBB, mid) && !GetBit64(all_pieces(g), end)) {
                return 2;
            }
        }
    }

    return 1;
}

int player_has_any_move(GameState *g, int player) {
    unsigned long long men = (player==0)? g->red_man : g->blk_man;
    unsigned long long kings = (player==0)? g->red_king : g->blk_king;
    unsigned long long occupied = all_pieces(g);
    unsigned long long opponent = (player==0)? all_black(g) : all_red(g);

    for (int idx = 0; idx < 64; ++idx) {
        if (!GetBit64(men | kings, idx)) continue;
        int r = idx / 8, c = idx % 8;
        int directions[4][2] = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
        for (int d = 0; d < 4; ++d) {
            int dr = directions[d][0], dc = directions[d][1];
            if (GetBit64(men, idx)) {
                if (player == 0 && dr < 0) continue;
                if (player == 1 && dr > 0) continue;
            }
            int tr = r + dr, tc = c + dc;
            if (on_board(tr, tc) && is_playable_square(tr,tc)) {
                int tidx = tr*8 + tc;
                if (!GetBit64(occupied, tidx)) return 1; /* simple move exists */
            }
            int midr = r + dr, midc = c + dc;
            int endr = r + 2*dr, endc = c + 2*dc;
            if (on_board(midr, midc) && on_board(endr, endc) && is_playable_square(midr,midc) && is_playable_square(endr,endc)) {
                int mid = midr*8 + midc, endi = endr*8 + endc;
                if (GetBit64(opponent, mid) && !GetBit64(occupied, endi)) return 1;
            }
        }
    }
    return 0;
}

int count_red(GameState *g) { return CountBits64(g->red_man) + CountBits64(g->red_king); }
int count_black(GameState *g) { return CountBits64(g->blk_man) + CountBits64(g->blk_king); }


int parse_move_input(const char *line, int *from_idx, int *to_idx) {
    if (!line || !from_idx || !to_idx) return 0;
    char copy[256];
    strncpy(copy, line, sizeof(copy)-1);
    copy[sizeof(copy)-1] = '\0';
    for (char *p = copy; *p; ++p) {
        if (*p == '-' || *p == 'x' || *p == 'X') *p = ' ';
    }
    char *tok = strtok(copy, " \t\r\n");
    if (!tok) return 0;
    char sfrom[8], sto[8];
    strncpy(sfrom, tok, sizeof(sfrom)-1);
    sfrom[sizeof(sfrom)-1] = '\0';
    tok = strtok(NULL, " \t\r\n");
    if (!tok) return 0;
    strncpy(sto, tok, sizeof(sto)-1);
    sto[sizeof(sto)-1] = '\0';
    int fi = coord_to_index(sfrom);
    int ti = coord_to_index(sto);
    if (fi < 0 || ti < 0) return 0;
    *from_idx = fi; *to_idx = ti;
    return 1;
}

void play_game() {
    GameState g;
    init_game(&g);
    printf("Welcome to BitBoard Checkers (text-mode)!\n");
    printf("Coordinate system: a-h left->right, 1-8 top->bottom. Example: b3 c4\n");
    printf("Red pieces (r) start at top (rows 1..3) and move DOWN; Black (b) start bottom and move UP.\n");
    printf("Kings are R/B. Play alternates turns. Captures are mandatory.\n\n");

    while (1) {
        print_board(&g);
        int red_count = count_red(&g), black_count = count_black(&g);
        if (red_count == 0) { printf("Black wins! (red has no pieces)\n"); break; }
        if (black_count == 0) { printf("Red wins! (black has no pieces)\n"); break; }
        int player = g.turn;
        if (!player_has_any_move(&g, player)) {
            if (player == 0) printf("Red has no legal moves. Black wins!\n");
            else printf("Black has no legal moves. Red wins!\n");
            break;
        }
        printf("%s's turn. Enter move (e.g. b3 c4): ", (player==0) ? "Red" : "Black");
        char line[256];
        if (!fgets(line, sizeof(line), stdin)) {
            printf("Input error or EOF; quitting.\n");
            break;
        }
        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0') continue;
        if (tolower(p[0]) == 'q') { printf("Quitting.\n"); break; }
        int from_idx = -1, to_idx = -1;
        if (!parse_move_input(p, &from_idx, &to_idx)) {
            printf("Couldn't parse move. Use e.g. b3 c4 or b3-c4. Try again.\n");
            continue;
        }
        /* Check occupation and ownership */
        if (!GetBit64((player==0? all_red(&g) : all_black(&g)), from_idx)) {
            printf("No your piece at source square. Try again.\n");
            continue;
        }
        int is_cap = 0;
        if (!validate_simple_move(&g, player, from_idx, to_idx, &is_cap)) {
            printf("Illegal move according to rules. Try again.\n");
            continue;
        }
        if (player_has_capture(&g, player)) {
            if (!is_cap) {
                printf("A capture is available somewhere; you must capture. Try again.\n");
                continue;
            }
        }
        int exec_res = execute_move(&g, player, from_idx, to_idx);
        if (exec_res == -1) {
            printf("Error executing move. Try again.\n");
            continue;
        } else if (exec_res == 2) {
            char coord[8], nextline[256];
            index_to_coord(to_idx, coord);
            printf("Capture performed. Another capture is available from %s. You must continue.\n", coord);
            int curr_from = to_idx;
            while (1) {
                print_board(&g);
                printf("Continue jump from %s to (enter destination): ", coord);
                if (!fgets(nextline, sizeof(nextline), stdin)) { printf("Input error; quitting.\n"); return; }
                if (!parse_move_input(nextline, &from_idx, &to_idx)) {
                    char trimmed[32]; int k=0;
                    for (char *q = nextline; *q && isspace((unsigned char)*q); q++);
                    char *q = nextline;
                    while (*q && isspace((unsigned char)*q)) q++;
                    while (*q && !isspace((unsigned char)*q) && k < 30) trimmed[k++] = *q++;
                    trimmed[k] = '\0';
                    if (k >= 2) {
                        int to_try = coord_to_index(trimmed);
                        if (to_try >= 0) {
                            from_idx = curr_from;
                            to_idx = to_try;
                        } else {
                            printf("Couldn't parse destination. Try again.\n");
                            continue;
                        }
                    } else {
                        printf("Couldn't parse input. Try again.\n");
                        continue;
                    }
                } else {
                    if (from_idx != curr_from) {
                        printf("You must continue jumping with the same piece from %s. Try again.\n", coord);
                        continue;
                    }
                }
                int is_cap2 = 0;
                if (!validate_simple_move(&g, player, from_idx, to_idx, &is_cap2) || !is_cap2) {
                    printf("That is not a legal capturing jump. Try again.\n");
                    continue;
                }
                int er = execute_move(&g, player, from_idx, to_idx);
                if (er == -1) { printf("Error executing jump. Try again.\n"); continue; }
                curr_from = to_idx;
                index_to_coord(curr_from, coord);
                if (er == 2) {
                } else {
                    break;
                }
            }
            g.turn = 1 - g.turn;
        } else {
            g.turn = 1 - g.turn;
        }
    }
}

void run_phase1_tests() {
    printf("=== Phase 1 tests (bit manipulation) ===\n");
    unsigned int v32 = 0u;
    v32 = SetBit32(v32, 3);
    printf("Set bit 3 on 32-bit value: ");
    PrintBinary32(v32);
    printf("Count bits: %d (expected 1)\n", CountBits32(v32));

    v32 = ToggleBit32(v32, 3);
    printf("Toggle bit 3: ");
    PrintBinary32(v32);
    printf("Count bits: %d (expected 0)\n", CountBits32(v32));

    v32 = SetBit32(v32, 31);
    printf("Set bit 31: ");
    PrintBinary32(v32);

    printf("ShiftLeft32(1,5): ");
    PrintBinary32(ShiftLeft32(1u,5));

    printf("PrintHex32(0xDEADBEEF): ");
    PrintHex32(0xDEADBEEF);

    unsigned long long v64 = 0ULL;
    v64 = SetBit64(v64, 0);
    v64 = SetBit64(v64, 63);
    printf("64-bit binary with bits 0 and 63 set:\n");
    PrintBinary64(v64);
    printf("CountBits64: %d (expected 2)\n", CountBits64(v64));
    printf("PrintHex64: ");
    PrintHex64(0x12345678ABCDEFULL);
    printf("=== End Phase 1 tests ===\n\n");
}

int main(int argc, char **argv) {
    printf("BitBoard Checkers - Full Project Implementation\n");
    printf("Running Phase 1 tests...\n");
    run_phase1_tests();

    printf("Starting Phase 2: Playable Checkers game.\n");
    play_game();

    printf("Thanks for playing. Goodbye!\n");
    return 0;
}
