#pragma once
#include "GameState.hpp"
#include "../models/Activity.hpp"
#include "../models/NPC.hpp"
#include "../models/Quest.hpp"
#include "../models/Place.hpp"
#include <vector>

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

public:
    TownState(GameEngine* eng);
    
    void handle_input(int ch) override;
    void update() override;
    void render() override;
    void on_enter() override;
};