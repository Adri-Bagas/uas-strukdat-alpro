#include "DialogManagers.hpp"
#include "../GameEngine.hpp"
#include "../utils/components/Popup.hpp"

bool evaluate_choice_condition(GameEngine* engine, const std::string& condition) {
    if (condition.empty()) return true;
    
    Player* p = engine->get_player_manager().get_player();
    if (!p) return false;

    // Colleague's simple parser: "item_ransum >= 1"
    if (condition.rfind("item_", 0) == 0) {
        std::string req = condition.substr(5);
        size_t op_pos = req.find(">=");
        if (op_pos != std::string::npos) {
            std::string item_id = req.substr(0, op_pos);
            // Trim
            item_id.erase(item_id.find_last_not_of(" \n\r\t") + 1);
            int amt = std::stoi(req.substr(op_pos + 2));
            return p->get_item_count(item_id) >= amt;
        }
    }
    
    // Fallback: Check variables
    auto it = engine->get_dialogs().get_variables().find(condition);
    if (it != engine->get_dialogs().get_variables().end()) {
        return it->second > 0;
    }

    return true;
}

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

void DialogManager::queue_dialog(const DialogNode &node) {
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

void DialogManager::queue_popup(const std::string &msg) {
    popup_queue.push(msg);
}

bool DialogManager::has_queued_popup() const {
    return !popup_queue.empty();
}

std::string DialogManager::pop_popup() {
    std::string msg = popup_queue.front();
    popup_queue.pop();
    return msg;
}

void DialogManager::start_scene(const DialogScene& scene, GameEngine* engine) {
    clear_choices();
    next_scene_id = scene.next_scene_id;
    on_exit_actions = scene.on_exit;

    execute_actions(scene.on_start, engine);

    for (const auto& node : scene.nodes) {
        queue_dialog(node);
    }

    for (const auto& choice : scene.choices) {
        pending_choices.push_back(choice);
    }
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
}

void DialogManager::select_choice(int idx, GameEngine* engine) {
    if (idx < 0 || idx >= (int)active_choices.size()) return;
    
    std::string next = active_choices[idx].next_scene;
    clear_choices();
    
    if (!next.empty()) {
        const DialogScene* scene = engine->get_db().get_dialog_scene(next);
        if (scene) {
            start_scene(*scene, engine);
        }
    }
}

void DialogManager::execute_actions(const std::vector<std::string>& actions, GameEngine* engine) {
    for (const auto& action : actions) {
        if (action.empty()) continue;
        
        // Colleague's hardcoded actions refactored to use our system where possible
        if (action == "move_to_outskirts") {
            engine->get_places().set_current_place("permukiman_kumuh");
        }
        else if (action == "move_to_pasar_gelap") {
            engine->get_places().set_current_place("permukiman_kumuh"); // Placeholder
        }
        else if (action == "move_to_menara_tua") {
            engine->get_places().set_current_place("menara_tua");
        }
        else if (action == "move_to_alun_alun") {
            engine->get_places().set_current_place("alun_alun");
        }
        else if (action == "set_time_siang") {
            engine->get_calendar().setDayTime(AFTERNOON);
        }
        else if (action == "check_zona_kuning") {
            engine->get_dialogs().queue_popup("Peringatan: Memasuki Zona Kuning!");
        }
        else if (action.rfind("add_trust_warga_", 0) == 0) {
            int amt = std::stoi(action.substr(16));
            Player* p = engine->get_player_manager().get_player();
            if (p) p->add_var("trust_warga", amt);
        }
        else if (action.rfind("remove_item_", 0) == 0) {
            // Refactored to use generic Action dispatcher if needed, 
            // but for now I'll just fix the Colleague's direct implementation
            std::string rem = action.substr(12);
            size_t last_underscore = rem.find_last_of('_');
            if (last_underscore != std::string::npos) {
                std::string item_id = rem.substr(0, last_underscore);
                int amt = std::stoi(rem.substr(last_underscore + 1));
                Player* p = engine->get_player_manager().get_player();
                if (p) p->remove_item(item_id, amt);
            }
        }
        else if (action == "set_quest_warga_miskin_progres") {
            Quest* q = engine->get_quests().get_quest("warga_miskin");
            if (q) q->set_state(QuestState::IN_PROGRESS);
        }
        else if (action.rfind("add_item_", 0) == 0) {
            std::string add = action.substr(9);
            size_t last_underscore = add.find_last_of('_');
            if (last_underscore != std::string::npos) {
                std::string item_id = add.substr(0, last_underscore);
                int amt = std::stoi(add.substr(last_underscore + 1));
                engine->get_actions().execute("give_item " + item_id + " " + std::to_string(amt));
            }
        }
        else if (action.rfind("add_clue_count_", 0) == 0) {
            int amt = std::stoi(action.substr(15));
            Player* p = engine->get_player_manager().get_player();
            if (p) p->add_var("clue_count", amt);
        }
        else if (action.rfind("consume_time_", 0) == 0) {
            int amt = std::stoi(action.substr(13));
            for (int i = 0; i < amt; ++i) engine->get_calendar().advanceTime(false);
        }
        else {
            // Try standard dispatcher
            engine->get_actions().execute(action);
        }
    }
}