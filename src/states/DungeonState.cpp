#include "DungeonState.hpp"
#include "../GameEngine.hpp" // Full include allowed in .cpp

DungeonState::DungeonState(GameEngine* eng) : GameState(eng) {}

void DungeonState::on_enter() {
    
}

void DungeonState::handle_input(int ch) {
    
}

void DungeonState::update() {
    // Check traps, update animations
}

void DungeonState::render() {
    // Engine holds the layout view
}