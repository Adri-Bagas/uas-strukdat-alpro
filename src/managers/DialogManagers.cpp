#include "DialogManagers.hpp"
#include "../GameEngine.hpp"
#include "../utils/Logger.hpp"
#include <sstream>

void DialogManager::add_thought(const DialogNode &data) {
    combined_log.push_back(data);
    if (combined_log.size() > max_history) {
        combined_log.erase(combined_log.begin());
    }
}

void DialogManager::add_dialog(const DialogNode &data) {
    combined_log.push_back(data);
    if (combined_log.size() > max_history) {
        combined_log.erase(combined_log.begin());
    }
}

void DialogManager::add_popup(const DialogNode &data) {
    pop_up_log.push_back(data);
    if (pop_up_log.size() > max_history) {
        pop_up_log.erase(pop_up_log.begin());
    }
}

void DialogManager::queue_dialog(const DialogNode& node) {
    dialog_queue.push(node);
}

bool DialogManager::has_queued_dialog() const {
    return !dialog_queue.empty();
}

DialogNode DialogManager::pop_dialog() {
    DialogNode node = dialog_queue.front();
    dialog_queue.pop();
    return node;
}

static bool evaluate_choice_condition(GameEngine* engine, const std::string& cond) {
    if (cond.empty()) return true;

    // Condition syntax: "<var> <op> <val>"
    // e.g. "item_ransum >= 1"
    std::stringstream ss(cond);
    std::string var;
    std::string op;
    int value = 0;
    
    if (ss >> var >> op >> value) {
        int actual_val = 0;
        if (var.rfind("item_", 0) == 0) {
            std::string item_id = var.substr(5);
            Player* p = engine->get_player_manager().get_player();
            if (p) {
                for (auto* item : p->get_inventory()) {
                    if (item && item->get_id() == item_id) {
                        actual_val++;
                    }
                }
            }
        } else {
            // General variable check
            auto& vars = engine->get_dialogs().get_variables();
            if (vars.find(var) != vars.end()) {
                actual_val = vars.at(var);
            }
        }
        
        if (op == ">=") return actual_val >= value;
        if (op == "<=") return actual_val <= value;
        if (op == "==") return actual_val == value;
        if (op == ">")  return actual_val > value;
        if (op == "<")  return actual_val < value;
    }
    
    Logger::log("DialogManager WARNING: Condition syntax invalid or not supported: '" + cond + "'");
    return false;
}

void DialogManager::start_scene(const DialogScene& scene, GameEngine* engine) {
    active_choices.clear();
    pending_choices.clear();
    selected_choice_index = 0;

    // Execute enter actions
    execute_actions(scene.on_start, engine);

    for (const auto& node : scene.nodes) {
        queue_dialog(node);
    }

    on_exit_actions = scene.on_exit;
    next_scene_id = scene.next_scene_id;
    pending_choices = scene.choices;
    
    Logger::log("DialogManager: Loaded scene '" + scene.id + "' with " + 
                std::to_string(scene.nodes.size()) + " nodes and " + 
                std::to_string(scene.choices.size()) + " choices.");
}

void DialogManager::activate_choices(GameEngine* engine) {
    active_choices.clear();
    for (const auto& choice : pending_choices) {
        if (evaluate_choice_condition(engine, choice.condition)) {
            active_choices.push_back(choice);
        }
    }
    pending_choices.clear();
    selected_choice_index = 0;
    Logger::log("DialogManager: Activated " + std::to_string(active_choices.size()) + " choices.");
}

void DialogManager::select_choice(int idx, GameEngine* engine) {
    if (idx < 0 || idx >= (int)active_choices.size()) return;
    
    DialogChoice choice = active_choices[idx];
    Logger::log("DialogManager: Selected choice: '" + choice.text + "' leading to scene '" + choice.next_scene + "'");
    
    clear_choices();
    
    if (!choice.next_scene.empty()) {
        const DialogScene* next_scene = engine->get_db().get_dialog_scene(choice.next_scene);
        if (next_scene) {
            this->start_scene(*next_scene, engine);
        } else {
            Logger::log("DialogManager ERROR: Choice target scene '" + choice.next_scene + "' not found!");
        }
    }
}

