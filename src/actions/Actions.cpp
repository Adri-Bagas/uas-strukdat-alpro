#include "../GameEngine.hpp"
#include "../utils/Logger.hpp"
#include "./Actions.hpp"
#include <sstream>

Action::Action(GameEngine* eng) : engine(eng) {
    // Movement Actions
    register_action("move_to_stable", [this](const std::string&) {
        engine->get_places().set_current_place("kandang_kuda");
    });
    register_action("move_to_attic", [this](const std::string&) {
        engine->get_places().set_current_place("kamar_loteng");
    });
    register_action("move_to_town_hall", [this](const std::string&) {
        engine->get_places().set_current_place("balai_kota");
    });
    register_action("move_to_town_center", [this](const std::string&) {
        engine->get_places().set_current_place("alun_alun");
    });
    register_action("move_to_main_street", [this](const std::string&) {
        engine->get_places().set_current_place("jalanan_utama_kota");
    });
    register_action("move_to_arthur_bar", [this](const std::string&) {
        engine->get_places().set_current_place("kedai_usang");
    });

    // Time Actions
    register_action("set_time_morning", [this](const std::string&) {
        engine->get_calendar().setDayTime(MORNING);
    });
    register_action("set_time_afternoon", [this](const std::string&) {
        engine->get_calendar().setDayTime(AFTERNOON);
    });
    register_action("set_time_evening", [this](const std::string&) {
        engine->get_calendar().setDayTime(EVENING);
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
                Logger::log("Action: Healed player for " + std::to_string(amount) + " HP");
            }
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
            if (p) p->add_item(item_id, amount);
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
                Logger::log("Action: Quest accepted: " + arg);
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
                Logger::log("Action: Quest completed: " + arg);
            }
        }
    });

    register_action("reveal_name", [this](const std::string& arg) {
        if (!arg.empty()) {
            for (auto* npc_const : engine->get_db().get_all_npcs()) {
                if (npc_const->get_id() == arg) {
                    const_cast<NPC*>(npc_const)->reveal();
                    Logger::log("Action: Revealed NPC identity: " + arg);
                    break;
                }
            }
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
        Logger::log("Action ERROR: Unknown action command '" + cmd + "'");
    }
}