#pragma once

#include <functional>
#include <string>
#include <unordered_map>
class GameEngine;

class Action {

    GameEngine *engine;

    std::unordered_map<std::string, std::function<void(const std::string&)>> actions;

    public:
    Action(GameEngine *eng);
    void register_action(const std::string& key, std::function<void(const std::string&)> func);
    void execute(const std::string& command_line);
};