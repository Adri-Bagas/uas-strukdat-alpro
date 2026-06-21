#pragma once
#include <ncurses.h>

namespace Utils {
// Call this after initscr()
inline void request_fullscreen() {
    printf("\033[9;1t"); // Request fullscreen
    fflush(stdout);
}
} // namespace Utils