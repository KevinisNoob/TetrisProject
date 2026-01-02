// ui_win32.cpp  (Kevin / B: Win32 GUI + timer + grid + test block)
// Build: Windows subsystem (wWinMain)

#include <windows.h>

// ------------------------------
// Config
// ------------------------------
static constexpr int BOARD_W = 10;
static constexpr int BOARD_H = 20;

static constexpr int CELL = 24;   // cell size in pixels
static constexpr int ORIGIN_X = 20;   // board top-left (x)
static constexpr int ORIGIN_Y = 20;   // board top-left (y)

static constexpr UINT_PTR TIMER_ID = 1;
static constexpr UINT TIMER_MS = 500;

// ------------------------------
// Simple demo state (no core yet)
// ------------------------------
static int g_px = 4;   // piece x (in cells)
static int g_py = 0;   // piece y (in cells)
static bool g_paused = false;

// 2x2 block
static constexpr int PIECE_W = 2;
static constexpr int PIECE_H = 2;

// ------------------------------
// GDI objects
// ------------------------------
static HPEN   g_penGrid = nullptr;
static HPEN   g_penBorder = nullptr;
static HBRUSH g_brushPiece = nullptr;
static HBRUSH g_brushBg = nullptr;

// ------------------------------
// Helpers
// ------------------------------
static RECT CellRect(int cx, int cy)
{
    RECT r;
    r.left = ORIGIN_X + cx * CELL;
    r.top = ORIGIN_Y + cy * CELL;
    r.right = r.left + CELL;
    r.bottom = r.top + CELL;
    return r;
}

static void ClampPiece()
{
    if (g_px < 0) g_px = 0;
    if (g_px > BOARD_W - PIECE_W) g_px = BOARD_W - PIECE_W;

    if (g_py < 0) g_py = 0;
    if (g_py > BOARD_H - PIECE_H) g_py = BOARD_H - PIECE_H;
}

static void TickDown()
{
    if (g_paused) return;

    g_py += 1;
    if (g_py > BOARD_H - PIECE_H) {
        // demo: reset to top when hit bottom
        g_py = 0;
    }
}

static void DrawGrid(HDC hdc)
{
    // Draw outer border
    SelectObject(hdc, g_penBorder);
    Rectangle(
        hdc,
        ORIGIN_X,
        ORIGIN_Y,
        ORIGIN_X + BOARD_W * CELL,
        ORIGIN_Y + BOARD_H * CELL
    );

    // Draw inner grid lines
    SelectObject(hdc, g_penGrid);

    // vertical lines
    for (int x = 1; x < BOARD_W; ++x) {
        int px = ORIGIN_X + x * CELL;
        MoveToEx(hdc, px, ORIGIN_Y, nullptr);
        LineTo(hdc, px, ORIGIN_Y + BOARD_H * CELL);
    }

    // horizontal lines
    for (int y = 1; y < BOARD_H; ++y) {
        int py = ORIGIN_Y + y * CELL;
        MoveToEx(hdc, ORIGIN_X, py, nullptr);
        LineTo(hdc, ORIGIN_X + BOARD_W * CELL, py);
    }
}

static void DrawPiece(HDC hdc)
{
    // draw 2x2 block at (g_px, g_py)
    SelectObject(hdc, g_brushPiece);
    SelectObject(hdc, g_penBorder);

    for (int dy = 0; dy < PIECE_H; ++dy) {
        for (int dx = 0; dx < PIECE_W; ++dx) {
            int cx = g_px + dx;
            int cy = g_py + dy;

            RECT r = CellRect(cx, cy);
            // small inset to avoid overwriting grid lines too much
            InflateRect(&r, -2, -2);
            Rectangle(hdc, r.left, r.top, r.right, r.bottom);
        }
    }
}

// Double-buffer paint to reduce flicker
static void Paint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT client{};
    GetClientRect(hwnd, &client);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, client.right - client.left, client.bottom - client.top);
    HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

    // background
    FillRect(memDC, &client, g_brushBg);

    // board + piece
    DrawGrid(memDC);
    DrawPiece(memDC);

    // blit
    BitBlt(hdc, 0, 0, client.right - client.left, client.bottom - client.top, memDC, 0, 0, SRCCOPY);

    // cleanup
    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);

    EndPaint(hwnd, &ps);
}

// ------------------------------
// Window Proc
// ------------------------------
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE:
        // Create GDI objects
        g_penBorder = CreatePen(PS_SOLID, 2, RGB(60, 60, 60));
        g_penGrid = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        g_brushPiece = CreateSolidBrush(RGB(80, 140, 240));
        g_brushBg = CreateSolidBrush(RGB(255, 255, 255));

        SetTimer(hwnd, TIMER_ID, TIMER_MS, nullptr);
        return 0;

    case WM_TIMER:
        if (wParam == TIMER_ID) {
            TickDown();
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_LEFT:
            g_px -= 1;
            ClampPiece();
            InvalidateRect(hwnd, nullptr, FALSE);
            break;

        case VK_RIGHT:
            g_px += 1;
            ClampPiece();
            InvalidateRect(hwnd, nullptr, FALSE);
            break;

        case VK_DOWN:
            g_py += 1;
            ClampPiece();
            InvalidateRect(hwnd, nullptr, FALSE);
            break;

        case VK_SPACE:
            // hard drop to bottom (demo)
            g_py = BOARD_H - PIECE_H;
            InvalidateRect(hwnd, nullptr, FALSE);
            break;

        case 'P':
            g_paused = !g_paused;
            break;

        default:
            break;
        }
        return 0;

    case WM_PAINT:
        Paint(hwnd);
        return 0;

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);

        if (g_penBorder) DeleteObject(g_penBorder);
        if (g_penGrid) DeleteObject(g_penGrid);
        if (g_brushPiece) DeleteObject(g_brushPiece);
        if (g_brushBg) DeleteObject(g_brushBg);

        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ------------------------------
// Entry: wWinMain (Windows GUI)
// ------------------------------
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int nCmdShow)
{
    const wchar_t* CLASS_NAME = L"TetrisWin32Class";

    WNDCLASS wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wc)) {
        MessageBox(nullptr, L"RegisterClass failed.", L"Error", MB_ICONERROR);
        return 0;
    }

    // Window size to comfortably fit board + margins
    const int winW = ORIGIN_X * 2 + BOARD_W * CELL + 16;   // +16-ish for borders
    const int winH = ORIGIN_Y * 2 + BOARD_H * CELL + 39;   // +39-ish for title bar

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Tetris (Win32)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        winW, winH,
        nullptr, nullptr,
        hInst, nullptr
    );

    if (!hwnd) {
        MessageBox(nullptr, L"CreateWindowEx failed.", L"Error", MB_ICONERROR);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
