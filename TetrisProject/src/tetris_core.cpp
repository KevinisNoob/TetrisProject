#include "tetris_core.h"
#include <cstring>  // memset

namespace tetris {

    // 7 tetrominoes, 4 rotations, 4 blocks (x,y) relative to (0,0)
    // Using a common compact representation.
    static const int SHAPES[7][4][4][2] = {
        // I
        {
            {{0,1},{1,1},{2,1},{3,1}},
            {{2,0},{2,1},{2,2},{2,3}},
            {{0,2},{1,2},{2,2},{3,2}},
            {{1,0},{1,1},{1,2},{1,3}}
        },
        // O
        {
            {{1,0},{2,0},{1,1},{2,1}},
            {{1,0},{2,0},{1,1},{2,1}},
            {{1,0},{2,0},{1,1},{2,1}},
            {{1,0},{2,0},{1,1},{2,1}}
        },
        // T
        {
            {{1,0},{0,1},{1,1},{2,1}},
            {{1,0},{1,1},{2,1},{1,2}},
            {{0,1},{1,1},{2,1},{1,2}},
            {{1,0},{0,1},{1,1},{1,2}}
        },
        // S
        {
            {{1,0},{2,0},{0,1},{1,1}},
            {{1,0},{1,1},{2,1},{2,2}},
            {{1,1},{2,1},{0,2},{1,2}},
            {{0,0},{0,1},{1,1},{1,2}}
        },
        // Z
        {
            {{0,0},{1,0},{1,1},{2,1}},
            {{2,0},{1,1},{2,1},{1,2}},
            {{0,1},{1,1},{1,2},{2,2}},
            {{1,0},{0,1},{1,1},{0,2}}
        },
        // J
        {
            {{0,0},{0,1},{1,1},{2,1}},
            {{1,0},{2,0},{1,1},{1,2}},
            {{0,1},{1,1},{2,1},{2,2}},
            {{1,0},{1,1},{0,2},{1,2}}
        },
        // L
        {
            {{2,0},{0,1},{1,1},{2,1}},
            {{1,0},{1,1},{1,2},{2,2}},
            {{0,1},{1,1},{2,1},{0,2}},
            {{0,0},{1,0},{1,1},{1,2}}
        }
    };

    static uint32_t LcgNext(uint32_t& s) {
        // LCG parameters (Numerical Recipes)
        s = s * 1664525u + 1013904223u;
        return s;
    }

    static int RandType(Game* g) {
        return static_cast<int>(LcgNext(g->rng) % 7u);
    }

    static bool InBounds(int x, int y) {
        return (x >= 0 && x < BOARD_W && y >= 0 && y < BOARD_H);
    }

    static PieceBlocks GetBlocksAbs(int type, int rot, int ax, int ay) {
        PieceBlocks pb{};
        for (int i = 0; i < 4; ++i) {
            pb.x[i] = ax + SHAPES[type][rot][i][0];
            pb.y[i] = ay + SHAPES[type][rot][i][1];
        }
        return pb;
    }

    static bool Collides(const Game* g, int type, int rot, int ax, int ay) {
        PieceBlocks pb = GetBlocksAbs(type, rot, ax, ay);
        for (int i = 0; i < 4; ++i) {
            int x = pb.x[i];
            int y = pb.y[i];
            if (!InBounds(x, y)) return true;
            if (g->board[y][x] != 0) return true;
        }
        return false;
    }

    static void ClearBoard(Game* g) {
        std::memset(g->board, 0, sizeof(g->board));
    }

    static void LockPiece(Game* g) {
        PieceBlocks pb = GetBlocksAbs(g->curType, g->curRot, g->curX, g->curY);
        for (int i = 0; i < 4; ++i) {
            int x = pb.x[i], y = pb.y[i];
            if (InBounds(x, y)) g->board[y][x] = g->curType + 1; // store 1..7
        }
    }

