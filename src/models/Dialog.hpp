#pragma once 
#include <string>
#include <vector>
#include "Condition.hpp"

struct DialogNode {
    std::string value;
    std::string npc_name;
    int type; // 1: Dialog, 2: Thought, 3: Utils::Popup
};

struct DialogChoice {
    std::string text;
    Condition condition;
    std::string next_scene;
    std::vector<std::string> on_select;
};

struct DialogScene {
    std::string id;
    Condition condition;
    std::vector<DialogNode> nodes;
    std::vector<std::string> on_start;
    std::vector<std::string> on_exit;
    std::string next_scene_id;
    std::vector<DialogChoice> choices;
};
