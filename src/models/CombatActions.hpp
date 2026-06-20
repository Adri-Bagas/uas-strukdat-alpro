#pragma once
#include <string>
#include <vector>
#include "../enums/Element.hpp"

enum class TargetRange {
    DIRECT,
    REACH,
    AOE
};

enum class MagicType {
    ATTACKING,
    SUPPORT,
    HEALING
};

struct CombatModifier {
    std::string stat_name; // e.g., "str", "agi", "def"
    int amount;
    int duration_turns;
};

struct Magic {
    std::string id;
    std::string name;
    MagicType type;
    int mana_cost = 0;
    int power = 0; // Base damage or heal amount
    Element elem = Element::NONE;
    TargetRange range = TargetRange::REACH;
    std::vector<CombatModifier> modifiers; // Buffs/Debuffs
};

struct SpecialMove {
    std::string id;
    std::string name;
    int max_uses_per_day = 1;
    int current_uses = 0;
    int power = 0;
    Element elem = Element::NONE;
    TargetRange range = TargetRange::AOE;
    std::vector<CombatModifier> modifiers;
};
