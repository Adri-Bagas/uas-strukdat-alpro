#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Condition.hpp"

struct Activity {
    std::string id;
    std::string name;
    
    // Visibility
    Condition visible_condition;

    // Kept for UI display purposes
    int time_cost = 1;
    
    std::vector<int> days; // Empty means all days
    std::vector<std::string> phases; // e.g. ["Morning", "Night"]
    
    // Persona-like stat requirements (e.g. {"str": 10, "intl": 5})
    std::unordered_map<std::string, int> req_stats;
    bool is_locked = false; 

    // Custom dialog responses
    std::string dialog_success;
    std::string dialog_fail;
    std::string dialog_locked;
    
    // Actions triggered when this activity is performed
    std::vector<std::string> on_execute;
};