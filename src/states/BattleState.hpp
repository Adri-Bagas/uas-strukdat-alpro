#pragma once
#include "GameState.hpp"
#include "../views/BattlePage.hpp"
#include "../models/NPC.hpp"
#include <memory>
#include <vector>
#include <array>
#include <string>

class GameEngine;

class BattleState : public GameState {
private:
    GameEngine* engine;
    BattlePage view;

    // The combatants
    std::array<Entity*, 4> party_slots;
    std::array<Entity*, 4> enemy_slots;
    
    // Enemy queue for dynamic spawning
    std::vector<std::unique_ptr<Monster>> enemy_pool;
    
    // Owned active enemies currently in slots
    std::vector<std::unique_ptr<Monster>> active_enemies;

    std::vector<Entity*> turn_queue;
    
    // UI states
    int current_menu_selection;
    std::vector<std::string> menu_options;
    std::string battle_log;

    // Sub-states
    enum class Phase {
        WAITING_FOR_ACTION,
        SELECTING_TARGET_ENEMY,
        SELECTING_TARGET_ALLY,
        SELECTING_MAGIC,
        SELECTING_ITEM,
        PROCESSING_TURN
    };
    Phase current_phase;
    std::string pending_action; // e.g., "Attack", "Magic:Fireball"

    void populate_enemies(const std::string& monster_group_id);
    void build_turn_queue();
    void next_turn();
    void execute_enemy_turn(Entity* enemy);
    
    void build_main_menu();

public:
    BattleState(GameEngine* engine, const std::string& monster_group_id);
    ~BattleState() override = default;

    void on_enter() override;
    void handle_input(int ch) override;
    void update() override;
    void render() override;
    void on_exit() override;
};
