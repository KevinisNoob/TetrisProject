#pragma once
#include <cstdint>

namespace tetris {

    static constexpr int BOARD_W = 10;
    static constexpr int BOARD_H = 20;

    enum class MoveResult : int {
        Ok = 0,
        Blocked = 1
    };

    struct PieceBlocks {
        // 4 blocks in absolute board coordinates
        int x[4];
        int y[4];
    };

    struct Game {
        int board[BOARD_H][BOARD_W]; // 0=empty, 1~7=piece type
        int score = 0;
        int lines = 0;
        bool gameOver = false;

        // current falling piece
        int curType = 0;    // 0..6
        int curRot = 0;    // 0..3
        int curX = 3;    // spawn x
        int curY = 0;    // spawn y (top)

        // next piece
        int nextType = 0;

        // RNG (simple LCG)
        uint32_t rng = 0x12345678u;
    };

    // ---------- lifecycle ----------
    void Game_Init(Game* g, uint32_t seed = 0);
    void Game_Reset(Game* g, uint32_t seed = 0);

    // ---------- tick & input ----------
    void Game_Tick(Game* g);                 // one gravity step
    MoveResult Game_Move(Game* g, int dx, int dy);
    MoveResult Game_RotateCW(Game* g);
    void Game_HardDrop(Game* g);

    // ---------- query ----------
    bool Game_IsGameOver(const Game* g);
    int  Game_GetScore(const Game* g);
    int  Game_GetLines(const Game* g);
    const int* Game_GetBoardPtr(const Game* g);  // pointer to 200 ints, row-major [y*10+x]
    PieceBlocks Game_GetCurrentPieceBlocks(const Game* g);
    int  Game_GetNextType(const Game* g);

} // namespace tetris