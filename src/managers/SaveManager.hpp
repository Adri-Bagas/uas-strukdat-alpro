#pragma once
#include <string>
#include <vector>

class GameEngine;

struct SaveFileInfo {
    std::string filename;
    std::string player_name;
    int level;
    int day;
    std::string phase_name;
};

class SaveManager {
public:
    static bool save_game(GameEngine* engine, const std::string& filename);
    static bool load_game(GameEngine* engine, const std::string& filename);
    static std::vector<SaveFileInfo> get_save_files();
};
