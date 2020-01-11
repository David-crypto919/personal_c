#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

#include "tictactoe.h"
#include "twozerofoureight.h"
#include "hangman.h"

#if defined(_WIN32) || defined(_WIN64)
static inline int getFontSize(HANDLE windowHandle, COORD* size)
{
    CONSOLE_FONT_INFOEX font = { sizeof(CONSOLE_FONT_INFOEX) };

    if (!GetCurrentConsoleFontEx(windowHandle, 0, &font))
        return 0;

    *size = font.dwFontSize;

    return 1;
}

static inline void setFontSize(HANDLE windowHandle, COORD size)
{
    CONSOLE_FONT_INFOEX font = { sizeof(CONSOLE_FONT_INFOEX) };

    if (!GetCurrentConsoleFontEx(windowHandle, 0, &font))
        return;

    font.dwFontSize = size;

    if (!SetCurrentConsoleFontEx(windowHandle, 0, &font))
        return;
}

static inline void initFont(void)
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD size;

    if (getFontSize(h, &size)) {
        size.X += (SHORT)(size.X * .5);
        size.Y += (SHORT)(size.Y * .5);
        setFontSize(h, size);
    }
}
#else
static inline void initFont(void)
{
}
#endif

int main()
{
	int x;

	printf("What game do you want to play?\n");
	printf("1. for TicTacToe\n");
	printf("2. for 2048\n");
    printf("3. for HangMan\n");
	(void)scanf("%d", &x);

    switch (x) {
    case 1:
        initFont();
        TicTacToe_launch();
        break;
    case 2:
        initFont();
        TwoZeroFourEight_launch();
        break;      
    case 3:
        HangMan_launch();
        break;      
    default:
        break;
    }

	return 0;
}