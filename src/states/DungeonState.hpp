#pragma once
#include "GameState.hpp"

class DungeonState : public GameState {
public:
    DungeonState(GameEngine* eng);
    
    void handle_input(int ch) override;
    void update() override;
    void render() override;
    void on_enter() override;
};