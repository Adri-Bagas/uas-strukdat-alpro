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
    // Putar musik menu saat awal mulai program
    engine->get_music_manager().playMusic("exploration.mp3");
}

void StartState::on_resume() {
    // Putar kembali musik menu jika kembali dari test battle/dungeon
    engine->get_music_manager().playMusic("exploration.mp3");
    // Reset player back to normal
    engine->get_player_manager().init_player("hero", "Nirva Hero");
}

void StartState::handle_input(int ch) {
    if (ch == KEY_RESIZE) {
        clear();
        return;
    }

    if (ch == KEY_UP || ch == 'w') {
        selection_index--;
        if (selection_index < 0) selection_index = menu_options.size() - 1;
        engine->get_music_manager().playSfx("select_001.mp3");
    } else if (ch == KEY_DOWN || ch == 's') {
        selection_index++;
        if (selection_index >= (int)menu_options.size()) selection_index = 0;
        engine->get_music_manager().playSfx("select_001.mp3");
    } else if (ch == '\n' || ch == ' ') {
        if (selection_index == 0) { // Start New
            engine->get_music_manager().stopMusic(); // Matikan musik menu
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
        
            // Make Arthur and Silas more powerful
            auto party = engine->get_player_manager().get_party_slots();
            for (int i = 1; i < 4; ++i) {
                if (party[i]) {
                    party[i]->set_str(100);
                    party[i]->set_cons(100);
                    party[i]->set_agi(100);
                    party[i]->set_intl(100);
                    party[i]->set_wis(100);
                    party[i]->set_max_hp(1000);
                    party[i]->heal_hp(1000);
                    party[i]->set_max_mp(500);
                    party[i]->restore_mp(500);
                }
            }

            Player* p = engine->get_player_manager().get_player();
            p->set_str(150);
            p->set_cons(150);
            p->set_agi(150);
            p->set_intl(150);
            p->set_wis(150);
            p->set_max_hp(1500);
            p->heal_hp(1500);
            p->set_max_mp(800);
            p->restore_mp(800);
            
            // Add all elemental and healing magic
            Magic fire;
            fire.id = "fire_1";
            fire.name = "Fireball";
            fire.type = MagicType::ATTACKING;
            fire.mana_cost = 15;
            fire.power = 40;
            fire.elem = Element::FIRE;
            fire.range = TargetRange::REACH;
            p->add_magic(fire);

            Magic water;
            water.id = "water_1";
            water.name = "Water Gun";
            water.type = MagicType::ATTACKING;
            water.mana_cost = 15;
            water.power = 40;
            water.elem = Element::WATER;
            water.range = TargetRange::REACH;
            p->add_magic(water);

            Magic earth;
            earth.id = "earth_1";
            earth.name = "Earth Spike";
            earth.type = MagicType::ATTACKING;
            earth.mana_cost = 15;
            earth.power = 40;
            earth.elem = Element::EARTH;
            earth.range = TargetRange::REACH;
            p->add_magic(earth);

            Magic wind;
            wind.id = "wind_1";
            wind.name = "Wind Slash";
            wind.type = MagicType::ATTACKING;
            wind.mana_cost = 15;
            wind.power = 40;
            wind.elem = Element::WIND;
            wind.range = TargetRange::REACH;
            p->add_magic(wind);

            Magic light;
            light.id = "light_1";
            light.name = "Holy Light";
            light.type = MagicType::ATTACKING;
            light.mana_cost = 20;
            light.power = 50;
            light.elem = Element::LIGHT;
            light.range = TargetRange::REACH;
            p->add_magic(light);
        
            Magic heal;
            heal.id = "heal_1";
            heal.name = "Minor Heal";
            heal.type = MagicType::HEALING;
            heal.mana_cost = 10;
            heal.power = 30;
            heal.elem = Element::NONE;
            heal.range = TargetRange::REACH;
            p->add_magic(heal);

            Magic major_heal;
            major_heal.id = "heal_2";
            major_heal.name = "Holy Heal";
            major_heal.type = MagicType::HEALING;
            major_heal.mana_cost = 25;
            major_heal.power = 100;
            major_heal.elem = Element::LIGHT;
            major_heal.range = TargetRange::REACH;
            p->add_magic(major_heal);
        
            SpecialMove slash;
            slash.id = "slash_1";
            slash.name = "Cross Slash";
            slash.max_uses_per_day = 3;
            slash.power = 80;
            p->set_special_move(slash);

            // Equip MC with a wooden bow
            const Item* bow = engine->get_db().get_item("wooden_bow");
            if (bow) {
                p->add_item("wooden_bow", 1);
                p->equip(bow);
            }

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