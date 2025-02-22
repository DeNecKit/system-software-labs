#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <wchar.h>
#include <locale.h>

#define error(...)                     \
    do {                               \
        fwprintf(stderr, __VA_ARGS__); \
        exit(EXIT_FAILURE);            \
    } while (0);

bool do_stick = false;
int dx, dy;
int mx, my;
int moves = 0;

#define SENS 10

LRESULT CALLBACK wnd_proc(
    HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_PAINT: return 0;
        case WM_CREATE: return 0;
        case WM_NCRBUTTONDOWN: {
            do_stick = true;
            RECT wnd_rect;
            GetWindowRect(window, &wnd_rect);
            POINT mpos;
            GetCursorPos(&mpos);
            dx = wnd_rect.left - mpos.x;
            dy = wnd_rect.top - mpos.y;
            mx = mpos.x;
            my = mpos.y;
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(window, msg, wparam, lparam);
}

void stick(HWND window)
{
    if (do_stick) {
        if (moves == 3) {
            do_stick = false;
            moves = 0;
        }
        POINT mpos;
        GetCursorPos(&mpos);
        SetWindowPos(window, NULL,
            mpos.x + dx, mpos.y + dy, 0, 0, SWP_NOSIZE);
        int mdx = mpos.x - mx;
        int mdy = mpos.y - my;
        if (abs(mdx) >= SENS || abs(mdy) >= SENS) {
            if (mdx >= SENS && abs(mdy) <= SENS / 4) {
                wprintf(L"Вправо\n");
                if (moves == 0) moves = 1;
                else if (moves != 1) { wprintf(L"Неправильное движение\n"); moves = 0; }
            }
            else if (mdx <= SENS && abs(mdy) <= SENS / 4) {
                wprintf(L"Влево\n");
                if (moves == 1) moves = 2;
                else if (moves != 2) { wprintf(L"Неправильное движение\n"); moves = 0; }
            }
            else if (mdy >= SENS && abs(mdx) <= SENS / 4) {
                wprintf(L"Вниз\n");
                if (moves == 2) moves = 3;
                else if (moves != 3) { wprintf(L"Неправильное движение\n"); moves = 0; }
            }
            else if (mdy <= SENS && abs(mdx) <= SENS / 4) {
                wprintf(L"Вверх\n");
                wprintf(L"Неправильное движение\n"); moves = 0;
            }
            mx = mpos.x;
            my = mpos.y;
        }
    }
}

int WINAPI WinMain(
    HINSTANCE h, HINSTANCE , char*, int CmdShow)
{
    setlocale(LC_ALL, "Russian"); // TODO

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = h;
    wc.lpszClassName = "Window class";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    if (!RegisterClass(&wc)) error(L"Не удалось создать класс окна\n");

    HWND window = CreateWindowW(
#ifdef __MINGW32__
        L"Window class",
#elif
        "Window class",
#endif
        L"Окно", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        HWND_DESKTOP, NULL, h, NULL);

    if (window == NULL) error(L"Не удалось создать окно\n");

    ShowWindow(window, CmdShow);
    UpdateWindow(window);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        stick(window);
    }

    return msg.wParam;
}
