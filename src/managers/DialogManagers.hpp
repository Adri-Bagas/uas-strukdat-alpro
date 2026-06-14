#pragma once
#include <queue>
#include <vector>
#include "../models/Dialog.hpp"


class DialogManager {
    std::vector<DialogNode> pop_up_log;
    std::vector<DialogNode> combined_log;

    std::queue<DialogNode> dialog_queue;

    std::vector<std::string> on_exit_action;
    std::string next_scene_id;

    size_t max_history = 50;

public:
    // Getters for the View
    const std::vector<DialogNode>& get_popup() const { return pop_up_log; }
    const std::vector<DialogNode>& get_combined_log() const { return combined_log; }

    void add_thought(const DialogNode& text);
    void add_dialog(const DialogNode& text);
    void add_popup(const DialogNode& text);

    void queue_dialog(const DialogNode& node);
    bool has_queued_dialog() const;
    DialogNode pop_dialog();

    void set_on_exit(const std::vector<std::string>& actions) { on_exit_action = actions; }
    std::vector<std::string> get_on_exit() const { return on_exit_action; }

    void set_next_scene(const std::string& id) { next_scene_id = id; }
    std::string get_next_scene() const { return next_scene_id; }
};