#include "StartMenuPage.hpp"

void StartMenuPage::draw(int selection_index, const std::vector<std::string>& menu_options) {
    erase(); // Clear the standard screen

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // ASCII Art for NIRVA
    std::vector<std::string> ascii_art = {
        "  _   _ _                  ",
        " | \\ | (_)                 ",
        " |  \\| |_ _ ____   ____ _  ",
        " | . ` | | '__\\ \\ / / _` | ",
        " | |\\  | | |   \\ V / (_| | ",
        " |_| \\_|_|_|    \\_/ \\__,_| "
    };

    int art_start_y = max_y / 2 - 8;
    if (art_start_y < 0) art_start_y = 0;

    for (size_t i = 0; i < ascii_art.size(); ++i) {
        int start_x = (max_x - ascii_art[i].length()) / 2;
        if (start_x < 0) start_x = 0;
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(art_start_y + i, start_x, "%s", ascii_art[i].c_str());
        attroff(COLOR_PAIR(4) | A_BOLD);
    }

    int menu_start_y = art_start_y + ascii_art.size() + 3;

    for (size_t i = 0; i < menu_options.size(); ++i) {
        std::string text = (selection_index == (int)i) ? "> " + menu_options[i] + " <" : "  " + menu_options[i] + "  ";
        int start_x = (max_x - text.length()) / 2;
        if (start_x < 0) start_x = 0;

        if (selection_index == (int)i) {
            attron(A_REVERSE | A_BOLD | COLOR_PAIR(4));
            mvprintw(menu_start_y + i, start_x, "%s", text.c_str());
            attroff(A_REVERSE | A_BOLD | COLOR_PAIR(4));
        } else {
            mvprintw(menu_start_y + i, start_x, "%s", text.c_str());
        }
    }

    wnoutrefresh(stdscr); // Mark stdscr for update
}
