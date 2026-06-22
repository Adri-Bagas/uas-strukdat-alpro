#include "src/GameEngine.hpp"
#include "src/states/TownState.hpp"
#include "src/models/Place.hpp"
#include <iostream>

int main() {
    GameEngine engine;
    // We cannot fully init ncurses easily in a test script, so we will just test logic.
    // Actually, GameEngine::init() calls initscr(), so we can't just run it without a terminal.
    return 0;
}
