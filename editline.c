#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/*
 */
bool EditLine(char cin, char *cout, char line[], int size)
{
    static int pos = 0;

    // echo by default
    *cout = cin;

    switch (cin) {

    case '\r':
        // carriage return is ignored
        break;

    case '\n':
        // end-of-line
        line[pos] = '\0';
        pos = 0;
        return true;

    case 0x7F:
    case 0x08:
        // backspace
        if (pos > 0) {
            pos--;
        }
        break;

    default:
        // store char as long as there is space to do so
        if (pos < (size - 1)) {
            line[pos++] = cin;
        } else {
            *cout = 0x07; // bell
        }
        break;
    }

    return false;
}


