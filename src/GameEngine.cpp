#include "GameEngine.hpp"
#include "./managers/DialogManagers.hpp"
#include "./states/GameState.hpp"
#include "./views/MainPage.hpp"
#include "states/TownState.hpp"
#include "states/StartState.hpp"
#include "./utils/Logger.hpp"
#include "./utils/components/ErrorPopup.hpp"
#include <memory>
#include <ncurses.h>

GameEngine::GameEngine() : actions(this) {}

void GameEngine::init() {
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    refresh();

    calendar.on_popup = [this](const std::string& msg){ dialogs.queue_popup(msg); };
    places.on_popup = [this](const std::string& msg){ dialogs.queue_popup(msg); };

    start_color();
    use_default_colors();
    init_pair(1, -1, -1);
    init_pair(2, COLOR_WHITE, -1);  // Stale Color
    init_pair(3, COLOR_BLUE, -1);   // Idk
    init_pair(4, COLOR_YELLOW, -1); // Active Color
    init_pair(5, COLOR_RED, -1);    // Danger Color

    db.load_dialogs("data/dialogs");
    db.load_places("data/places");
    db.load_items("data/items"); // Load item data dari folder JSON
    db.load_npcs("data/npcs");
    db.load_monsters("data/monsters");
    db.load_quests("data/quests");

    for (auto* p_const : db.get_all_places()) {
        places.add_place(const_cast<Place*>(p_const));
    }
    
    for (auto* npc_const : db.get_all_npcs()) {
        places.register_npc(const_cast<NPC*>(npc_const));
    }

    for (auto* q_const : db.get_all_quests()) {
        quests.add_quest(const_cast<Quest*>(q_const));
    }

    places.resolve_connections();
    places.set_current_place("kandang_kuda");

    player_manager.init_player("hero", "Nirva Hero");

    push_state(new StartState(this));
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
    Logger::log("Engine: Starting run loop.");
    try {
        while (is_running && !state_stack.empty()) {
            int ch = getch();

            if (ch == KEY_RESIZE) {
                state_stack.top()->handle_input(ch);
                if (active_popup) active_popup->handle_input(ch);
            }

            if (active_popup) {
                if (ch != KEY_RESIZE) active_popup->handle_input(ch);
                active_popup->update();
                if (active_popup->is_dismissed()) {
                    active_popup.reset();
                }
            } else if (dialogs.has_queued_popup()) {
                active_popup = std::make_unique<Popup>(dialogs.pop_popup());
            } else {
                if (ch != KEY_RESIZE) state_stack.top()->handle_input(ch);
                state_stack.top()->update();
            }

            state_stack.top()->render();
            if (active_popup) {
                active_popup->render();
            }

            doupdate();

            napms(24);
        }
    } catch (const std::exception& e) {
        Logger::log("ENGINE FATAL EXCEPTION: " + std::string(e.what()));
        ErrorPopup err("Unexpected Engine Error: " + std::string(e.what()));
        err.show_fatal();
    } catch (...) {
        Logger::log("ENGINE FATAL EXCEPTION: Unknown error");
        ErrorPopup err("Unexpected Engine Error: Unknown");
        err.show_fatal();
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

DB &GameEngine::get_db() {
    return db;
}

Action &GameEngine::get_actions() {
    return actions;
}

void GameEngine::quit() {
    is_running = false;
}
