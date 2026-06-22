#include "Condition.hpp"
#include "Player.hpp"
#include "../managers/QuestManagers.hpp"

bool Condition::evaluate(const Player* player, const QuestManager* quest_manager) const {
    if (type == ConditionType::NONE) return true;
    if (!player) return false;

    switch (type) {
        case ConditionType::VAR_EQUAL:
            return player->get_var(key) == value;
        case ConditionType::VAR_GREATER_EQUAL:
            return player->get_var(key) >= value;
        case ConditionType::VAR_LESS_EQUAL:
            return player->get_var(key) <= value;
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