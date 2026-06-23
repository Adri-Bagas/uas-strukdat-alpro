#include "StartState.hpp"
#include "TownState.hpp"
#include "BattleState.hpp"
#include "DungeonState.hpp"
#include "../GameEngine.hpp"
#include "../managers/SaveManager.hpp"
#include "../utils/components/MenuPopup.hpp"
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
            engine->get_player_manager().init_player("hero", "Nirva Hero");
            // NOTE: We should also clear quests/inventory/places here, but this at least clears the party
            engine->get_places().set_current_place("kandang_kuda");
            engine->push_state(new TownState(engine));
        } else if (selection_index == 1) { // Load
            auto saves = SaveManager::get_save_files();
            std::vector<std::string> options;
            for (const auto& s : saves) {
                options.push_back(s.filename + " (" + s.player_name + " Lv." + std::to_string(s.level) + " " + s.phase_name + ")");
            }
            options.push_back("Batal");

            auto popup = std::make_unique<Utils::MenuPopup>(
                "Pilih Save Game untuk dimuat:",
                options,
                [this, saves](int choice) {
                    if (choice < 0 || choice >= (int)saves.size()) return; // Batal
                    if (SaveManager::load_game(engine, saves[choice].filename)) {
                        engine->get_dialogs().queue_popup("Game berhasil dimuat dari " + saves[choice].filename + "!");
                        engine->get_places().set_current_place("kamar_loteng"); // Safe fallback
                        engine->push_state(new TownState(engine));
                    } else {
                        engine->get_dialogs().queue_popup("Gagal memuat save game!");
                    }
                }
            );
            engine->show_popup(std::move(popup));
        } else if (selection_index == 2) { // Test Battle
            if (auto arthur = engine->get_db().get_npc("npc_arthur")) engine->get_player_manager().add_ally(*arthur);
            if (auto silas = engine->get_db().get_npc("npc_silas")) engine->get_player_manager().add_ally(*silas);
        
            Player* p = engine->get_player_manager().get_player();
            p->set_max_mp(100);
            p->restore_mp(100);
            
            Magic fire;
            fire.id = "fire_1";
            fire.name = "Fireball";
            fire.type = MagicType::ATTACKING;
            fire.mana_cost = 15;
            fire.power = 40;
            p->add_magic(fire);
        
            Magic heal;
            heal.id = "heal_1";
            heal.name = "Minor Heal";
            heal.type = MagicType::HEALING;
            heal.mana_cost = 10;
            heal.power = 30;
            p->add_magic(heal);
        
            SpecialMove slash;
            slash.id = "slash_1";
            slash.name = "Cross Slash";
            slash.max_uses_per_day = 3;
            slash.power = 80;
            p->set_special_move(slash);

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
