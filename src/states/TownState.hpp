#pragma once
#include "GameState.hpp"

class TownState : public GameState {
public:
    TownState(GameEngine* eng);
    
    void handle_input(int ch) override;
    void update() override;
    void render() override;
    void on_enter() override;
};