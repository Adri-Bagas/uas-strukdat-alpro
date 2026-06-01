#pragma once
#include <string>
#include <vector>

class DialogManager {
    std::vector<std::string> thought_log;
    std::vector<std::string> dialog_log;
    size_t max_history = 50; // Maximum history size

public:
    // Getters for the View
    const std::vector<std::string>& get_thoughts() const { return thought_log; }
    const std::vector<std::string>& get_dialog() const { return dialog_log; }

    void add_thought(const std::string& text);
    void add_dialog(const std::string& npc_name, const std::string& text);
};