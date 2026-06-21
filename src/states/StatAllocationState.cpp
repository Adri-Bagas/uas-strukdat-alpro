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
    if (ch == -1) return;
    
    Player* p = engine->get_player_manager().get_player();
    if (!p) return;

    if (ch == KEY_UP || ch == 'w') {
        current_selection--;
        if (current_selection < 0) current_selection = stat_names.size() - 1;
    } else if (ch == KEY_DOWN || ch == 's') {
        current_selection++;
        if (current_selection >= stat_names.size()) current_selection = 0;
    } else if (ch == 10) { // Enter
        if (current_selection == 5) { // Done
            engine->pop_state();
            return;
        }
        
        if (p->get_stat_points() > 0) {
            p->add_stat_points(-1);
            if (current_selection == 0) p->set_str(p->get_str() + 1);
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

    int popup_w = 55;
    int popup_h = stat_names.size() + 6;
    int popup_y = (LINES - popup_h) / 2;
    int popup_x = (COLS - popup_w) / 2;
    
    WINDOW* pop_win = newwin(popup_h, popup_w, popup_y, popup_x);
    if (pop_win) {
        wbkgdset(pop_win, COLOR_PAIR(2));
        werase(pop_win);
        box(pop_win, 0, 0);
        
        wattron(pop_win, COLOR_PAIR(4) | A_BOLD);
        mvwprintw(pop_win, 1, (popup_w - 18) / 2, "STAT ALLOCATION");
        wattroff(pop_win, COLOR_PAIR(4) | A_BOLD);
        
        mvwprintw(pop_win, 3, 2, "Stat Points Available: %d", p->get_stat_points());
        
        for (size_t i = 0; i < stat_names.size(); ++i) {
            if (i == current_selection) {
                wattron(pop_win, COLOR_PAIR(1) | A_REVERSE);
            }
            mvwprintw(pop_win, 5 + i, 4, "%-35s", stat_names[i].c_str());
            
            if (i == current_selection) {
                wattroff(pop_win, COLOR_PAIR(1) | A_REVERSE);
            }
            
            // Render current values
            if (i == 0) mvwprintw(pop_win, 5 + i, 45, "%d", p->get_str());
            else if (i == 1) mvwprintw(pop_win, 5 + i, 45, "%d", p->get_cons());
            else if (i == 2) mvwprintw(pop_win, 5 + i, 45, "%d", p->get_agi());
            else if (i == 3) mvwprintw(pop_win, 5 + i, 45, "%d", p->get_intl());
            else if (i == 4) mvwprintw(pop_win, 5 + i, 45, "%d", p->get_wis());
        }
        
        wnoutrefresh(pop_win);
        delwin(pop_win);
    }
}

void StatAllocationState::on_exit() {
    engine->get_layout().resize(); // Redraw underlying layout
}
