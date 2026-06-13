#pragma once
#include "GameState.hpp"
#include <vector>

class DungeonState : public GameState {
private:
    int height;
    int width;
    std::vector<std::vector<int>> grid; // 1 = WALL, 0 = PASSAGE
    int player_r, player_c;
    int exit_r, exit_c;
    bool has_won;

    void generate_maze();

public:
    DungeonState(GameEngine* eng);
    
    void handle_input(int ch) override;
    void update() override;
    void render() override;
    void on_enter() override;
};