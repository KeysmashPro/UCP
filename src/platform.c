/* platform.c */

#include "platform.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

void platform_init(void)
{
    HANDLE handles[2] = {
        GetStdHandle(STD_OUTPUT_HANDLE),
        GetStdHandle(STD_ERROR_HANDLE)
    };

    for (int i = 0; i < 2; ++i) {
        if (handles[i] == INVALID_HANDLE_VALUE || handles[i] == NULL) continue;
        DWORD mode = 0;
        if (!GetConsoleMode(handles[i], &mode)) continue;
        SetConsoleMode(handles[i], mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

#else

void platform_init(void) {}

#endif
