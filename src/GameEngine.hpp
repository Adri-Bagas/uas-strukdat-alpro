#pragma once
#include <memory>
#include <stack>
#include <vector>
#include "managers/DialogManagers.hpp"
#include "views/MainPage.hpp"
#include "views/StartMenuPage.hpp"
#include "states/GameState.hpp"
#include "managers/TimeCalendarManagers.hpp"
#include "managers/PlaceManagers.hpp"
#include "managers/QuestManagers.hpp"
#include "managers/PlayerManager.hpp"
#include "./db/DB.hpp"
#include "actions/Actions.hpp"
#include "managers/ShopManager.hpp"
#include "managers/LogManager.hpp"
#include "utils/components/Popup.hpp"
#include "managers/MusicManager.hpp"

class GameEngine {
private:
    DialogManager dialogs;
    MainPage page;
    StartMenuPage start_page;
    TimeCalendarManagers calendar;
    PlaceManagers places;
    QuestManager quests;
    PlayerManager player_manager;
    DB db;
    Action actions;
    MusicManager music_manager;
    ShopManager shop_manager;
    LogManager log_manager;

    std::stack<std::unique_ptr<GameState>> state_stack;
    std::unique_ptr<Utils::Popup> active_popup;
    bool is_running = true;

    struct StateOp {
        enum Type { PUSH, POP, CHANGE } type;
        std::unique_ptr<GameState> state;
    };
    std::vector<StateOp> pending_ops;
    void process_state_operations();

public:
    GameEngine();
    void init(); // Setup ncurses, set initial state

    void push_state(GameState* new_state);
    void change_state(GameState* new_state);

    // Removes top state, returning to the one below it (e.g., Battle ends -> back to Dungeon)
    void pop_state();
    void run();
    void show_popup(std::unique_ptr<Utils::Popup> popup);
    GameState* get_current_state() { return state_stack.empty() ? nullptr : state_stack.top().get(); }

    // Getters for states to access models
    DialogManager& get_dialogs();
    MainPage& get_layout();
    StartMenuPage& get_start_layout();
    TimeCalendarManagers& get_calendar();
    PlaceManagers& get_places();
    QuestManager& get_quests();
    PlayerManager& get_player_manager();
    DB& get_db();
    Action& get_actions();
    MusicManager& get_music_manager();
    ShopManager& get_shop_manager();
    LogManager& get_log_manager();
    void quit();
};
