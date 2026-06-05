#pragma once
#include <string>
#include <vector>

struct Activity {
    std::string id;
    std::string name;
    int time_cost = 1;
    int stamina_cost = 0;
    
    std::vector<int> days; // Empty means all days
    std::vector<std::string> phases; // e.g. ["Morning", "Night"]
    
    std::string req_quest;
    int min_str = 0;
    bool is_locked = false; 
    
    int reward_gold = 0;
    int reward_str = 0;
    std::string finish_quest_id;
};