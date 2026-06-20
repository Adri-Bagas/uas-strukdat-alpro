#include "BattleState.hpp"
#include "../GameEngine.hpp"
#include <algorithm>

BattleState::BattleState(GameEngine* engine, const std::string& monster_group_id)
    : GameState(engine), current_menu_selection(0), current_phase(Phase::PROCESSING_TURN) {
    for (int i = 0; i < 4; ++i) {
        enemy_slots[i] = nullptr;
    }
    // Deep copy party slots
    party_slots = engine->get_player_manager().get_party_slots();
    populate_enemies(monster_group_id);
}

void BattleState::populate_enemies(const std::string& monster_group_id) {
    // For now, load 2-4 random monsters or hardcoded group
    // Ideally we fetch from DB.
    // Let's just create some dummy monsters for testing logic.
    // We will refine this later.
    const Monster* base_goblin = engine->get_db().get_monster("monster_goblin");
    if (base_goblin) {
        active_enemies.push_back(std::make_unique<Monster>(*base_goblin));
        enemy_slots[0] = active_enemies.back().get();
        
        active_enemies.push_back(std::make_unique<Monster>(*base_goblin));
        enemy_slots[1] = active_enemies.back().get();
    }
}

void BattleState::on_enter() {
    build_turn_queue();
    next_turn();
}

void BattleState::build_turn_queue() {
    turn_queue.clear();
    for (int i = 0; i < 4; ++i) {
        if (party_slots[i] && party_slots[i]->get_hp() > 0) turn_queue.push_back(party_slots[i]);
        if (enemy_slots[i] && enemy_slots[i]->get_hp() > 0) turn_queue.push_back(enemy_slots[i]);
    }
    
    // Sort by AGI descending
    std::sort(turn_queue.begin(), turn_queue.end(), [](Entity* a, Entity* b) {
        return a->get_agi() > b->get_agi();
    });
}

void BattleState::next_turn() {
    if (turn_queue.empty()) {
        build_turn_queue();
    }
    
    if (turn_queue.empty()) {
        // No one can act??
        return;
    }

    Entity* active = turn_queue.front();
    
    // Check if player or ally
    bool is_party = false;
    for (int i = 0; i < 4; ++i) {
        if (party_slots[i] == active) is_party = true;
    }

    if (is_party) {
        if (active == engine->get_player_manager().get_player()) {
            build_main_menu();
            current_phase = Phase::WAITING_FOR_ACTION;
        } else {
            // NPC Ally AI
            battle_log = active->get_name() + " attacks!";
            // Dummy logic: deal 10 damage to first enemy
            for (int i = 0; i < 4; ++i) {
                if (enemy_slots[i] && enemy_slots[i]->get_hp() > 0) {
                    enemy_slots[i]->take_damage(10);
                    break;
                }
            }
            turn_queue.erase(turn_queue.begin());
            current_phase = Phase::PROCESSING_TURN;
        }
    } else {
        execute_enemy_turn(active);
    }
}

void BattleState::execute_enemy_turn(Entity* enemy) {
    battle_log = enemy->get_name() + " attacks!";
    // Dummy logic: attack first party member
    for (int i = 0; i < 4; ++i) {
        if (party_slots[i] && party_slots[i]->get_hp() > 0) {
            party_slots[i]->take_damage(5);
            break;
        }
    }
    turn_queue.erase(turn_queue.begin());
    current_phase = Phase::PROCESSING_TURN;
}

void BattleState::build_main_menu() {
    menu_options = {
        "1. Attack",
        "2. Magic",
        "3. Special Move",
        "4. Move (Swap Slot)",
        "5. Item",
        "6. Defend",
        "7. Flee"
    };
    current_menu_selection = 0;
}

void BattleState::handle_input(int ch) {
    if (current_phase == Phase::WAITING_FOR_ACTION) {
        if (ch == KEY_UP && current_menu_selection > 0) {
            current_menu_selection--;
        } else if (ch == KEY_DOWN && current_menu_selection < (int)menu_options.size() - 1) {
            current_menu_selection++;
        } else if (ch == 10) { // Enter
            if (current_menu_selection == 0) {
                // Attack
                battle_log = "Player chose Attack! (Dealing 15 damage to slot 1)";
                if (enemy_slots[0]) enemy_slots[0]->take_damage(15);
                turn_queue.erase(turn_queue.begin());
                current_phase = Phase::PROCESSING_TURN;
            } else if (current_menu_selection == 6) {
                // Flee
                engine->pop_state(); // End battle
            }
        }
    }
}

void BattleState::update() {
    if (current_phase == Phase::PROCESSING_TURN) {
        // If turn queue was modified, we process next turn.
        // Wait a bit or require space to continue?
        // Let's just immediately go to next turn for now.
        next_turn();
    }
}

void BattleState::render() {
    view.draw(
        party_slots,
        enemy_slots,
        turn_queue,
        0, // active idx in party (we'll fix this)
        current_menu_selection,
        current_phase == Phase::WAITING_FOR_ACTION ? menu_options : std::vector<std::string>{},
        battle_log
    );
}

void BattleState::on_exit() {
    clear();
}
