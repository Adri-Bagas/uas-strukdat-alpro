#pragma once
#include <ncurses.h>
#include <string>
#include <vector>
#include <array>

class Entity; // Forward declaration

class BattlePage {
public:
    BattlePage() = default;
    ~BattlePage() = default;

    // We assume there are 4 party slots and 4 enemy slots.
    // Turn queue is rendered on top or bottom.
    void draw(
        const std::array<Entity*, 4>& party_slots,
        const std::array<Entity*, 4>& enemy_slots,
        const std::vector<Entity*>& turn_queue,
        int active_entity_idx,
        int current_menu_selection,
        const std::vector<std::string>& menu_options,
        const std::string& battle_log
    );
};
