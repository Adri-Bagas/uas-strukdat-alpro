#include "GameEngine.hpp"
#include "./managers/DialogManagers.hpp"
#include "./states/GameState.hpp"
#include "./views/MainPage.hpp"
#include "states/TownState.hpp"
#include <memory>
#include <ncurses.h>

void GameEngine::init() {
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    refresh();

    page.resize();

    start_color();
    init_pair(1, -1, -1);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);  // Stale Color
    init_pair(3, COLOR_BLUE, COLOR_BLACK);   // Idk
    init_pair(4, COLOR_YELLOW, COLOR_BLACK); // Active Color
    init_pair(5, COLOR_RED, COLOR_BLACK);    // Danger Color

    player_manager.init_player("hero", "Nirva Hero");

    push_state(new TownState(this));
} // Setup ncurses, set initial state

void GameEngine::push_state(GameState *new_state) {
    state_stack.push(std::unique_ptr<GameState>(new_state));
    state_stack.top()->on_enter();
}

// Removes top state, returning to the one below it.
void GameEngine::pop_state() {
    if (!state_stack.empty()) {
        state_stack.top()->on_exit();
        state_stack.pop();
    }
}

void GameEngine::run() {
    while (is_running && !state_stack.empty()) {
        int ch = getch();

        state_stack.top()->handle_input(ch);
        state_stack.top()->update();

        state_stack.top()->render();
        doupdate();

        napms(24);
    }
}

DialogManager &GameEngine::get_dialogs() {
    return dialogs;
}

MainPage &GameEngine::get_layout() {
    return page;
}

TimeCalendarManagers &GameEngine::get_calendar() {
    return calendar;
}

PlaceManagers &GameEngine::get_places() {
    return places;
}

QuestManager &GameEngine::get_quests() {
    return quests;
}

PlayerManager &GameEngine::get_player_manager() {
    return player_manager;
}

void GameEngine::quit() {
    is_running = false;
}
