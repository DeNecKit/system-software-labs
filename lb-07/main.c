#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wchar.h>
#include <locale.h>

#define werror_exit(...)               \
    do {                               \
        fwprintf(stderr, __VA_ARGS__); \
        exit(EXIT_FAILURE);            \
    } while (0)

#define werror(...) fwprintf(stderr, __VA_ARGS__);

DWORD is_even(LPVOID args)
{
    DWORDLONG x = (DWORDLONG)args;
    if (!(x == 1 || x >> 1 << 1 == x)) return (DWORD)-1;
    DWORD pow = 0;
    while (x > 1) {
        pow++;
        x >>= 1;
    }
    return pow;
}

int main(void)
{
    setlocale(LC_ALL, "Russian");

    DWORDLONG x;
    wprintf(L"Введите число: ");
    if (scanf("%lld", &x) != 1)
        werror_exit(L"Не удалось прочитать число\n");

    HANDLE thread = CreateThread(NULL, 0, is_even,
        (LPVOID)x, CREATE_SUSPENDED, NULL);
    // SetThreadPriority(thread, THREAD_PRIORITY_LOWEST);
    SetThreadPriority(thread, THREAD_PRIORITY_TIME_CRITICAL);
    
    LARGE_INTEGER freq, start, end;
    float time;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    
    ResumeThread(thread);
    WaitForSingleObject(thread, INFINITE);

    QueryPerformanceCounter(&end);
    time = (float)(end.QuadPart - start.QuadPart) / freq.QuadPart;

    DWORD res;
    if (!GetExitCodeThread(thread, &res)) {
        DWORD err = GetLastError();
        LPVOID msg;

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, err,
        MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT),
        (LPSTR)&msg, 0, NULL);

        werror(L"Поток завершился неудачно: %S",
               (LPCSTR)msg);
        LocalFree(msg);
        exit(EXIT_FAILURE);
    }

    if (res != (DWORD)-1) wprintf(L"Степень двойки: %d\n", res);
    else wprintf(L"Не является степенью двойки\n");

    wprintf(L"Время выполнения потока: %.2fмс\n", time * 1000);
}
