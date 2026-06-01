#include <ncurses.h>
#include "GameEngine.hpp"

int main() {
    GameEngine engine;
    engine.init();
    engine.run();

    endwin();
    return 0;
}