void DialogManager::execute_actions(const std::vector<std::string>& actions, GameEngine* engine) {
    for (const auto& action : actions) {
        if (action.empty()) continue;
        Logger::log("DialogManager: Executing action: " + action);

        if (action == "move_to_tavern") {
            engine->get_places().set_current_place("kedai_usang");
        }
        else if (action == "move_to_attic") {
            engine->get_places().set_current_place("kamar_loteng");
        }
        else if (action == "move_to_outskirts") {
            Place* p = engine->get_places().get_place("outskirts");
            if (!p) {
                p = new Place("outskirts", "Permukiman Kumuh");
                engine->get_places().add_place(p);
            }
            engine->get_places().set_current_place("outskirts");
        }
        else if (action == "move_to_alun_alun") {
            engine->get_places().set_current_place("alun_alun");
        }
        else if (action == "move_to_pasar_gelap") {
            Place* p = engine->get_places().get_place("pasar_gelap");
            if (!p) {
                p = new Place("pasar_gelap", "Black Market Silas");
                engine->get_places().add_place(p);
            }
            engine->get_places().set_current_place("pasar_gelap");
        }
        else if (action == "move_to_menara_tua") {
            Place* p = engine->get_places().get_place("menara_tua");
            if (!p) {
                p = new Place("menara_tua", "Reruntuhan Menara Tua");
                engine->get_places().add_place(p);
            }
            engine->get_places().set_current_place("menara_tua");
        }
        else if (action == "set_time_siang") {
            engine->get_calendar().setDayTime(AFTERNOON);
        }
        else if (action == "check_zona_kuning") {
            DialogNode node;
            node.type = 3;
            node.npc_name = "System";
            node.value = "WARNING: Memasuki Zona Kuning!";
            this->queue_dialog(node);
        }
        else if (action.rfind("add_trust_warga_", 0) == 0) {
            int amt = std::stoi(action.substr(16));
            variables["trust_warga"] += amt;
        }
        else if (action.rfind("remove_item_", 0) == 0) {
            std::string rem = action.substr(12);
            size_t last_underscore = rem.find_last_of('_');
            if (last_underscore != std::string::npos) {
                std::string item_id = rem.substr(0, last_underscore);
                int amt = std::stoi(rem.substr(last_underscore + 1));
                
                Player* p = engine->get_player_manager().get_player();
                if (p) {
                    int removed = 0;
                    auto& inv = const_cast<std::vector<Item*>&>(p->get_inventory());
                    for (auto it = inv.begin(); it != inv.end(); ) {
                        if (removed >= amt) break;
                        if (*it && (*it)->get_id() == item_id) {
                            delete *it;
                            it = inv.erase(it);
                            removed++;
                        } else {
                            ++it;
                        }
                    }
                }
            }
        }
        else if (action == "set_quest_warga_miskin_progres") {
            Quest* q = engine->get_quests().get_quest("warga_miskin");
            if (!q) {
                q = new Quest("warga_miskin", "Membantu warga miskin di permukiman kumuh");
                engine->get_quests().add_quest(q);
            }
            q->set_state(QuestState::IN_PROGRESS);
        }
        else if (action.rfind("add_item_", 0) == 0) {
            std::string add = action.substr(9);
            size_t last_underscore = add.find_last_of('_');
            if (last_underscore != std::string::npos) {
                std::string item_id = add.substr(0, last_underscore);
                int amt = std::stoi(add.substr(last_underscore + 1));
                
                Player* p = engine->get_player_manager().get_player();
                if (p) {
                    for (int i = 0; i < amt; ++i) {
                        p->add_item(new Item(item_id, item_id == "dokumen_gudang" ? "Dokumen Gudang" : item_id));
                    }
                }
            }
        }
        else if (action.rfind("add_clue_count_", 0) == 0) {
            int amt = std::stoi(action.substr(15));
            variables["clue_count"] += amt;
        }
        else if (action.rfind("consume_time_", 0) == 0) {
            int amt = std::stoi(action.substr(13));
            for (int i = 0; i < amt; ++i) {
                engine->get_calendar().advanceTime(false);
            }
        }
    }
}