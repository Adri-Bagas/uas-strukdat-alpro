#include "StartState.hpp"
#include "TownState.hpp"
#include "BattleState.hpp"
#include "DungeonState.hpp"
#include "../GameEngine.hpp"
#include <ncurses.h>

StartState::StartState(GameEngine* eng) : GameState(eng) {}

void StartState::on_enter() {
    selection_index = 0;
}

void StartState::handle_input(int ch) {
    if (ch == KEY_RESIZE) {
        clear();
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
            engine->get_places().set_current_place("kandang_kuda");
            engine->push_state(new TownState(engine));
        } else if (selection_index == 1) { // Load
            engine->get_dialogs().queue_popup("Fitur Load belum diimplementasikan.");
        } else if (selection_index == 2) { // Test Battle
            engine->push_state(new BattleState(engine, "test_group"));
        } else if (selection_index == 3) { // Test Dungeon
            engine->push_state(new DungeonState(engine));
        } else if (selection_index == 4) { // Settings
            engine->get_dialogs().queue_popup("Fitur Settings belum diimplementasikan.");
        } else if (selection_index == 5) { // Exit
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
    engine->get_start_layout().draw(selection_index, menu_options);
}
