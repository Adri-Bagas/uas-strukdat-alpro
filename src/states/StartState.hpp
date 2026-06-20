#pragma once
#include "GameState.hpp"
#include <vector>
#include <string>

class StartState : public GameState {
private:
    int selection_index = 0;
    std::vector<std::string> menu_options = {
        "Start New",
        "Load",
        "Settings",
        "Exit"
    };

public:
    StartState(GameEngine* eng);

    void on_enter() override;
    void handle_input(int ch) override;
    void update() override;
    void render() override;
};
