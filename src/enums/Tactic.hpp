#pragma once
#include <string>

enum class Tactic {
    ACT_FREELY,
    FULL_ASSAULT,
    HEAL_SUPPORT,
    CONSERVE_SP
};

inline std::string tactic_to_string(Tactic t) {
    switch (t) {
        case Tactic::ACT_FREELY: return "Act Freely";
        case Tactic::FULL_ASSAULT: return "Full Assault";
        case Tactic::HEAL_SUPPORT: return "Heal/Support";
        case Tactic::CONSERVE_SP: return "Conserve SP";
        default: return "Unknown";
    }
}
