#pragma once
#include <string>
#include <vector>
#include "Condition.hpp"
#include "../enums/QuestState.hpp"

class Player;

class Quest {
private:
    std::string id;
    std::string name;
    std::string description;
    std::string objective_text;
    QuestState state;

    std::string target_location_id;
    std::string location_trigger_scene;
    std::vector<std::string> location_trigger_action;
    
    std::string source_npc_id;
    std::string target_npc_id;
    std::string start_scene_id;
    std::string complete_scene_id;
    
    std::vector<Condition> unlock_conditions;
    Condition completion_condition;
    std::vector<std::string> on_complete;

public:
    Quest(std::string id, std::string name, std::string desc) 
        : id(std::move(id)), name(std::move(name)), description(std::move(desc)), state(QuestState::LOCKED) {}

    void set_objective_text(std::string text) { objective_text = std::move(text); }
    const std::string& get_objective_text() const { return objective_text; }

    void set_target_location_id(const std::string& loc) { target_location_id = loc; }
    const std::string& get_target_location_id() const { return target_location_id; }

    void set_location_trigger_scene(std::string scene_id) { location_trigger_scene = std::move(scene_id); }
    const std::string& get_location_trigger_scene() const { return location_trigger_scene; }

    void set_location_trigger_action(std::vector<std::string> actions) { location_trigger_action = std::move(actions); }
    const std::vector<std::string>& get_location_trigger_action() const { return location_trigger_action; }

    void set_source_npc(const std::string& npc_id) { source_npc_id = npc_id; }
    const std::string& get_source_npc_id() const { return source_npc_id.empty() ? target_npc_id : source_npc_id; }

    void set_target_npc(const std::string& npc_id) { target_npc_id = npc_id; }
    const std::string& get_target_npc_id() const { return target_npc_id; }

    void set_start_scene(std::string scene_id) { start_scene_id = std::move(scene_id); }
    const std::string& get_start_scene() const { return start_scene_id; }

    void set_complete_scene(std::string scene_id) { complete_scene_id = std::move(scene_id); }
    const std::string& get_complete_scene() const { return complete_scene_id; }

    void add_unlock_condition(Condition cond) { unlock_conditions.push_back(std::move(cond)); }
    const std::vector<Condition>& get_unlock_conditions() const { return unlock_conditions; }

    void set_completion_condition(Condition cond) { completion_condition = std::move(cond); }
    const Condition& get_completion_condition() const { return completion_condition; }

    void set_on_complete(std::vector<std::string> effects) { on_complete = std::move(effects); }
    const std::vector<std::string>& get_on_complete() const { return on_complete; }

    void try_unlock(const Player* player) {
        if (state == QuestState::LOCKED) {
            bool unlocked = true;
            for (const auto& cond : unlock_conditions) {
                if (!cond.evaluate(player)) {
                    unlocked = false;
                    break;
                }
            }
            if (unlocked) {
                state = QuestState::AVAILABLE;
            }
        }
    }

    bool can_complete(const Player* player, const QuestManager* qm = nullptr) const {
        return (state == QuestState::IN_PROGRESS || state == QuestState::READY_TO_TURN_IN) && completion_condition.evaluate(player, qm);
    }

    QuestState get_state() const { return state; }
    void set_state(QuestState s) { state = s; }
    const std::string& get_id() const { return id; }
    const std::string& get_name() const { return name; }
    const std::string& get_description() const { return description; }
};