#pragma once
#include "GameState.hpp"
#include <string>
#include <vector>


class ShopState : public GameState {
private:
    int current_selection;
    std::string shop_id;
    std::vector<std::string> display_items;

public:
    ShopState(GameEngine* eng, const std::string& shop_id);
    void on_enter() override;
    void handle_input(int ch) override;
    void update() override;
    void render() override;
    void on_exit() override;
};
