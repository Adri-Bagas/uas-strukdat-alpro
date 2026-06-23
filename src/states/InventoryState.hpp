#pragma once
#include "GameState.hpp"
#include <vector>
#include <string>

struct Item;

class InventoryState : public GameState {
private:
    int selection_index = 0;
    std::vector<std::pair<const Item*, int>> current_items;
    std::string message;

    void refresh_inventory();
public:
    InventoryState(GameEngine* eng);
    void on_enter() override;
    void handle_input(int ch) override;
    void update() override;
    void render() override;
};
