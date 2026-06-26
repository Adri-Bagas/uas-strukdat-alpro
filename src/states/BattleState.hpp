#pragma once
#include "GameState.hpp"
#include "../views/BattlePage.hpp"
#include "../models/NPC.hpp"
#include <memory>
#include <vector>
#include <array>
#include <string>
#include "../utils/components/ChoicePopup.hpp"
#include "../utils/CircularLinkedList.hpp"

class GameEngine;

class BattleState : public GameState {
private:
    BattlePage view;

    // The combatants
    std::array<Entity*, 4> party_slots;
    std::array<Entity*, 4> enemy_slots;
    
    // Enemy queue for dynamic spawning
    std::vector<std::unique_ptr<Monster>> enemy_pool;
    
    // Owned active enemies currently in slots
    std::vector<std::unique_ptr<Monster>> active_enemies;

    Utils::CircularLinkedList<Entity*> turn_queue;
    
    // UI states
    int current_menu_selection;
    std::vector<std::string> menu_options;
    std::vector<std::string> battle_log;
    bool skip_animations = false;

    // Sub-states
    enum class Phase {
        WAITING_FOR_ACTION,
        SELECTING_TARGET_ENEMY,
        SELECTING_TARGET_ALLY,
        SELECTING_MAGIC,
        SELECTING_ITEM,
        SELECTING_SWAP_SLOT,
        SELECTING_TACTIC_MEMBER,
        SELECTING_TACTIC_TYPE,
        PROCESSING_TURN,
        WAITING_FOR_INPUT, // Wait for user to read log
        BATTLE_END
    };
    Phase current_phase;
    std::string pending_action; // e.g., "Attack", "Magic:Fireball"
    
    // End Battle Drops
    int accumulated_exp = 0;
    int accumulated_gold = 0;
    std::vector<std::string> end_battle_logs;
    int selected_magic_idx = -1;

    std::string selected_item_id;
    std::vector<std::string> current_item_menu_ids;
    int selected_tactic_member = -1;

    void populate_enemies(const std::string& monster_group_id);
    void build_turn_queue();
    void next_turn();
    void execute_ai_turn(Entity* active, bool is_ally);
    void add_log(const std::string& msg);
    void animate_hit(Entity* target);
    
    std::unique_ptr<Utils::Popup> end_popup;
    std::string victory_action;
    
    void build_main_menu();
    void build_enemy_target_menu();
    void build_ally_target_menu();
    void build_swap_slot_menu();
    void build_magic_menu();
    void build_item_menu();
    void build_tactic_member_menu();
    void build_tactic_type_menu();
    void execute_action(Entity* target);

    bool can_flee;

public:
    BattleState(GameEngine* engine, const std::string& enemies_list, const std::string& victory_action = "", bool can_flee = true);
    ~BattleState() override = default;

    void on_enter() override;
    void handle_input(int ch) override;
    void update() override;
    void render() override;
    void on_exit() override;
};
