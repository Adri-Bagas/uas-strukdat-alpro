#include "StatAllocationState.hpp"
#include "../GameEngine.hpp"
#include <ncurses.h>
#include <algorithm>

StatAllocationState::StatAllocationState(GameEngine* eng) : GameState(eng), current_selection(0) {
    stat_names = {"Strength (STR) [+Damage/HP]", "Constitution (CON) [+HP/Def]", "Agility (AGI) [+Speed]", "Intelligence (INT) [+Magic Dmg]", "Wisdom (WIS) [+MP/Def]", "Done"};
}

void StatAllocationState::on_enter() {
    current_selection = 0;
    render();
}

void StatAllocationState::update() {
    // Nothing to update periodically
}

void StatAllocationState::handle_input(int ch) {
    if (ch == KEY_RESIZE) {
        engine->get_layout().resize();
        return;
    }

    if (ch == -1) return;
    
    Player* p = engine->get_player_manager().get_player();
    if (!p) return;

    if (ch == KEY_UP || ch == 'w') {
        current_selection--;
        if (current_selection < 0) current_selection = stat_names.size() - 1;
        engine->get_music_manager().playSfx("select_001.mp3");
    } else if (ch == KEY_DOWN || ch == 's') {
        current_selection++;
        if (current_selection >= stat_names.size()) current_selection = 0;
        engine->get_music_manager().playSfx("select_001.mp3");
    } else if (ch == 10) { // Enter
        if (current_selection == 5) { // Done
            engine->pop_state();
            return;
        }
        
        if (p->get_stat_points() > 0) {
            p->add_stat_points(-1);
            if (current_selection == 0) p->set_str(p->Entity::get_str() + 1);
            else if (current_selection == 1) { p->set_cons(p->get_cons() + 1); p->set_max_hp(p->get_max_hp() + 10); p->heal_hp(10); }
            else if (current_selection == 2) p->set_agi(p->get_agi() + 1);
            else if (current_selection == 3) p->set_intl(p->get_intl() + 1);
            else if (current_selection == 4) { p->set_wis(p->get_wis() + 1); p->set_max_mp(p->get_max_mp() + 5); p->restore_mp(5); }
        }
    } else if (ch == 'q' || ch == 27 || ch == 'c' || ch == 'C') { // Escape/Quit
        engine->pop_state();
        return;
    }
}

void StatAllocationState::render() {
    Player* p = engine->get_player_manager().get_player();
    if (!p) return;

    int my, mx;
    getmaxyx(stdscr, my, mx);
    if (my < 12 || mx < 40) {
        mvprintw(my / 2, std::max(0, (mx - 30) / 2), "TERMINAL TERLALU KECIL (%dx%d)", mx, my);
        refresh();
        return;
    }

    int popup_w = std::min(55, mx - 2);
    int popup_h = std::min((int)stat_names.size() + 6, my - 2);
    int popup_y = std::max(0, (my - popup_h) / 2);
    int popup_x = std::max(0, (mx - popup_w) / 2);
    
    WINDOW* pop_win = newwin(popup_h, popup_w, popup_y, popup_x);
    if (pop_win) {
        wbkgdset(pop_win, COLOR_PAIR(2));
        werase(pop_win);
        box(pop_win, 0, 0);
        
        wattron(pop_win, COLOR_PAIR(4) | A_BOLD);
        mvwprintw(pop_win, 1, (popup_w - 18) / 2, "STAT ALLOCATION");
        wattroff(pop_win, COLOR_PAIR(4) | A_BOLD);
        
        mvwprintw(pop_win, 3, 2, "Stat Points Available: %d", p->get_stat_points());
        
        int val_col = popup_w - 10;
        if (val_col < 30) val_col = 30;
        for (size_t i = 0; i < stat_names.size(); ++i) {
            if (i == current_selection) {
                wattron(pop_win, COLOR_PAIR(1) | A_REVERSE);
            }
            std::string sn = stat_names[i];
            int sn_max = val_col - 6;
            if ((int)sn.length() > sn_max) sn = sn.substr(0, sn_max - 1) + "~";
            mvwprintw(pop_win, 5 + i, 4, "%s", sn.c_str());
            
            if (i == current_selection) {
                wattroff(pop_win, COLOR_PAIR(1) | A_REVERSE);
            }
            
            // Render current values
            int val = 0;
            if (i == 0) val = p->get_str();
            else if (i == 1) val = p->get_cons();
            else if (i == 2) val = p->get_agi();
            else if (i == 3) val = p->get_intl();
            else if (i == 4) val = p->get_wis();
            mvwprintw(pop_win, 5 + i, val_col, "%d", val);
        }
        
        wnoutrefresh(pop_win);
        delwin(pop_win);
    }
}

void StatAllocationState::on_exit() {
    engine->get_layout().resize(); // Redraw underlying layout
}
