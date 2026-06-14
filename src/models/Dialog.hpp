#pragma once 
#include <string>
#include <vector>
#include "Condition.hpp"

struct DialogNode {
    std::string value;
    std::string npc_name;
    int type; // 1: Dialog, 2: Thought, 3: Popup
};

struct DialogScene {
    std::string id;
    Condition condition;
    std::vector<DialogNode> nodes;
    std::vector<std::string> on_enter;
    std::vector<std::string> on_exit;
    std::string next_scene_id;
};
