#pragma once
#include "GameState.hpp"
#include <string>
#include <vector>

class StatAllocationState : public GameState {
private:
    int current_selection;
    std::vector<std::string> stat_names;

public:
    StatAllocationState(GameEngine* eng);
    void on_enter() override;
    void handle_input(int ch) override;
    void update() override;
    void render() override;
    void on_exit() override;
};
