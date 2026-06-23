#include "GameOverState.hpp"
#include "../GameEngine.hpp"
#include "StartState.hpp"
#include <ncurses.h>

GameOverState::GameOverState(GameEngine* eng, const std::string& title, const std::string& subtitle)
    : GameState(eng), title(title), subtitle(subtitle) {}

void GameOverState::handle_input(int ch) {
    if (ch == KEY_UP || ch == 'w') {
        selection_index--;
        if (selection_index < 0) selection_index = options.size() - 1;
    } else if (ch == KEY_DOWN || ch == 's') {
        selection_index++;
        if (selection_index >= (int)options.size()) selection_index = 0;
    } else if (ch == '\n' || ch == ' ') {
        if (selection_index == 0) {
            engine->change_state(new StartState(engine));
        } else if (selection_index == 1) {
            engine->quit();
        }
    }
}

void GameOverState::update() {
}

void GameOverState::render() {
    erase();

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    std::vector<std::string> art;
    if (title == "GAME OVER") {
        art = {
            "  ____    _    __  __ _____    ___     _____ ____  ",
            " / ___|  / \\  |  \\/  | ____|  / _ \\   | ____|  _ \\ ",
            "| |  _  / _ \\ | |\\/| |  _|   | | | |  |  _| | |_) |",
            "| |_| |/ ___ \\| |  | | |___  | |_| |  | |___|  _ < ",
            " \\____/_/   \\_\\_|  |_|_____|  \\___/   |_____|_| \\_\\"
        };
    } else {
        art = {
            " _____ _   _ _____   _____ _   _ ____  ",
            "|_   _| | | | ____| | ____| \\ | |  _ \\ ",
            "  | | | |_| |  _|   |  _| |  \\| | | | |",
            "  | | |  _  | |___  | |___| |\\  | |_| |",
            "  |_| |_| |_|_____| |_____|_| \\_|____/ "
        };
    }

    int start_y = max_y / 2 - 6;
    if (start_y < 0) start_y = 0;

    int color_pair = (title == "GAME OVER") ? 5 : 4;
    attron(COLOR_PAIR(color_pair) | A_BOLD);
    for (size_t i = 0; i < art.size(); ++i) {
        int start_x = (max_x - art[i].length()) / 2;
        if (start_x < 0) start_x = 0;
        mvprintw(start_y + i, start_x, "%s", art[i].c_str());
    }
    attroff(COLOR_PAIR(color_pair) | A_BOLD);

    int sub_y = start_y + art.size() + 2;
    int sub_x = (max_x - subtitle.length()) / 2;
    if (sub_x < 0) sub_x = 0;
    mvprintw(sub_y, sub_x, "%s", subtitle.c_str());

    int menu_y = sub_y + 3;
    for (size_t i = 0; i < options.size(); ++i) {
        std::string text = (selection_index == (int)i) ? "> " + options[i] + " <" : "  " + options[i] + "  ";
        int opt_x = (max_x - text.length()) / 2;
        if (opt_x < 0) opt_x = 0;

        if (selection_index == (int)i) {
            attron(A_REVERSE | A_BOLD | COLOR_PAIR(4));
            mvprintw(menu_y + i, opt_x, "%s", text.c_str());
            attroff(A_REVERSE | A_BOLD | COLOR_PAIR(4));
        } else {
            mvprintw(menu_y + i, opt_x, "%s", text.c_str());
        }
    }

    wnoutrefresh(stdscr);
}
