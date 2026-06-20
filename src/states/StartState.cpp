#include "StartState.hpp"
#include "TownState.hpp"
#include "../GameEngine.hpp"
#include <ncurses.h>

StartState::StartState(GameEngine* eng) : GameState(eng) {}

void StartState::on_enter() {
    selection_index = 0;
}

void StartState::handle_input(int ch) {
    if (ch == KEY_RESIZE) {
        erase();
        return;
    }

    if (ch == KEY_UP || ch == 'w') {
        selection_index--;
        if (selection_index < 0) selection_index = menu_options.size() - 1;
    } else if (ch == KEY_DOWN || ch == 's') {
        selection_index++;
        if (selection_index >= (int)menu_options.size()) selection_index = 0;
    } else if (ch == '\n' || ch == ' ') {
        if (selection_index == 0) { // Start New
            engine->push_state(new TownState(engine));
        } else if (selection_index == 1) { // Load
            engine->get_dialogs().queue_popup("Fitur Load belum diimplementasikan.");
        } else if (selection_index == 2) { // Settings
            engine->get_dialogs().queue_popup("Fitur Settings belum diimplementasikan.");
        } else if (selection_index == 3) { // Exit
            engine->quit();
        }
    } else if (ch == 'q') {
        engine->quit();
    }
}

void StartState::update() {
    // Nothing specific to update every frame
}

void StartState::render() {
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
