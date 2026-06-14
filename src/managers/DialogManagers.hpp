#pragma once
#include <queue>
#include <vector>
#include <string>
#include <unordered_map>
#include "../models/Dialog.hpp"

class GameEngine;

class DialogManager {
    std::vector<DialogNode> pop_up_log;
    std::vector<DialogNode> combined_log;

    std::queue<DialogNode> dialog_queue;
    std::vector<std::string> on_exit_actions;
    std::string next_scene_id;

    std::vector<DialogChoice> pending_choices;
    std::vector<DialogChoice> active_choices;
    int selected_choice_index = 0;

    std::unordered_map<std::string, int> variables;
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

    void set_on_exit(const std::vector<std::string>& actions) { on_exit_actions = actions; }
    const std::vector<std::string>& get_on_exit() const { return on_exit_actions; }

    void set_next_scene(const std::string& id) { next_scene_id = id; }
    std::string get_next_scene() const { return next_scene_id; }

    void start_scene(const DialogScene& scene, GameEngine* engine);
    
    bool has_pending_choices() const { return !pending_choices.empty(); }
    bool has_active_choices() const { return !active_choices.empty(); }
    void activate_choices(GameEngine* engine);

    const std::vector<DialogChoice>& get_active_choices() const { return active_choices; }
    int get_selected_choice_index() const { return selected_choice_index; }
    void set_selected_choice_index(int idx) { selected_choice_index = idx; }
    
    void select_choice(int idx, GameEngine* engine);
    void clear_choices() {
        active_choices.clear();
        pending_choices.clear();
        selected_choice_index = 0;
    }

    std::unordered_map<std::string, int>& get_variables() { return variables; }
    const std::unordered_map<std::string, int>& get_variables() const { return variables; }

    void execute_actions(const std::vector<std::string>& actions, GameEngine* engine);
};