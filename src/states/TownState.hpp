#pragma once
#include "GameState.hpp"
#include "../models/Activity.hpp"
#include <vector>

class TownState : public GameState {
    int selection_index = 0;
    std::vector<Activity> current_activities;
public:
    TownState(GameEngine* eng);
    
    void handle_input(int ch) override;
    void update() override;
    void render() override;
    void on_enter() override;
};