    static int ClearLines(Game* g) {
        int cleared = 0;
        for (int y = BOARD_H - 1; y >= 0; --y) {
            bool full = true;
            for (int x = 0; x < BOARD_W; ++x) {
                if (g->board[y][x] == 0) { full = false; break; }
            }
            if (full) {
                // move all rows [0..y-1] down by 1
                for (int yy = y; yy > 0; --yy) {
                    for (int x = 0; x < BOARD_W; ++x) {
                        g->board[yy][x] = g->board[yy - 1][x];
                    }
                }
                // clear top row
                for (int x = 0; x < BOARD_W; ++x) g->board[0][x] = 0;

                ++cleared;
                ++y; // re-check same y after shifting
            }
        }
        return cleared;
    }

    static void AddScoreForLines(Game* g, int cleared) {
        // Simple scoring (classic-ish)
        // 1:100, 2:300, 3:500, 4:800
        static const int table[5] = { 0, 100, 300, 500, 800 };
        if (cleared >= 0 && cleared <= 4) g->score += table[cleared];
    }

    static void Spawn(Game* g) {
        g->curType = g->nextType;
        g->nextType = RandType(g);

        g->curRot = 0;
        g->curX = 3;
        g->curY = 0;

        if (Collides(g, g->curType, g->curRot, g->curX, g->curY)) {
            g->gameOver = true;
        }
    }

    // ---------- public ----------
    void Game_Init(Game* g, uint32_t seed) {
        if (!g) return;
        Game_Reset(g, seed);
    }

    void Game_Reset(Game* g, uint32_t seed) {
        if (!g) return;
        ClearBoard(g);
        g->score = 0;
        g->lines = 0;
        g->gameOver = false;

        if (seed != 0) g->rng = seed;
        else g->rng = 0xC0FFEEu; // deterministic default

        g->nextType = RandType(g);
        Spawn(g);
    }

    void Game_Tick(Game* g) {
        if (!g || g->gameOver) return;

        // try move down
        if (!Collides(g, g->curType, g->curRot, g->curX, g->curY + 1)) {
            g->curY += 1;
            return;
        }

        // lock & clear & spawn
        LockPiece(g);
        int cleared = ClearLines(g);
        if (cleared > 0) {
            g->lines += cleared;
            AddScoreForLines(g, cleared);
        }
        Spawn(g);
    }

    MoveResult Game_Move(Game* g, int dx, int dy) {
        if (!g || g->gameOver) return MoveResult::Blocked;
        int nx = g->curX + dx;
        int ny = g->curY + dy;
        if (Collides(g, g->curType, g->curRot, nx, ny)) return MoveResult::Blocked;
        g->curX = nx;
        g->curY = ny;
        return MoveResult::Ok;
    }

    MoveResult Game_RotateCW(Game* g) {
        if (!g || g->gameOver) return MoveResult::Blocked;

        int nr = (g->curRot + 1) & 3;

        // basic "wall kick" attempts
        const int kicks[] = { 0, -1, 1, -2, 2 };
        for (int k : kicks) {
            int nx = g->curX + k;
            int ny = g->curY;
            if (!Collides(g, g->curType, nr, nx, ny)) {
                g->curRot = nr;
                g->curX = nx;
                return MoveResult::Ok;
            }
        }
        return MoveResult::Blocked;
    }

    void Game_HardDrop(Game* g) {
        if (!g || g->gameOver) return;
        while (!Collides(g, g->curType, g->curRot, g->curX, g->curY + 1)) {
            g->curY += 1;
        }
        // lock immediately
        LockPiece(g);
        int cleared = ClearLines(g);
        if (cleared > 0) {
            g->lines += cleared;
            AddScoreForLines(g, cleared);
        }
        Spawn(g);
    }

    bool Game_IsGameOver(const Game* g) { return g ? g->gameOver : true; }
    int  Game_GetScore(const Game* g) { return g ? g->score : 0; }
    int  Game_GetLines(const Game* g) { return g ? g->lines : 0; }

    const int* Game_GetBoardPtr(const Game* g) {
        // row-major pointer to 200 ints
        return g ? &(g->board[0][0]) : nullptr;
    }

    PieceBlocks Game_GetCurrentPieceBlocks(const Game* g) {
        if (!g) return PieceBlocks{};
        return GetBlocksAbs(g->curType, g->curRot, g->curX, g->curY);
    }

    int Game_GetNextType(const Game* g) {
        return g ? g->nextType : 0;
    }


} // namespace tetris