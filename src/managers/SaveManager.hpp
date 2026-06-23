#pragma once
#include <string>

class GameEngine;

class SaveManager {
public:
    static bool save_game(GameEngine* engine, const std::string& filepath);
    static bool load_game(GameEngine* engine, const std::string& filepath);
};
