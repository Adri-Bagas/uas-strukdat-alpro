#include "BattlePage.hpp"
#include "../models/NPC.hpp"

void BattlePage::draw(
    const std::array<Entity*, 4>& party_slots,
    const std::array<Entity*, 4>& enemy_slots,
    const std::vector<Entity*>& turn_queue,
    int active_entity_idx,
    int current_menu_selection,
    const std::vector<std::string>& menu_options,
    const std::string& battle_log
) {
    clear();

    int my, mx;
    getmaxyx(stdscr, my, mx);

    // Draw header / Turn Queue
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(1, 2, "=== BATTLE ===");
    attroff(COLOR_PAIR(4) | A_BOLD);

    mvprintw(2, 2, "Turn Queue:");
    int q_x = 14;
    for (size_t i = 0; i < turn_queue.size() && q_x < mx - 10; ++i) {
        if (i == 0) attron(A_REVERSE); // Active turn
        
        std::string name = turn_queue[i] ? turn_queue[i]->get_name() : "???";
        mvprintw(2, q_x, "[%s]", name.c_str());
        
        if (i == 0) attroff(A_REVERSE);
        q_x += name.length() + 3;
    }

    // Draw Lanes
    int col_w = (mx - 6) / 4;
    int party_y = 6;
    int enemy_y = party_y + 8;

    mvprintw(party_y - 2, 2, "--- PARTY ---");
    for (int i = 0; i < 4; ++i) {
        int x = 3 + i * col_w;
        if (party_slots[i]) {
            Entity* e = party_slots[i];
            // Highlight if active
            bool is_active = (turn_queue.size() > 0 && turn_queue[0] == e);
            if (is_active) attron(COLOR_PAIR(4) | A_BOLD);
            
            mvprintw(party_y, x, "Slot %d", i + 1);
            mvprintw(party_y + 1, x, "%s", e->get_name().c_str());
            mvprintw(party_y + 2, x, "HP: %d/%d", e->get_hp(), e->get_max_hp());
            mvprintw(party_y + 3, x, "MP: %d/%d", e->get_mp(), e->get_max_mp());
            
            if (is_active) attroff(COLOR_PAIR(4) | A_BOLD);
        } else {
            attron(COLOR_PAIR(2));
            mvprintw(party_y, x, "Slot %d (Kosong)", i + 1);
            attroff(COLOR_PAIR(2));
        }
    }

    mvprintw(enemy_y - 2, 2, "--- ENEMIES ---");
    for (int i = 0; i < 4; ++i) {
        int x = 3 + i * col_w;
        if (enemy_slots[i]) {
            Entity* e = enemy_slots[i];
            bool is_active = (turn_queue.size() > 0 && turn_queue[0] == e);
            if (is_active) attron(COLOR_PAIR(5) | A_BOLD); // Enemy active red
            
            mvprintw(enemy_y, x, "Slot %d", i + 1);
            mvprintw(enemy_y + 1, x, "%s", e->get_name().c_str());
            mvprintw(enemy_y + 2, x, "HP: %d/%d", e->get_hp(), e->get_max_hp());
            
            if (is_active) attroff(COLOR_PAIR(5) | A_BOLD);
        } else {
            attron(COLOR_PAIR(2));
            mvprintw(enemy_y, x, "Slot %d (Kosong)", i + 1);
            attroff(COLOR_PAIR(2));
        }
    }

    // Draw Battle Log
    int log_y = enemy_y + 6;
    mvprintw(log_y, 2, "Log: %s", battle_log.c_str());

    // Draw Menu (only if menu_options is not empty, means player action selection)
    if (!menu_options.empty()) {
        int menu_y = log_y + 2;
        mvprintw(menu_y, 2, "Select Action:");
        for (size_t i = 0; i < menu_options.size(); ++i) {
            if (static_cast<int>(i) == current_menu_selection) {
                attron(A_REVERSE);
            }
            mvprintw(menu_y + 1 + i, 4, "%s", menu_options[i].c_str());
            if (static_cast<int>(i) == current_menu_selection) {
                attroff(A_REVERSE);
            }
        }
    }

    refresh();
}
