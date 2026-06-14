#include "DialogManagers.hpp"

void DialogManager::add_thought(const DialogNode &data) {
    combined_log.push_back(data);
    if (combined_log.size() > max_history) {
        combined_log.erase(combined_log.begin());
    }
}

void DialogManager::add_dialog(const DialogNode &data) {
    combined_log.push_back(data);
    if (combined_log.size() > max_history) {
        combined_log.erase(combined_log.begin());
    }
}

void DialogManager::add_popup(const DialogNode &data) {
    pop_up_log.push_back(data);
    if (pop_up_log.size() > max_history) {
        pop_up_log.erase(pop_up_log.begin());
    }
}

void DialogManager::queue_dialog(const DialogNode& node) {
    dialog_queue.push(node);
}

bool DialogManager::has_queued_dialog() const {
    return !dialog_queue.empty();
}

DialogNode DialogManager::pop_dialog() {
    DialogNode node = dialog_queue.front();
    dialog_queue.pop();
    return node;
}