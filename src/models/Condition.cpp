#include "Condition.hpp"
#include "Player.hpp"
#include "../managers/QuestManagers.hpp"

bool Condition::evaluate(const Player* player, const QuestManager* quest_manager) const {
    if (type == ConditionType::NONE) return true;
    if (!player) return false;

    auto get_val = [&](const std::string& k) {
        if (k == "gold" || k == "Gold") return player->get_gold();
        return player->get_var(k);
    };

    switch (type) {
        case ConditionType::VAR_EQUAL:
            return get_val(key) == value;
        case ConditionType::VAR_GREATER_EQUAL:
            return get_val(key) >= value;
        case ConditionType::VAR_LESS_EQUAL:
            return get_val(key) <= value;
        case ConditionType::HAS_ITEM:
            return player->get_item_count(key) >= value;
        case ConditionType::QUEST_STATE:
            if (quest_manager) {
                const Quest* q = const_cast<QuestManager*>(quest_manager)->get_quest(key);
                if (q) return (int)q->get_state() == value;
            }
            return false;
        case ConditionType::KILLED_MONSTER:
            return player->get_kill_count(key) >= value;
        case ConditionType::EXPLORED_AREA:
            return player->has_explored(string_value);
        default:
            return true;
    }
}