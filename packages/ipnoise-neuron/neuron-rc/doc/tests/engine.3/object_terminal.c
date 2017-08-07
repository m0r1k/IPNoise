#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "object_terminal.h"

uint8_t object_terminal__getchar_hidden()
{
    static struct termios   oldt, newt;
    uint8_t                 c;

    // tcgetattr gets the parameters of the current terminal
    // STDIN_FILENO will tell tcgetattr that it should write
    // the settings of stdin to oldt
    tcgetattr(STDIN_FILENO, &oldt);

    // now the settings will be copied
    newt = oldt;

    // ICANON normally takes care that one line at a time
    // will be processed that means it will return
    // if it sees a "\n" or an EOF or an EOL
    newt.c_lflag &= ~(ICANON | ECHO);

    // those new settings will be set to STDIN
    // TCSANOW tells tcsetattr to change attributes immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    c = getchar();
    // putchar(c);

    // restore the old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return c;
}

