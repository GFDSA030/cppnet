#include "D_getch.h"
#ifdef _WIN32
#include <conio.h> // _getch()
#else
#include <termios.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int D_getch()
{
#ifdef _WIN32
    return _getch();
#else
    struct termios oldt, newt;
    int c;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    c = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return c;
#endif
}
int D_kbhit()
{
#ifdef _WIN32
    return _kbhit();
#else
    struct termios oldt, newt;
    int c;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    c = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (c != EOF)
    {
        ungetc(c, stdin);
        return 1;
    }
    return 0;
#endif
}
