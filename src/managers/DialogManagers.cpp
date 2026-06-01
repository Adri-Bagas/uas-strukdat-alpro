#include "DialogManagers.hpp"

void DialogManager::add_thought(const std::string &text) {
    thought_log.push_back(text);
    if (thought_log.size() > max_history) {
        thought_log.erase(thought_log.begin());
    }
}

void DialogManager::add_dialog(const std::string &npc_name, const std::string &text) {
    dialog_log.push_back("[" + npc_name + "]: " + text);
    if (dialog_log.size() > max_history) {
        dialog_log.erase(dialog_log.begin());
    }
}