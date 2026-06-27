#pragma once
#include "GameState.hpp"
#include "../models/Dungeon.hpp"
#include "../utils/components/ChoicePopup.hpp"
#include <string>
#include <vector>
#include <memory>
#include <ncurses.h>

struct QuestMarker {
    std::string quest_id;
    std::string scene_id;
    int floor_number;
    int x, y;
};

class DungeonState : public GameState {
private:
    Dungeon dungeon;
    bool has_won;
    int active_tab; // 0 = Party, 1 = Map

    std::vector<QuestMarker> quest_markers;

    std::unique_ptr<Utils::ChoicePopup> current_choice_popup;

    std::vector<std::vector<int>> generate_maze_grid(int h, int w, int exit_r, int exit_c);
    void update_visited(DungeonFloor& floor);

    void render_thought_tabs();
    void render_party_tab(WINDOW* win);
    void render_map_tab(WINDOW* win, const DungeonFloor& floor);

    void process_dialogue_queue();
    void handle_post_dialogue();

public:
    DungeonState(GameEngine* eng);
    
    void handle_input(int ch) override;
    void update() override;
    void render() override;
    void on_enter() override;
    void on_resume() override;
};