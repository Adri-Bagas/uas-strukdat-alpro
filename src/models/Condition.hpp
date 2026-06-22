#pragma once
#include <string>

class Player;
class QuestManager;

enum class ConditionType {
    NONE,
    VAR_EQUAL,
    VAR_GREATER_EQUAL,
    VAR_LESS_EQUAL,
    HAS_ITEM,
    QUEST_STATE,
    KILLED_MONSTER,
    EXPLORED_AREA
};

struct Condition {
    ConditionType type = ConditionType::NONE;
    std::string key;
    int value = 0;
    std::string string_value;

    bool is_empty() const { return type == ConditionType::NONE; }

    bool evaluate(const Player* player, const QuestManager* quest_manager = nullptr) const;
};