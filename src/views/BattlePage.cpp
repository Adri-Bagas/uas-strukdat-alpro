#include "BattlePage.hpp"
#include "../models/NPC.hpp"

void BattlePage::draw(
    const std::array<Entity*, 4>& party,
    const std::array<Entity*, 4>& enemies,
    const std::vector<Entity*>& turn_queue,
    int active_party_idx,
    int current_menu_selection,
    const std::vector<std::string>& menu_options,
    const std::vector<std::string>& battle_log,
    size_t enemy_pool_size,
    Entity* flashing_entity
) {
    erase(); // Use erase instead of clear to reduce flickering

    int my, mx;
    getmaxyx(stdscr, my, mx);

    if (my < 22 || mx < 80) {
        mvprintw(my / 2, (mx - 30) / 2, "TERMINAL TERLALU KECIL (%dx%d)", mx, my);
        mvprintw(my / 2 + 1, (mx - 30) / 2, "Dibutuhkan: 80x22");
        refresh(); 
        return;
    }

    int left_w = mx * 0.25; // Log width reduced to 25%
    int right_w = mx - left_w;

    auto draw_box = [&](int y, int x, int w, int h, bool active, bool is_enemy, bool is_flashing) {
        int color = is_enemy ? 5 : 4;
        if (is_flashing) {
            attron(COLOR_PAIR(color) | A_BOLD | A_REVERSE);
        } else if (active) {
            attron(COLOR_PAIR(color) | A_BOLD);
        }
        
        mvaddch(y, x, ACS_ULCORNER);
        mvhline(y, x + 1, ACS_HLINE, w - 2);
        mvaddch(y, x + w - 1, ACS_URCORNER);
        
        mvvline(y + 1, x, ACS_VLINE, h - 2);
        mvvline(y + 1, x + w - 1, ACS_VLINE, h - 2);
        
        mvaddch(y + h - 1, x, ACS_LLCORNER);
        mvhline(y + h - 1, x + 1, ACS_HLINE, w - 2);
        mvaddch(y + h - 1, x + w - 1, ACS_LRCORNER);
        
        if (is_flashing) {
            attroff(COLOR_PAIR(color) | A_BOLD | A_REVERSE);
        } else if (active) {
            attroff(COLOR_PAIR(color) | A_BOLD);
        }
    };

    // --- BATTLE LOG (LEFT) ---
    draw_box(0, 0, left_w, my, false, false, false);
    attron(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
    mvwprintw(stdscr, 0, 2, " BATTLE LOG ");
    attroff(COLOR_PAIR(4) | A_REVERSE | A_BOLD);

    int max_logs = my - 2;
    int start_log = 0;
    int num_logs = battle_log.size();
    if (num_logs > max_logs) {
        start_log = num_logs - max_logs;
    }

    // Text from bottom to up
    int log_precision = std::max(0, left_w - 4);
    int logs_to_draw = std::min(num_logs, max_logs);
    for (int i = 0; i < logs_to_draw; ++i) {
        int draw_y = (my - 1) - logs_to_draw + i;
        mvprintw(draw_y, 2, "%.*s", log_precision, battle_log[start_log + i].c_str());
    }

    // --- MAIN BATTLE AREA (RIGHT) ---
    draw_box(0, left_w, right_w, my, false, false, false);
    attron(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
    mvwprintw(stdscr, 0, left_w + 2, " BATTLE ARENA ");
    attroff(COLOR_PAIR(4) | A_REVERSE | A_BOLD);

    int box_w = 18;
    int box_h = 7;
    int text_w = box_w - 4;
    int gap = 2;
    int lanes_w = 4 * box_w + 3 * gap;
    
    // Center lanes in the right side
    int start_x = left_w + (right_w - lanes_w) / 2;
    if (start_x < left_w + 1) start_x = left_w + 1;
    
    int content_h = box_h * 2 + 4 + 10; 
    int start_y = (my - content_h) / 2;
    if (start_y < 2) start_y = 2; 

    mvprintw(start_y - 1, start_x, "Turn Queue:");
    int q_x = start_x + 12;
    for (size_t i = 0; i < turn_queue.size() && q_x < start_x + lanes_w - 5; ++i) {
        if (i == 0) attron(A_REVERSE);
        std::string name = turn_queue[i] ? turn_queue[i]->get_name() : "???";
        int remain = (start_x + lanes_w - 5) - q_x - 2;
        if (remain < 3) break;
        mvprintw(start_y - 1, q_x, "[%.*s]", remain, name.c_str());
        if (i == 0) attroff(A_REVERSE);
        q_x += std::min((int)name.length(), remain) + 3;
    }

    int enemy_y = start_y + 2;
    int party_y = enemy_y + box_h + 2;

    // Enemies
    mvprintw(enemy_y - 1, start_x, "--- ENEMIES --- (Waiting in queue: %zu)", enemy_pool_size);
    for (int i = 0; i < 4; ++i) {
        int x = start_x + i * (box_w + gap);
        bool is_active = (enemies[i] && turn_queue.size() > 0 && turn_queue[0] == enemies[i]);
        bool is_flashing = (enemies[i] && enemies[i] == flashing_entity);
        
        draw_box(enemy_y, x, box_w, box_h, is_active, true, is_flashing);
        if (enemies[i]) {
            Entity* e = enemies[i];
            if (is_flashing) attron(COLOR_PAIR(5) | A_BOLD | A_REVERSE);
            else if (is_active) attron(COLOR_PAIR(5) | A_BOLD);
            
            mvprintw(enemy_y + 1, x + 2, "Slot %d", i + 1);
            mvprintw(enemy_y + 2, x + 2, "%.*s", text_w, e->get_name().c_str());
            std::string hp_s = "HP:" + std::to_string(e->get_hp()) + "/" + std::to_string(e->get_max_hp());
            mvprintw(enemy_y + 4, x + 2, "%.*s", text_w, hp_s.c_str());
            
            if (is_flashing) attroff(COLOR_PAIR(5) | A_BOLD | A_REVERSE);
            else if (is_active) attroff(COLOR_PAIR(5) | A_BOLD);
        } else {
            attron(COLOR_PAIR(2));
            mvprintw(enemy_y + 3, x + 4, "(Kosong)");
            attroff(COLOR_PAIR(2));
        }
    }

    // Party
    mvprintw(party_y - 1, start_x, "--- PARTY ---");
    for (int i = 0; i < 4; ++i) {
        int x = start_x + i * (box_w + gap);
        bool is_active = (party[i] && turn_queue.size() > 0 && turn_queue[0] == party[i]);
        bool is_flashing = (party[i] && party[i] == flashing_entity);
        
        draw_box(party_y, x, box_w, box_h, is_active, false, is_flashing);
        if (party[i]) {
            Entity* e = party[i];
            if (is_flashing) attron(COLOR_PAIR(4) | A_BOLD | A_REVERSE);
            else if (is_active) attron(COLOR_PAIR(4) | A_BOLD);
            
            mvprintw(party_y + 1, x + 2, "Slot %d", i + 1);
            mvprintw(party_y + 2, x + 2, "%.*s", text_w, e->get_name().c_str());
            { std::string s = "HP:" + std::to_string(e->get_hp()) + "/" + std::to_string(e->get_max_hp()); mvprintw(party_y + 3, x + 2, "%.*s", text_w, s.c_str()); }
            { std::string s = "MP:" + std::to_string(e->get_mp()) + "/" + std::to_string(e->get_max_mp()); mvprintw(party_y + 4, x + 2, "%.*s", text_w, s.c_str()); }
            
            if (is_flashing) attroff(COLOR_PAIR(4) | A_BOLD | A_REVERSE);
            else if (is_active) attroff(COLOR_PAIR(4) | A_BOLD);
        } else {
            attron(COLOR_PAIR(2));
            mvprintw(party_y + 3, x + 4, "(Kosong)");
            attroff(COLOR_PAIR(2));
        }
    }

    // Menu
    if (!menu_options.empty()) {
        int menu_y = party_y + box_h + 1;
        int menu_h = std::min((int)menu_options.size() + 2, my - menu_y - 1);
        if (menu_h < 4) menu_h = 4; // minimum height
        int menu_txt_w = lanes_w - 4;
        
        draw_box(menu_y, start_x, lanes_w, menu_h, true, false, false); // active style (color 4)
        
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(menu_y, start_x + 2, " ACTION MENU ");
        attroff(COLOR_PAIR(4) | A_BOLD);

        for (size_t i = 0; i < menu_options.size() && (int)i < menu_h - 2; ++i) {
            if (static_cast<int>(i) == current_menu_selection) attron(A_REVERSE);
            mvprintw(menu_y + 1 + i, start_x + 2, "%.*s", menu_txt_w, menu_options[i].c_str());
            if (static_cast<int>(i) == current_menu_selection) attroff(A_REVERSE);
        }
    }

    wnoutrefresh(stdscr);
}
