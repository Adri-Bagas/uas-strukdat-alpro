#include <fstream>
#include "../states/StartState.hpp"
#include "../states/GameOverState.hpp"
#include "../GameEngine.hpp"
#include "../utils/Logger.hpp"
#include "./Actions.hpp"
#include "../states/ShopState.hpp"
#include "../states/BattleState.hpp"
#include "../managers/SaveManager.hpp"
#include "../utils/components/InputPopup.hpp"
#include "../utils/components/MenuPopup.hpp"
#include <sstream>

Action::Action(GameEngine* eng) : engine(eng) {
    // Movement Actions
    register_action("move_to_stable", [this](const std::string&) {
        engine->get_places().set_current_place("kandang_kuda");
        if (engine->get_current_state()) engine->get_current_state()->on_enter();
    });
    register_action("move_to_attic", [this](const std::string&) {
        engine->get_places().set_current_place("kamar_loteng");
        if (engine->get_current_state()) engine->get_current_state()->on_enter();
    });
    register_action("move_to_town_hall", [this](const std::string&) {
        engine->get_places().set_current_place("balai_kota");
        if (engine->get_current_state()) engine->get_current_state()->on_enter();
    });
    register_action("move_to_town_center", [this](const std::string&) {
        engine->get_places().set_current_place("alun_alun");
        if (engine->get_current_state()) engine->get_current_state()->on_enter();
    });
    register_action("move_to_main_street", [this](const std::string&) {
        engine->get_places().set_current_place("jalanan_utama_kota");
        if (engine->get_current_state()) engine->get_current_state()->on_enter();
    });
    register_action("move_to_arthur_bar", [this](const std::string&) {
        engine->get_places().set_current_place("kedai_usang");
        if (engine->get_current_state()) engine->get_current_state()->on_enter();
    });
    register_action("move_to_outskirts", [this](const std::string&) {
        engine->get_places().set_current_place("permukiman_kumuh");
        if (engine->get_current_state()) engine->get_current_state()->on_enter();
    });
    register_action("move_to_pasar_gelap", [this](const std::string&) {
        engine->get_places().set_current_place("permukiman_kumuh"); // Placeholder
        if (engine->get_current_state()) engine->get_current_state()->on_enter();
    });
    register_action("move_to_menara_tua", [this](const std::string&) {
        engine->get_places().set_current_place("menara_tua");
        if (engine->get_current_state()) engine->get_current_state()->on_enter();
    });
    register_action("move_to_alun_alun", [this](const std::string&) {
        engine->get_places().set_current_place("alun_alun");
        if (engine->get_current_state()) engine->get_current_state()->on_enter();
    });
    register_action("move_to_biara", [this](const std::string&) {
        engine->get_places().set_current_place("biara");
    });
    register_action("move_to", [this](const std::string& arg) {
        if (!arg.empty()) {
            engine->get_places().set_current_place(arg);
        }
    });

    // Time Actions
    register_action("set_time_pagi", [this](const std::string&) {
        engine->get_calendar().setDayTime(MORNING);
    });
    register_action("set_time_siang", [this](const std::string&) {
        engine->get_calendar().setDayTime(AFTERNOON);
    });
    register_action("set_time_malam", [this](const std::string&) {
        engine->get_calendar().setDayTime(EVENING);
    });
    register_action("set_time_siang", [this](const std::string&) {
        engine->get_calendar().setDayTime(AFTERNOON);
    });
    register_action("check_zona_kuning", [this](const std::string&) {
        engine->get_dialogs().queue_popup("Peringatan: Memasuki Zona Kuning!");
    });

    // Generic Activity Actions
    register_action("add_gold", [this](const std::string& arg) {
        if (!arg.empty()) {
            int amount = std::stoi(arg);
            Player* p = engine->get_player_manager().get_player();
            if (p) p->add_gold(amount);
        }
    });

    register_action("advance_time", [this](const std::string& arg) {
        if (!arg.empty()) {
            int amount = std::stoi(arg);
            for (int i = 0; i < amount; ++i) {
                engine->get_calendar().advanceTime(false);
            }
        }
    });

    // Stats Actions
    register_action("heal_hp", [this](const std::string& arg) {
        if (!arg.empty()) {
            int amount = std::stoi(arg);
            Player* p = engine->get_player_manager().get_player();
            if (p) {
                p->heal_hp(amount);
                Utils::Logger::log("Action: Healed player for " + std::to_string(amount) + " HP");
            }
        }
    });

    register_action("restore_mp", [this](const std::string& arg) {
        if (!arg.empty()) {
            int amount = std::stoi(arg);
            Player* p = engine->get_player_manager().get_player();
            if (p) {
                p->restore_mp(amount);
                Utils::Logger::log("Action: Restored player for " + std::to_string(amount) + " MP");
            }
        }
    });

    register_action("heal_full", [this](const std::string&) {
        Player* p = engine->get_player_manager().get_player();
        if (p) {
            p->heal_hp(p->get_max_hp());
            p->restore_mp(p->get_max_mp());
            Utils::Logger::log("Action: Healed player to full HP and MP");
        }
    });

    register_action("advance_day", [this](const std::string&) {
        engine->get_calendar().advanceDate();
        engine->get_player_manager().get_player()->set_var("day", engine->get_calendar().getDay());
        if (engine->get_current_state()) engine->get_current_state()->on_enter();
        engine->get_log_manager().add_log(engine->get_calendar().getTimeString(), "A new day has begun.");
        Utils::Logger::log("Action: Advanced to the next day");
    });

    // Save & Sleep Prompts
    register_action("sleep_prompt", [this](const std::string&) {
        const DialogScene* scene = engine->get_db().get_dialog_scene("scene_sleep_prompt");
        if (scene) engine->get_dialogs().start_scene(*scene, engine);
    });
    register_action("save_game", [this](const std::string&) {
        auto saves = SaveManager::get_save_files();
        std::vector<std::string> options;
        options.push_back("<Buat Save Baru>");
        for (const auto& s : saves) {
            options.push_back(s.filename + " (" + s.player_name + " Lv." + std::to_string(s.level) + " " + s.phase_name + ")");
        }
        options.push_back("Batal");

        auto popup = std::make_unique<Utils::MenuPopup>(
            "Pilih Slot Save:",
            options,
            [this, saves](int choice) {
                if (choice < 0 || choice == (int)saves.size() + 1) return; // Batal
                if (choice == 0) {
                    // Buat Save Baru
                    auto input_popup = std::make_unique<Utils::InputPopup>(
                        "Masukkan nama save file (tanpa spasi/simbol):",
                        [this](const std::string& name) {
                            if (SaveManager::save_game(engine, name)) {
                                engine->get_dialogs().queue_popup("Game berhasil disimpan ke " + name + "!");
                            } else {
                                engine->get_dialogs().queue_popup("Gagal menyimpan game!");
                            }
                        }
                    );
                    engine->show_popup(std::move(input_popup));
                } else {
                    // Overwrite existing save
                    int save_idx = choice - 1;
                    if (SaveManager::save_game(engine, saves[save_idx].filename)) {
                        engine->get_dialogs().queue_popup("Game berhasil ditimpa ke " + saves[save_idx].filename + "!");
                    } else {
                        engine->get_dialogs().queue_popup("Gagal menyimpan game!");
                    }
                }
            }
        );
        engine->show_popup(std::move(popup));
    });

    register_action("mark_current_place_entered", [this](const std::string&) {
        Place* cur = engine->get_places().get_current_place();
        if (cur) {
            cur->set_has_entered(true);
        }
    });

    register_action("return_to_main_menu", [this](const std::string&) {
        engine->change_state(new StartState(engine));
    });

    register_action("go_to_game_over", [this](const std::string& arg) {
        if (arg == "good") {
            engine->change_state(new GameOverState(engine, "THE END", "Kamu berhasil membuktikan bahwa kamu tidak bersalah!"));
        } else {
            engine->change_state(new GameOverState(engine, "GAME OVER", "Kamu gagal membersihkan namamu dalam 14 hari."));
        }
    });

    // Party Actions
    register_action("learn_magic", [this](const std::string& arg) {
        Player* p = engine->get_player_manager().get_player();
        if (!p) return;
        std::istringstream iss(arg);
        std::string magic_id, item_id;
        iss >> magic_id >> item_id;

        for (const auto& mag : p->get_magics()) {
            if (mag.id == magic_id) {
                engine->get_dialogs().queue_popup("Kamu sudah mengetahui sihir ini!");
                if (!item_id.empty()) p->add_item(item_id, 1); // Refund
                return;
            }
        }
        
        if (magic_id == "inferno") {
            Magic m; m.id = "inferno"; m.name = "Inferno Blast"; m.type = MagicType::ATTACKING;
            m.power = 40; m.mana_cost = 15; m.elem = Element::FIRE; m.range = TargetRange::AOE;
            p->add_magic(m);
            engine->get_dialogs().queue_popup("Mempelajari sihir: Inferno Blast!");
        } else if (magic_id == "heal_greater") {
            Magic m; m.id = "heal_greater"; m.name = "Greater Heal"; m.type = MagicType::HEALING;
            m.power = 50; m.mana_cost = 10; m.elem = Element::LIGHT; m.range = TargetRange::DIRECT;
            p->add_magic(m);
            engine->get_dialogs().queue_popup("Mempelajari sihir: Greater Heal!");
        } else if (magic_id == "water_splash") {
            Magic m; m.id = "water_splash"; m.name = "Water Splash"; m.type = MagicType::ATTACKING;
            m.power = 20; m.mana_cost = 5; m.elem = Element::WATER; m.range = TargetRange::DIRECT;
            p->add_magic(m);
            engine->get_dialogs().queue_popup("Mempelajari sihir: Water Splash!");
        } else if (magic_id == "earth_spike") {
            Magic m; m.id = "earth_spike"; m.name = "Earth Spike"; m.type = MagicType::ATTACKING;
            m.power = 20; m.mana_cost = 5; m.elem = Element::EARTH; m.range = TargetRange::DIRECT;
            p->add_magic(m);
            engine->get_dialogs().queue_popup("Mempelajari sihir: Earth Spike!");
        } else if (magic_id == "wind_blade") {
            Magic m; m.id = "wind_blade"; m.name = "Wind Blade"; m.type = MagicType::ATTACKING;
            m.power = 20; m.mana_cost = 5; m.elem = Element::WIND; m.range = TargetRange::DIRECT;
            p->add_magic(m);
            engine->get_dialogs().queue_popup("Mempelajari sihir: Wind Blade!");
        } else if (magic_id == "fire_bolt") {
            Magic m; m.id = "fire_bolt"; m.name = "Fire Bolt"; m.type = MagicType::ATTACKING;
            m.power = 20; m.mana_cost = 5; m.elem = Element::FIRE; m.range = TargetRange::DIRECT;
            p->add_magic(m);
            engine->get_dialogs().queue_popup("Mempelajari sihir: Fire Bolt!");
        } else if (magic_id == "minor_heal") {
            Magic m; m.id = "minor_heal"; m.name = "Minor Heal"; m.type = MagicType::HEALING;
            m.power = 25; m.mana_cost = 5; m.elem = Element::LIGHT; m.range = TargetRange::DIRECT;
            p->add_magic(m);
            engine->get_dialogs().queue_popup("Mempelajari sihir: Minor Heal!");
        } else if (magic_id == "water_wave") {
            Magic m; m.id = "water_wave"; m.name = "Water Wave"; m.type = MagicType::ATTACKING;
            m.power = 18; m.mana_cost = 10; m.elem = Element::WATER; m.range = TargetRange::REACH;
            p->add_magic(m);
            engine->get_dialogs().queue_popup("Mempelajari sihir: Water Wave!");
        } else if (magic_id == "earth_tremor") {
            Magic m; m.id = "earth_tremor"; m.name = "Earth Tremor"; m.type = MagicType::ATTACKING;
            m.power = 18; m.mana_cost = 10; m.elem = Element::EARTH; m.range = TargetRange::REACH;
            p->add_magic(m);
            engine->get_dialogs().queue_popup("Mempelajari sihir: Earth Tremor!");
        } else if (magic_id == "wind_gust") {
            Magic m; m.id = "wind_gust"; m.name = "Wind Gust"; m.type = MagicType::ATTACKING;
            m.power = 18; m.mana_cost = 10; m.elem = Element::WIND; m.range = TargetRange::REACH;
            p->add_magic(m);
            engine->get_dialogs().queue_popup("Mempelajari sihir: Wind Gust!");
        } else if (magic_id == "fire_wall") {
            Magic m; m.id = "fire_wall"; m.name = "Fire Wall"; m.type = MagicType::ATTACKING;
            m.power = 18; m.mana_cost = 10; m.elem = Element::FIRE; m.range = TargetRange::REACH;
            p->add_magic(m);
            engine->get_dialogs().queue_popup("Mempelajari sihir: Fire Wall!");
        }
    });

    register_action("learn_special", [this](const std::string& arg) {
        Player* p = engine->get_player_manager().get_player();
        if (!p) return;
        std::istringstream iss(arg);
        std::string move_id, item_id;
        iss >> move_id >> item_id;

        if (p->has_special() && p->get_special_move().id == move_id) {
            engine->get_dialogs().queue_popup("Kamu sudah mempelajari Special Move ini!");
            if (!item_id.empty()) p->add_item(item_id, 1); // Refund
            return;
        }

        if (move_id == "meteor") {
            SpecialMove sm; sm.id = "meteor"; sm.name = "Meteor Strike"; sm.power = 80;
            sm.max_uses_per_day = 1; sm.current_uses = 1; sm.elem = Element::FIRE; sm.range = TargetRange::AOE;
            p->set_special_move(sm);
            engine->get_dialogs().queue_popup("Mempelajari Special Move: Meteor Strike!");
        }
    });

    register_action("add_party_member", [this](const std::string& npc_id) {
        if (!npc_id.empty()) {
            const NPC* npc = engine->get_db().get_npc(npc_id);
            if (npc) {
                if (engine->get_player_manager().add_ally(*npc)) {
                    Player* p = engine->get_player_manager().get_player();
                    if (p) p->set_var("party_" + npc_id, 1);
                    Utils::Logger::log("Action: Added party member " + npc_id);
                }
            }
        }
    });

    register_action("remove_party_member", [this](const std::string& npc_id) {
        if (!npc_id.empty()) {
            engine->get_player_manager().remove_ally(npc_id);
            Player* p = engine->get_player_manager().get_player();
            if (p) p->set_var("party_" + npc_id, 0);
            Utils::Logger::log("Action: Removed party member " + npc_id);
        }
    });

    // Inventory Actions
    register_action("give_item", [this](const std::string& arg) {
        if (!arg.empty()) {
            std::stringstream ss(arg);
            std::string item_id;
            int amount = 1; // Default amount
            ss >> item_id;
            if (!ss.eof()) {
                ss >> amount;
            }
            Player* p = engine->get_player_manager().get_player();
            if (p) {
                p->add_item(item_id, amount);
                const Item* item = engine->get_db().get_item(item_id);
                if (item) {
                    std::string msg = "Obtained " + std::to_string(amount) + "x " + item->get_name() + ".";
                    engine->get_log_manager().add_log(engine->get_calendar().getTimeString(), msg);
                }
            }
        }
    });

    register_action("remove_item", [this](const std::string& arg) {
        if (!arg.empty()) {
            std::stringstream ss(arg);
            std::string item_id;
            int amount = 1;
            ss >> item_id;
            if (!ss.eof()) {
                ss >> amount;
            }
            Player* p = engine->get_player_manager().get_player();
            if (p) p->remove_item(item_id, amount);
        }
    });

    // Equipment Actions
    register_action("equip_item", [this](const std::string& arg) {
        if (!arg.empty()) {
            const Item* item = engine->get_db().get_item(arg);
            Player* p = engine->get_player_manager().get_player();
            if (p && item) {
                p->equip(item);
            }
        }
    });

    register_action("unequip_item", [this](const std::string& arg) {
        if (!arg.empty()) {
            Player* p = engine->get_player_manager().get_player();
            // arg is expected to be the slot name, e.g., "weapon"
            if (p) p->unequip(arg);
        }
    });

    // Variable Actions
    register_action("set_var", [this](const std::string& arg) {
        std::stringstream ss(arg);
        std::string key;
        int val;
        if (ss >> key >> val) {
            Player* p = engine->get_player_manager().get_player();
            if (p) {
                if (key == "str") p->set_str(val);
                else if (key == "cons") p->set_cons(val);
                else if (key == "agi") p->set_agi(val);
                else if (key == "intl") p->set_intl(val);
                else if (key == "wis") p->set_wis(val);
                else p->set_var(key, val);
            }
        }
    });

    register_action("add_var", [this](const std::string& arg) {
        std::stringstream ss(arg);
        std::string key;
        int amount;
        if (ss >> key >> amount) {
            Player* p = engine->get_player_manager().get_player();
            if (p) {
                if (key == "str") p->set_str(p->get_str() + amount);
                else if (key == "cons") p->set_cons(p->get_cons() + amount);
                else if (key == "agi") p->set_agi(p->get_agi() + amount);
                else if (key == "intl") p->set_intl(p->get_intl() + amount);
                else if (key == "wis") p->set_wis(p->get_wis() + amount);
                else p->add_var(key, amount);
            }
        }
    });

    // Quest Actions
    register_action("accept_quest", [this](const std::string& arg) {
        if (!arg.empty()) {
            Quest* q = engine->get_quests().get_quest(arg);
            if (!q || q->get_state() == QuestState::IN_PROGRESS) return;

            bool has_active = false;
            for (const auto& pair : engine->get_quests().get_all_quests()) {
                if (pair.second->get_state() == QuestState::IN_PROGRESS) {
                    has_active = true;
                    break;
                }
            }

            if (has_active) {
                engine->get_dialogs().queue_popup("Gagal: Selesaikan misi aktifmu terlebih dahulu!");
                return;
            }

            if (q && (q->get_state() == QuestState::AVAILABLE || q->get_state() == QuestState::LOCKED)) {
                q->set_state(QuestState::IN_PROGRESS);
                engine->get_log_manager().add_log(engine->get_calendar().getTimeString(), "Started Quest: " + q->get_name());
                engine->get_dialogs().queue_popup("Misi Baru: " + q->get_name());
                Utils::Logger::log("Action: Quest accepted: " + arg);
            }
        }
    });

    register_action("complete_quest", [this](const std::string& arg) {
        if (!arg.empty()) {
            Quest* q = engine->get_quests().get_quest(arg);
            if (q && (q->get_state() == QuestState::IN_PROGRESS || q->get_state() == QuestState::READY_TO_TURN_IN)) {
                q->set_state(QuestState::COMPLETED);
                for (const auto& action : q->get_on_complete()) {
                    engine->get_actions().execute(action);
                }
                engine->get_log_manager().add_log(engine->get_calendar().getTimeString(), "Completed Quest: " + q->get_name());
                Utils::Logger::log("Action: Quest completed: " + arg);
            }
        }
    });

    register_action("reveal_name", [this](const std::string& arg) {
        if (!arg.empty()) {
            for (auto* npc_const : engine->get_db().get_all_npcs()) {
                if (npc_const->get_id() == arg) {
                    const_cast<NPC*>(npc_const)->reveal();
                    engine->get_log_manager().add_log(engine->get_calendar().getTimeString(), "Met " + npc_const->get_name() + ".");
                    Utils::Logger::log("Action: Revealed NPC identity: " + arg);
                    break;
                }
            }
        }
    });

    register_action("set_next_dialog", [this](const std::string& arg) {
        if(!arg.empty()) {
            engine->get_dialogs().set_next_scene(arg);
        }
    });

  
    register_action("play_music", [this](const std::string& arg) {
        if (!arg.empty()) {
            engine->get_music_manager().playMusic(arg);
        }
    });

    register_action("stop_music", [this](const std::string&) {
        engine->get_music_manager().stopMusic();
    });

    register_action("play_sfx", [this](const std::string& arg) {
        if (!arg.empty()) {
            engine->get_music_manager().playSfx(arg);
        }
      });
  
    register_action("open_quest_menu", [this](const std::string&) {
        Player* p = engine->get_player_manager().get_player();
        if (p) p->set_var("open_quest_menu", 1);
    });

    register_action("open_shop", [this](const std::string& arg) {
        if (!arg.empty()) {
            engine->push_state(new ShopState(engine, arg));
            Utils::Logger::log("Action: Opened shop " + arg);
        }
    });

    register_action("start_battle", [this](const std::string& arg) {
        if (!arg.empty()) {
            std::string enemies_list;
            std::string victory_action;
            size_t quote_start = arg.find('"');
            if (quote_start != std::string::npos) {
                size_t quote_end = arg.find('"', quote_start + 1);
                if (quote_end != std::string::npos) {
                    victory_action = arg.substr(quote_start + 1, quote_end - quote_start - 1);
                }
                enemies_list = arg.substr(0, quote_start);
            } else {
                enemies_list = arg;
            }
            // Trim trailing whitespace from enemies_list
            size_t last_not_space = enemies_list.find_last_not_of(" \t\r\n");
            if (last_not_space != std::string::npos) {
                enemies_list.erase(last_not_space + 1);
            } else {
                enemies_list.clear();
            }
            
            engine->push_state(new BattleState(engine, enemies_list, victory_action));
            Utils::Logger::log("Action: Started battle with enemies: " + enemies_list);
        }
    });

    register_action("start_story_battle", [this](const std::string& arg) {
        if (!arg.empty()) {
            std::string enemies_list;
            std::string victory_action;
            size_t quote_start = arg.find('"');
            if (quote_start != std::string::npos) {
                size_t quote_end = arg.find('"', quote_start + 1);
                if (quote_end != std::string::npos) {
                    victory_action = arg.substr(quote_start + 1, quote_end - quote_start - 1);
                }
                enemies_list = arg.substr(0, quote_start);
            } else {
                enemies_list = arg;
            }
            // Trim trailing whitespace from enemies_list
            size_t last_not_space = enemies_list.find_last_not_of(" \t\r\n");
            if (last_not_space != std::string::npos) {
                enemies_list.erase(last_not_space + 1);
            } else {
                enemies_list.clear();
            }
            
            engine->push_state(new BattleState(engine, enemies_list, victory_action, false));
            Utils::Logger::log("Action: Started story battle with enemies: " + enemies_list);
        }
    });
}


void Action::register_action(const std::string& key, std::function<void(const std::string&)> func) {
    actions[key] = func;
}

void Action::execute(const std::string& command_line) {
    if (command_line.empty()) return;

    std::stringstream ss(command_line);
    std::string cmd, arg;
    ss >> cmd;
    std::getline(ss, arg);
    
    // Trim leading whitespace from arg
    if (!arg.empty() && arg[0] == ' ') {
        arg.erase(0, 1);
    }

    auto it = actions.find(cmd);
    if (it != actions.end()) {
        it->second(arg);
    } else {
        Utils::Logger::log("Action ERROR: Unknown action command '" + cmd + "'");
    }
}