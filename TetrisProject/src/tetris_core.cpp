#include "../include/tetris_core.h"
#include <ctime>

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;

const std::vector<std::vector<Point>> TETROMINOS = {
    {{0,1}, {1,1}, {2,1}, {3,1}}, // I
    {{0,0}, {0,1}, {1,1}, {2,1}}, // J
    {{2,0}, {0,1}, {1,1}, {2,1}}, // L
    {{1,0}, {2,0}, {1,1}, {2,1}}, // O
    {{1,0}, {2,0}, {0,1}, {1,1}}, // S
    {{1,0}, {0,1}, {1,1}, {2,1}}, // T
    {{0,0}, {1,0}, {1,1}, {2,1}}  // Z
};

TetrisGame::TetrisGame() {
    rng.seed(std::time(nullptr));
    Init();
}

void TetrisGame::Init() {
    board.assign(BOARD_HEIGHT, std::vector<int>(BOARD_WIDTH, 0));
    score = 0;
    state = GameState::PLAYING;
    timer = 0;
    SpawnPiece();
}

void TetrisGame::SpawnPiece() {
    int type = rng() % 7;
    currentPiece = TETROMINOS[type];
    currentColor = type + 1;
    currentPos = { 3, 0 };

    if (CheckCollision(currentPiece, currentPos.x, currentPos.y)) {
        state = GameState::GAME_OVER;
    }
}

bool TetrisGame::CheckCollision(const std::vector<Point>& piece, int offX, int offY) {
    for (auto& p : piece) {
        int nx = p.x + offX;
        int ny = p.y + offY;
        if (nx < 0 || nx >= BOARD_WIDTH || ny >= BOARD_HEIGHT) return true;
        if (ny >= 0 && board[ny][nx] != 0) return true;
    }
    return false;
}

void TetrisGame::Rotate() {
    if (currentColor == 4) return;
    std::vector<Point> nextPiece = currentPiece;
    Point center = nextPiece[2];
    for (auto& p : nextPiece) {
        int x = p.x - center.x;
        int y = p.y - center.y;
        p.x = center.x - y;
        p.y = center.y + x;
    }
    if (!CheckCollision(nextPiece, currentPos.x, currentPos.y)) {
        currentPiece = nextPiece;
    }
}

void TetrisGame::Input(int key) {
    if (state != GameState::PLAYING) return;

    if (key == 37) { // Left
        if (!CheckCollision(currentPiece, currentPos.x - 1, currentPos.y))
            currentPos.x--;
    }
    else if (key == 39) { // Right
        if (!CheckCollision(currentPiece, currentPos.x + 1, currentPos.y))
            currentPos.x++;
    }
    else if (key == 40) { // Down
        if (!CheckCollision(currentPiece, currentPos.x, currentPos.y + 1))
            currentPos.y++;
    }
    else if (key == 38) { // Up (Rotate)
        Rotate();
    }
}

void TetrisGame::Update() {
    if (state != GameState::PLAYING) return;

    timer++;
    if (timer > 10) {
        timer = 0;
        if (!CheckCollision(currentPiece, currentPos.x, currentPos.y + 1)) {
            currentPos.y++;
        }
        else {
            MergePiece();
            ClearLines();
            SpawnPiece();
        }
    }
}

void TetrisGame::MergePiece() {
    for (auto& p : currentPiece) {
        int nx = p.x + currentPos.x;
        int ny = p.y + currentPos.y;
        if (ny >= 0 && ny < BOARD_HEIGHT && nx >= 0 && nx < BOARD_WIDTH) {
            board[ny][nx] = currentColor;
        }
    }
}

void TetrisGame::ClearLines() {
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        bool full = true;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x] == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            board.erase(board.begin() + y);
            board.insert(board.begin(), std::vector<int>(BOARD_WIDTH, 0));
            score += 100;
            y++;
        }
    }
}

const std::vector<std::vector<int>>& TetrisGame::GetBoard() const { return board; }
const std::vector<Point>& TetrisGame::GetCurrentPiece() const { return currentPiece; }
int TetrisGame::GetCurrentColor() const { return currentColor; }
Point TetrisGame::GetCurrentPos() const { return currentPos; }
GameState TetrisGame::GetState() const { return state; }
int TetrisGame::GetScore() const { return score; }