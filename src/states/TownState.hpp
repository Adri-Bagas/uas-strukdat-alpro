#pragma once
#include "GameState.hpp"
#include "../models/Activity.hpp"
#include "../models/NPC.hpp"
#include "../models/Quest.hpp"
#include "../models/Place.hpp"
#include <vector>
#include <memory>

class ChoicePopup;

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

    // Map Mode
    bool is_in_map_mode = false;
    int map_selection_index = 0;
    std::vector<Place*> map_places;

    // Choice Component
    std::unique_ptr<ChoicePopup> current_choice_popup;

private:
    // --- Input Helpers ---
    void handle_quest_menu_input(int ch);
    void handle_world_menu_input(int ch);
    void handle_map_menu_input(int ch);
    void execute_npc_interaction(NPC* npc);
    void execute_activity(const Activity& act);
    void execute_movement(Place* target);

    // --- Update Helpers ---
    void process_dialogue_queue();
    void handle_post_dialogue();

    // --- Render Helpers ---
    void render_player_status(Player* p);
    void render_quest_menu(Player* p, std::vector<std::string>& menu_display);
    void render_world_menu(Player* p, std::vector<std::string>& menu_display);
    void render_map_preview(Player* p, std::vector<std::string>& menu_display);
    void render_sidebars(Player* p);

public:
    TownState(GameEngine* eng);
    
    void handle_input(int ch) override;
    void update() override;
    void render() override;
    void on_enter() override;
};