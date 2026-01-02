#include <windows.h>
#include "../include/tetris_core.h"
#include <string>

TetrisGame game;
const int CELL_SIZE = 30;

void DrawRect(HDC hdc, int x, int y, int colorIndex) {
    COLORREF colors[] = {
        RGB(0,0,0), RGB(0,255,255), RGB(0,0,255), RGB(255,165,0),
        RGB(255,255,0), RGB(0,255,0), RGB(128,0,128), RGB(255,0,0)
    };
    HBRUSH hBrush = CreateSolidBrush(colors[colorIndex]);
    RECT rect = { x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE };
    FillRect(hdc, &rect, hBrush);
    FrameRect(hdc, &rect, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
    DeleteObject(hBrush);
}

void Draw(HDC hdc) {
    HDC hMemDC = CreateCompatibleDC(hdc);

    // 【修正1】將畫布寬度從 350 改為 450 (配合視窗寬度)，避免文字被切掉
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 450, 650);
    SelectObject(hMemDC, hBitmap);

    // 【修正2】背景填色範圍也改成 450
    RECT bg = { 0, 0, 450, 650 };
    FillRect(hMemDC, &bg, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // 畫棋盤
    auto board = game.GetBoard();
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 10; x++) {
            if (board[y][x] != 0) DrawRect(hMemDC, x, y, board[y][x]);
            else {
                RECT grid = { x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE };
                FrameRect(hMemDC, &grid, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
            }
        }
    }

    // 畫當前落下的方塊
    auto piece = game.GetCurrentPiece();
    auto pos = game.GetCurrentPos();
    int color = game.GetCurrentColor();
    for (auto& p : piece) {
        DrawRect(hMemDC, p.x + pos.x, p.y + pos.y, color);
    }

    // 【修正3】顯示分數：確保文字在畫布範圍內
    std::wstring s = L"Score: " + std::to_wstring(game.GetScore());
    TextOut(hMemDC, 320, 20, s.c_str(), s.length());

    // 顯示說明文字 (選用)
    TextOut(hMemDC, 320, 50, L"操作方式:", 5); // 5 是字數
    TextOut(hMemDC, 320, 70, L"方向鍵", 3);    // 3 是字數
    if (game.GetState() == GameState::GAME_OVER) {
        // 讓 Game Over 顯示明顯一點
        SetTextColor(hMemDC, RGB(255, 0, 0));
        TextOut(hMemDC, 120, 300, L"GAME OVER", 9);
        SetTextColor(hMemDC, RGB(0, 0, 0)); // 改回黑色
    }

    // 【修正4】複製到螢幕的範圍也要改成 450
    BitBlt(hdc, 0, 0, 450, 650, hMemDC, 0, 0, SRCCOPY);

    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        SetTimer(hwnd, 1, 50, NULL);
        return 0;
    case WM_TIMER:
        game.Update();
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_KEYDOWN:
        game.Input((int)wParam);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Draw(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"TetrisWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"Tetris Game", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 450, 650,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}