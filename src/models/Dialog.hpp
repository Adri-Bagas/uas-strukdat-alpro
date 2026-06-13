#pragma once 
#include <string>
#include <vector>

struct DialogNode {
    std::string value;
    std::string npc_name;
    int type; // 1: Dialog, 2: Thought, 3: Popup
};

struct DialogChoice {
    std::string text;
    std::string condition;
    std::string next_scene;
};

struct DialogScene {
    std::string id;
    std::vector<DialogNode> nodes;
    std::vector<std::string> on_start;
    std::vector<std::string> on_exit;
    std::string next_scene_id;
    std::vector<DialogChoice> choices;
};
