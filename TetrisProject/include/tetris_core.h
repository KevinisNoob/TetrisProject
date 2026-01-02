#pragma once
#include <vector>
#include <random>

enum class GameState { PLAYING, GAME_OVER };

struct Point { int x, y; };

class TetrisGame {
public:
    TetrisGame();
    void Init();
    void Update();
    void Input(int key);
    const std::vector<std::vector<int>>& GetBoard() const;
    const std::vector<Point>& GetCurrentPiece() const;
    int GetCurrentColor() const;
    Point GetCurrentPos() const;
    GameState GetState() const;
    int GetScore() const;

private:
    void SpawnPiece();
    bool CheckCollision(const std::vector<Point>& piece, int offX, int offY);
    void MergePiece();
    void ClearLines();
    void Rotate();

    std::vector<std::vector<int>> board;
    std::vector<Point> currentPiece;
    Point currentPos;
    int currentColor;
    GameState state;
    int score;
    int timer;
    std::mt19937 rng;
};