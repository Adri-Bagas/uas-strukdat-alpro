#include "DialogManagers.hpp"
#include "../GameEngine.hpp"
#include "../utils/components/Popup.hpp"

bool evaluate_choice_condition(GameEngine* engine, const Condition& condition) {
    if (condition.is_empty()) return true;
    
    Player* p = engine->get_player_manager().get_player();
    if (!p) return false;

    return condition.evaluate(p, &engine->get_quests());
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

void DialogManager::queue_popup(const std::string &msg, bool is_narrator) {
    popup_queue.push({msg, is_narrator});
}

bool DialogManager::has_queued_popup() const {
    return !popup_queue.empty();
}

std::pair<std::string, bool> DialogManager::pop_popup() {
    auto data = popup_queue.front();
    popup_queue.pop();
    return data;
}

void DialogManager::start_scene(const DialogScene& scene, GameEngine* engine) {
    clear_choices();
    next_scene_id = scene.next_scene_id;
    on_exit_actions = scene.on_exit;

    for (auto node : scene.nodes) {
        if (!node.npc_name.empty()) {
            const NPC* npc = engine->get_db().get_npc(node.npc_name);
            if (npc) {
                if (!npc->name_known()) {
                    node.npc_name = "???";
                } else {
                    node.npc_name = npc->get_name();
                }
            }
        }
        queue_dialog(node);
    }

    execute_actions(scene.on_start, engine);

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
    std::vector<std::string> on_select = active_choices[idx].on_select;
    
    clear_choices();
    
    execute_actions(on_select, engine);
    
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
        
        // Use standard dispatcher
        engine->get_actions().execute(action);
    }
}