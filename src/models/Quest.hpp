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
    QuestState state;

    std::string target_location;
    std::string target_npc_id;
    std::string start_scene_id;
    std::string complete_scene_id;
    
    Condition unlock_condition;
    Condition completion_condition;
    std::vector<std::string> on_complete;

public:
    Quest(std::string id, std::string name, std::string desc) 
        : id(std::move(id)), name(std::move(name)), description(std::move(desc)), state(QuestState::LOCKED) {}

    void set_target_location(const std::string& loc) { target_location = loc; }
    const std::string& get_target_location() const { return target_location; }

    void set_target_npc(const std::string& npc_id) { target_npc_id = npc_id; }
    const std::string& get_target_npc_id() const { return target_npc_id; }

    void set_start_scene(std::string scene_id) { start_scene_id = std::move(scene_id); }
    const std::string& get_start_scene() const { return start_scene_id; }

    void set_complete_scene(std::string scene_id) { complete_scene_id = std::move(scene_id); }
    const std::string& get_complete_scene() const { return complete_scene_id; }

    void set_unlock_condition(Condition cond) { unlock_condition = std::move(cond); }
    const Condition& get_unlock_condition() const { return unlock_condition; }

    void set_completion_condition(Condition cond) { completion_condition = std::move(cond); }
    const Condition& get_completion_condition() const { return completion_condition; }

    void set_on_complete(std::vector<std::string> effects) { on_complete = std::move(effects); }
    const std::vector<std::string>& get_on_complete() const { return on_complete; }

    void try_unlock(const Player* player) {
        if (state == QuestState::LOCKED && unlock_condition.evaluate(player)) {
            state = QuestState::AVAILABLE;
        }
    }

    bool can_complete(const Player* player) const {
        return state == QuestState::IN_PROGRESS && completion_condition.evaluate(player);
    }

    QuestState get_state() const { return state; }
    void set_state(QuestState s) { state = s; }
    const std::string& get_id() const { return id; }
    const std::string& get_name() const { return name; }
    const std::string& get_description() const { return description; }
};