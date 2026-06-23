#include "EndingState.hpp"
#include "../GameEngine.hpp"
#include "StartState.hpp"
#include <ncurses.h>

EndingState::EndingState(GameEngine* eng, const std::string& ending_id)
    : GameState(eng), ending_id(ending_id) {
    ending_text = engine->get_db().get_ending(ending_id);
    if (ending_text.empty()) {
        ending_text.push_back("Ending ID: " + ending_id);
        ending_text.push_back("(Teks ending tidak ditemukan di database/JSON)");
    }
}

void EndingState::handle_input(int ch) {
    if (ch == '\n' || ch == KEY_ENTER || ch == ' ') {
        engine->change_state(new StartState(engine));
    }
}

void EndingState::update() {
}

void EndingState::render() {
    erase();

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    std::string title = "=== AKHIR CERITA ===";
    int start_x = (max_x - title.length()) / 2;
    if (start_x < 0) start_x = 0;
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(max_y / 2 - (ending_text.size() / 2) - 3, start_x, "%s", title.c_str());
    attroff(COLOR_PAIR(4) | A_BOLD);

    int start_y = max_y / 2 - (ending_text.size() / 2);
    if (start_y < 2) start_y = 2;

    for (size_t i = 0; i < ending_text.size(); ++i) {
        int line_x = (max_x - ending_text[i].length()) / 2;
        if (line_x < 0) line_x = 0;
        mvprintw(start_y + i, line_x, "%s", ending_text[i].c_str());
    }

    std::string prompt = "Tekan [ENTER] atau [SPASI] untuk kembali ke Menu Utama";
    int prompt_x = (max_x - prompt.length()) / 2;
    if (prompt_x < 0) prompt_x = 0;
    attron(COLOR_PAIR(2));
    mvprintw(max_y - 3, prompt_x, "%s", prompt.c_str());
    attroff(COLOR_PAIR(2));

    wnoutrefresh(stdscr);
}
