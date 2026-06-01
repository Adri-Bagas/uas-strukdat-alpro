#pragma once
#include <ncurses.h>

// Call this after initscr()
inline void request_fullscreen() {
    printf("\033[9;1t"); // Request fullscreen
    fflush(stdout);
}