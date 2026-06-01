#pragma once
#include <string>
#include <functional>
#include "../enums/QuestState.hpp"

class Quest {
private:
    std::string id;
    std::string description;
    QuestState state;

    std::string target_location;
    std::string target_npc_id;
    
    // std::function allows dynamic condition checks (e.g., check global clue_count or trust)
    std::function<bool()> unlock_condition; 
    std::function<void()> on_complete;

public:
    Quest(std::string id, std::string desc) 
        : id(std::move(id)), description(std::move(desc)), state(QuestState::LOCKED) {}

    void set_target_location(const std::string& loc) { target_location = loc; }
    void set_target_npc(const std::string& npc_id) { target_npc_id = npc_id; }

    const std::string& get_target_location() const { return target_location; }
    const std::string& get_target_npc_id() const { return target_npc_id; }

    void set_unlock_condition(std::function<bool()> condition) {
        unlock_condition = condition;
    }

    void set_on_complete(std::function<void()> effect) {
        on_complete = effect;
    }

    void try_unlock() {
        if (state == QuestState::LOCKED && unlock_condition && unlock_condition()) {
            state = QuestState::AVAILABLE;
        }
    }

    void complete() {
        if (state == QuestState::IN_PROGRESS) {
            state = QuestState::COMPLETED;
            if (on_complete) on_complete(); // Execute rewards (give gold, increase clue)
        }
    }

    QuestState get_state() const { return state; }
    void set_state(QuestState s) { state = s; }
    const std::string& get_id() const { return id; }
};