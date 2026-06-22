#include "../GameEngine.hpp"
#include "../utils/Logger.hpp"
#include "./Actions.hpp"
#include "../states/ShopState.hpp"
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
        if (engine->get_current_state()) engine->get_current_state()->on_enter();
        engine->get_log_manager().add_log(engine->get_calendar().getTimeString(), "A new day has begun.");
        Utils::Logger::log("Action: Advanced to the next day");
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
            if (q && q->get_state() == QuestState::AVAILABLE) {
                q->set_state(QuestState::IN_PROGRESS);
                engine->get_log_manager().add_log(engine->get_calendar().getTimeString(), "Started Quest: " + q->get_name());
                Utils::Logger::log("Action: Quest accepted: " + arg);
            }
        }
    });

    register_action("complete_quest", [this](const std::string& arg) {
        if (!arg.empty()) {
            Quest* q = engine->get_quests().get_quest(arg);
            if (q && q->get_state() == QuestState::IN_PROGRESS) {
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
                    engine->get_encyclopedia().discover_entry(arg);
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