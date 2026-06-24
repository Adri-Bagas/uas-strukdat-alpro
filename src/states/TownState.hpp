#pragma once
#include "GameState.hpp"
#include "../models/Activity.hpp"
#include "../models/NPC.hpp"
#include "../models/Quest.hpp"
#include "../models/Place.hpp"
#include <vector>
#include <memory>
#include <queue>
#include "../utils/Queue.hpp"
#include "../utils/Stack.hpp"
#include "../views/MainPage.hpp"

namespace Utils { class ChoicePopup; }

enum class MenuTab {
    NPC,
    ACTIVITY,
    EXIT,
    MAP
};

class TownState : public GameState {
    int selection_index = 0;
    std::vector<Activity> current_activities;
    std::vector<NPC*> current_npcs;
    std::vector<Place*> current_exits;

    // NPC Interaction Mode
    NPC* interacting_npc = nullptr;
    std::vector<Quest*> available_quests;
    bool is_in_quest_menu = false;
    int quest_selection_index = 0;

    // Tab Mode
    MenuTab current_tab = MenuTab::MAP;
    int map_selection_index = 0;
    std::vector<Place*> map_places;

    // Cached Map Graph untuk Optimasi Performa
    std::string cached_root_id = "";
    std::vector<GraphNode> cached_graph_nodes;
    std::vector<GraphEdge> cached_edges;

    // Choice Component
    std::unique_ptr<Utils::ChoicePopup> current_choice_popup;

    // Fast Travel
    Utils::Queue<std::string> fast_travel_queue;
    bool is_fast_traveling = false;
    bool is_confirming_fast_travel = false;
    Place* fast_travel_target = nullptr;
    std::vector<std::string> fast_travel_path_preview;

    // Undo Movement History
    Utils::Stack<std::string> movement_history;

    // Queued Scenes
    std::queue<std::string> queued_scenes;

private:
    // --- Input Helpers ---
    void cycle_tab();
    void init_tabs();
    void handle_quest_menu_input(int ch);
    void handle_world_menu_input(int ch);
    void handle_map_menu_input(int ch);
    void execute_npc_interaction(NPC* npc);
    void execute_activity(const Activity& act);
    void execute_movement(Place* target);
    void execute_fast_travel_step();
    std::vector<std::string> find_shortest_path(const std::string& start, const std::string& target);

    // --- Update Helpers ---
    void process_dialogue_queue();
    void handle_post_dialogue();

    // --- Render Helpers ---
    void render_player_status(Player* p);
    void render_quest_menu(Player* p, std::vector<std::string>& menu_display);
    void render_world_menu(Player* p, std::vector<std::string>& menu_display);
    void render_map_preview(Player* p, std::vector<std::string>& menu_display);
    void render_sidebars(Player* p);

    // --- Optimasi ---
    void rebuild_map_graph();

public:
    TownState(GameEngine* eng);
    
    void handle_input(int ch) override;
    void update() override;
    void render() override;
    void on_enter() override;
};