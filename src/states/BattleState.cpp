#include "BattleState.hpp"
#include "../GameEngine.hpp"
#include <algorithm>

BattleState::BattleState(GameEngine* engine, const std::string& monster_group_id)
    : GameState(engine), current_menu_selection(0), current_phase(Phase::PROCESSING_TURN) {
    enemy_slots.fill(nullptr);
    // Deep copy party slots
    party_slots = engine->get_player_manager().get_party_slots();
    populate_enemies(monster_group_id);
}

void BattleState::populate_enemies(const std::string& monster_group_id) {
    const Monster* base_monster = engine->get_db().get_monster("mon_forest_troll");
    if (!base_monster) base_monster = engine->get_db().get_monster("mon_slime");
    
    if (base_monster) {
        // Add 2 enemies to slots (Trolls are tough, 2 is plenty for testing)
        for (int i = 0; i < 2; ++i) {
            active_enemies.push_back(std::make_unique<Monster>(*base_monster));
            enemy_slots[i] = active_enemies.back().get();
        }
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
    auto vec = turn_queue.to_vector();
    std::sort(vec.begin(), vec.end(), [](Entity* a, Entity* b) {
        return a->get_agi() > b->get_agi();
    });
    turn_queue.clear();
    for (auto e : vec) {
        turn_queue.push_back(e);
    }
}

void BattleState::next_turn() {
    // Check win/loss condition
    bool all_party_dead = true;
    for (int i = 0; i < 4; ++i) {
        if (party_slots[i] && party_slots[i]->get_hp() > 0) {
            all_party_dead = false;
        }
    }
    
    if (all_party_dead) {
        add_log("Your party has been defeated... GAME OVER");
        current_phase = Phase::WAITING_FOR_ACTION; // Just wait indefinitely or pop_state
        engine->pop_state();
        return;
    }

    // Process dead enemies and spawn from pool
    bool all_enemies_dead = true;
    for (int i = 0; i < 4; ++i) {
        if (enemy_slots[i]) {
            if (enemy_slots[i]->get_hp() <= 0) {
                // Enemy is dead. Clear slot.
                Entity* dead_enemy = enemy_slots[i];
                if (Monster* m = dynamic_cast<Monster*>(dead_enemy)) {
                    accumulated_exp += m->get_exp_drop();
                    accumulated_gold += m->get_gold_drop();
                    
                    // Process loot table
                    for (const auto& loot : m->get_loot_table()) {
                        int roll = rand() % 100;
                        if (roll < loot.drop_chance) {
                            engine->get_player_manager().get_player()->add_item(loot.item_id, 1);
                            // Get item name for better logging if possible, but ID works too
                            const Item* item_data = engine->get_db().get_item(loot.item_id);
                            std::string item_name = item_data ? item_data->name : loot.item_id;
                            end_battle_logs.push_back("Found loot: " + item_name + "!");
                        }
                    }
                }
                enemy_slots[i] = nullptr;
                // Remove from turn queue if present
                turn_queue.remove(dead_enemy);
            }
        }
        
        // Try to spawn if slot is empty and pool has monsters
        if (!enemy_slots[i] && !enemy_pool.empty()) {
            active_enemies.push_back(std::move(enemy_pool.front()));
            enemy_pool.erase(enemy_pool.begin());
            enemy_slots[i] = active_enemies.back().get();
            add_log(enemy_slots[i]->get_name() + " has entered the battle!");
        }
        
        if (enemy_slots[i] && enemy_slots[i]->get_hp() > 0) {
            all_enemies_dead = false;
        }
    }
    
    if (all_enemies_dead && enemy_pool.empty()) {
        add_log("All enemies defeated! YOU WIN!");
        
        // Calculate alive party members
        int alive_members = 0;
        for (int i = 0; i < 4; ++i) {
            if (party_slots[i] && party_slots[i]->get_hp() > 0) alive_members++;
        }
        
        // Push general rewards
        end_battle_logs.push_back("Found " + std::to_string(accumulated_gold) + " Gold!");
        engine->get_player_manager().get_player()->add_gold(accumulated_gold);
        
        Player* p = engine->get_player_manager().get_player();
        if (p && p->get_hp() > 0) {
            end_battle_logs.push_back(p->get_name() + " gains " + std::to_string(accumulated_exp) + " EXP!");
            p->add_exp(accumulated_exp);
            int levels_gained = p->check_level_up();
            if (levels_gained > 0) {
                end_battle_logs.push_back(p->get_name() + " leveled up to " + std::to_string(p->get_level()) + "!");
                end_battle_logs.push_back("Gained " + std::to_string(levels_gained * 5) + " Stat Points to distribute!");
            }
        }
        
        std::string popup_msg = "VICTORY!\n\n";
        for (const auto& log : end_battle_logs) {
            popup_msg += log + "\n";
        }
        popup_msg += "\nPress Space or Enter to continue...";
        
        end_popup = std::make_unique<Utils::Popup>(popup_msg);
        current_phase = Phase::BATTLE_END;
        return;
    }

    if (turn_queue.empty()) {
        build_turn_queue();
    }
    
    if (turn_queue.empty()) {
        return;
    }

    Entity* active = turn_queue.front();
    if (active->get_hp() <= 0) {
        turn_queue.advance();
        next_turn();
        return;
    }
    
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
            execute_ai_turn(active, true);
        }
    } else {
        execute_ai_turn(active, false);
    }
}

void BattleState::execute_ai_turn(Entity* active, bool is_ally) {
    skip_animations = false;
    
    // Find target
    Entity* direct_target = nullptr;
    int my_slot = -1;
    bool active_is_party = false;
    
    for (int i = 0; i < 4; ++i) {
        if (party_slots[i] == active) { my_slot = i; active_is_party = true; }
        if (enemy_slots[i] == active) { my_slot = i; active_is_party = false; }
    }
    
    auto& my_team = active_is_party ? party_slots : enemy_slots;
    auto& opp_team = active_is_party ? enemy_slots : party_slots;

    if (my_slot != -1 && opp_team[my_slot] && opp_team[my_slot]->get_hp() > 0) {
        direct_target = opp_team[my_slot];
    }
    
    if (!direct_target && my_slot != -1) {
        // Move to a slot that has an enemy in front, even if populated (swap)
        bool moved = false;
        int wait_chance = rand() % 100;
        if (wait_chance >= 20) { // 80% chance to move, 20% to wait
            for (int i = 0; i < 4; ++i) {
                if (i != my_slot && opp_team[i] && opp_team[i]->get_hp() > 0) {
                    if (active_is_party) {
                        engine->get_player_manager().swap_slots(my_slot, i);
                        party_slots = engine->get_player_manager().get_party_slots();
                    } else {
                        std::swap(my_team[my_slot], my_team[i]);
                    }
                    add_log(active->get_name() + " moves to Slot " + std::to_string(i + 1) + "!");
                    my_slot = i;
                    direct_target = opp_team[i];
                    moved = true;
                    break;
                }
            }
        }
        if (!moved) {
            add_log(active->get_name() + " waits.");
            turn_queue.advance();
            current_phase = Phase::WAITING_FOR_INPUT;
            menu_options = {"[Press any key to continue]"};
            current_menu_selection = 0;
            return;
        }
    }
    
    if (direct_target) {
        Tactic tactic = active->get_tactic();
        bool did_action = false;
        
        if (tactic == Tactic::HEAL_SUPPORT) {
            // Check for anyone below 50% HP
            Entity* need_heal = nullptr;
            for(int i=0; i<4; i++) {
                if (my_team[i] && my_team[i]->get_hp() > 0 && my_team[i]->get_hp() < my_team[i]->get_max_hp() * 0.5) {
                    need_heal = my_team[i]; break;
                }
            }
            if (need_heal) {
                // Find healing magic
                for (size_t i = 0; i < active->get_magics().size(); ++i) {
                    const auto& m = active->get_magics()[i];
                    if ((m.type == MagicType::HEALING || m.type == MagicType::SUPPORT) && active->get_mp() >= m.mana_cost) {
                        selected_magic_idx = i;
                        pending_action = "Magic";
                        Entity* t = need_heal;
                        if (m.range == TargetRange::AOE) t = nullptr;
                        execute_action(t);
                        did_action = true;
                        break;
                    }
                }
            }
            if (!did_action) {
                // If everyone is fine or no healing magic, maybe defend or weak attack
                int roll = rand() % 100;
                if (roll < 50) {
                    pending_action = "Defend";
                    execute_action(active);
                    did_action = true;
                }
            }
        } else if (tactic == Tactic::FULL_ASSAULT) {
            // Find highest damage magic/special
            int best_dmg = -1;
            int best_magic_idx = -1;
            for (size_t i = 0; i < active->get_magics().size(); ++i) {
                const auto& m = active->get_magics()[i];
                if (m.type == MagicType::ATTACKING && active->get_mp() >= m.mana_cost && m.power > best_dmg) {
                    best_dmg = m.power;
                    best_magic_idx = i;
                }
            }
            // Also check special
            if (active->has_special() && active->get_special_move().power > best_dmg) {
                int roll = rand() % 100;
                if (roll < 50) { // 50% chance to use special if it's strong
                    pending_action = "Special";
                    Entity* t = direct_target;
                    if (active->get_special_move().range == TargetRange::AOE) t = nullptr;
                    execute_action(t);
                    did_action = true;
                }
            }
            if (!did_action && best_magic_idx != -1) {
                selected_magic_idx = best_magic_idx;
                pending_action = "Magic";
                Entity* t = direct_target;
                if (active->get_magics()[best_magic_idx].range == TargetRange::AOE) t = nullptr;
                execute_action(t);
                did_action = true;
            }
        } else if (tactic == Tactic::CONSERVE_SP) {
            // Only basic attacks
            pending_action = "Attack";
            execute_action(direct_target);
            did_action = true;
        } 
        
        // ACT_FREELY or fallback
        if (!did_action) {
            int action = rand() % 100;
            if (action < 20 && active->has_special()) {
                pending_action = "Special";
                Entity* t = direct_target;
                if (active->get_special_move().range == TargetRange::AOE) t = nullptr;
                else if (active->get_special_move().range == TargetRange::REACH) {
                    std::vector<Entity*> available;
                    for(int i=0; i<4; i++) if (opp_team[i] && opp_team[i]->get_hp()>0) available.push_back(opp_team[i]);
                    if (!available.empty()) t = available[rand() % available.size()];
                }
                execute_action(t);
            } else if (action < 60 && !active->get_magics().empty()) {
                selected_magic_idx = rand() % active->get_magics().size();
                const Magic& m = active->get_magics()[selected_magic_idx];
                if (active->get_mp() >= m.mana_cost) {
                    pending_action = "Magic";
                    Entity* t = direct_target;
                    if (m.range == TargetRange::AOE) t = nullptr;
                    else if (m.type == MagicType::HEALING) {
                        Entity* need_heal = active;
                        for(int i=0; i<4; i++) {
                            if (my_team[i] && my_team[i]->get_hp() > 0 && my_team[i]->get_hp() < my_team[i]->get_max_hp()) {
                                need_heal = my_team[i]; break;
                            }
                        }
                        t = need_heal;
                    }
                    else if (m.range == TargetRange::REACH) {
                        std::vector<Entity*> available;
                        for(int i=0; i<4; i++) if (opp_team[i] && opp_team[i]->get_hp()>0) available.push_back(opp_team[i]);
                        if (!available.empty()) t = available[rand() % available.size()];
                    }
                    execute_action(t);
                } else {
                    pending_action = "Attack";
                    execute_action(direct_target);
                }
            } else {
                pending_action = "Attack";
                execute_action(direct_target);
            }
        }
    } else {
        add_log(active->get_name() + " waits.");
        turn_queue.advance();
        current_phase = Phase::WAITING_FOR_INPUT;
        menu_options = {"[Press any key to continue]"};
        current_menu_selection = 0;
    }
}

void BattleState::build_main_menu() {
    menu_options = {
        "1. Attack",
        "2. Magic",
        "3. Special Move",
        "4. Move (Swap Slot)",
        "5. Item",
        "6. Defend",
        "7. Tactics",
        "8. Flee"
    };
    current_menu_selection = 0;
}

void BattleState::build_tactic_member_menu() {
    menu_options.clear();
    for (int i = 0; i < 4; ++i) {
        if (party_slots[i] && party_slots[i] != engine->get_player_manager().get_player()) {
            menu_options.push_back("Slot " + std::to_string(i + 1) + ": " + party_slots[i]->get_name() + " [" + tactic_to_string(party_slots[i]->get_tactic()) + "]");
        } else if (party_slots[i] && party_slots[i] == engine->get_player_manager().get_player()) {
            menu_options.push_back("Slot " + std::to_string(i + 1) + ": " + party_slots[i]->get_name() + " (Player - N/A)");
        } else {
            menu_options.push_back("Slot " + std::to_string(i + 1) + ": Empty");
        }
    }
    menu_options.push_back("Cancel");
    current_menu_selection = 0;
}

void BattleState::build_tactic_type_menu() {
    menu_options = {
        tactic_to_string(Tactic::ACT_FREELY),
        tactic_to_string(Tactic::FULL_ASSAULT),
        tactic_to_string(Tactic::HEAL_SUPPORT),
        tactic_to_string(Tactic::CONSERVE_SP),
        "Cancel"
    };
    current_menu_selection = 0;
}

void BattleState::build_enemy_target_menu() {
    menu_options.clear();
    for (int i = 0; i < 4; ++i) {
        if (enemy_slots[i] && enemy_slots[i]->get_hp() > 0) {
            menu_options.push_back("Slot " + std::to_string(i + 1) + ": " + enemy_slots[i]->get_name());
        } else {
            menu_options.push_back("Slot " + std::to_string(i + 1) + ": (Kosong/Mati)");
        }
    }
    menu_options.push_back("Cancel");
    current_menu_selection = 0;
}

void BattleState::build_ally_target_menu() {
    menu_options.clear();
    for (int i = 0; i < 4; ++i) {
        if (party_slots[i]) {
            menu_options.push_back("Slot " + std::to_string(i + 1) + ": " + party_slots[i]->get_name());
        } else {
            menu_options.push_back("Slot " + std::to_string(i + 1) + ": (Kosong)");
        }
    }
    menu_options.push_back("Cancel");
    current_menu_selection = 0;
}


void BattleState::build_swap_slot_menu() {
    menu_options.clear();
    for (int i = 0; i < 4; ++i) {
        if (party_slots[i]) {
            menu_options.push_back("Swap with Slot " + std::to_string(i + 1) + ": " + party_slots[i]->get_name());
        } else {
            menu_options.push_back("Move to Slot " + std::to_string(i + 1) + " (Kosong)");
        }
    }
    menu_options.push_back("Cancel");
    current_menu_selection = 0;
}

void BattleState::build_magic_menu() {
    menu_options.clear();
    Entity* active = turn_queue.front();
    const auto& magics = active->get_magics();
    for (size_t i = 0; i < magics.size(); ++i) {
        menu_options.push_back(magics[i].name + " (MP: " + std::to_string(magics[i].mana_cost) + ")");
    }
    menu_options.push_back("Cancel");
    current_menu_selection = 0;
}

void BattleState::build_item_menu() {
    menu_options.clear();
    current_item_menu_ids.clear();
    
    auto& inv = engine->get_player_manager().get_player()->get_inventory();
    auto& db = engine->get_db();
    
    for (const auto& [item_id, count] : inv) {
        if (count > 0) {
            const Item* itm = db.get_item(item_id);
            if (itm && itm->type == ItemType::CONSUMABLE) {
                menu_options.push_back(itm->name + " (x" + std::to_string(count) + ") - " + itm->description);
                current_item_menu_ids.push_back(item_id);
            }
        }
    }
    
    if (menu_options.empty()) {
        menu_options.push_back("No usable items in inventory.");
    }
    
    menu_options.push_back("Cancel");
    current_menu_selection = 0;
}

void BattleState::execute_action(Entity* target) {
    skip_animations = false;
    Entity* active = turn_queue.front();

    if (pending_action == "Attack") {
        if (target) {
            std::string w_type = active->get_weapon_type();
            std::string w_name = active->get_weapon_name();
            
            int dmg = 1;
            std::string log_msg = "";
            
            if (w_type == "sword") {
                dmg = std::max(1, static_cast<int>(active->get_str() * 1.1) - target->get_cons());
                log_msg = active->get_name() + " menebas " + target->get_name() + " dengan " + w_name + " for " + std::to_string(dmg) + " damage!";
            } else if (w_type == "dagger") {
                dmg = std::max(1, active->get_agi() - target->get_cons());
                log_msg = active->get_name() + " menusuk " + target->get_name() + " dengan " + w_name + " for " + std::to_string(dmg) + " damage!";
                // Dagger has a 20% chance to attack twice
                if (rand() % 100 < 20) {
                    log_msg += " (Double Hit!)";
                    dmg *= 2;
                }
            } else if (w_type == "bow") {
                dmg = std::max(1, (active->get_str() + active->get_agi()) / 2 - target->get_cons());
                log_msg = active->get_name() + " memanah " + target->get_name() + " dengan " + w_name + " for " + std::to_string(dmg) + " damage! (REACH Effect)";
            } else if (w_type == "staff") {
                dmg = std::max(1, static_cast<int>(active->get_intl() * 1.2) - target->get_wis());
                log_msg = active->get_name() + " menembakkan energi sihir dari " + w_name + " ke " + target->get_name() + " for " + std::to_string(dmg) + " damage!";
            } else { // unarmed or others
                dmg = std::max(1, active->get_str() - target->get_cons());
                log_msg = active->get_name() + " memukul " + target->get_name() + " dengan tangan kosong for " + std::to_string(dmg) + " damage!";
            }
            
            target->take_damage(dmg);
            animate_hit(target);
            add_log(log_msg);
        }
    } else if (pending_action == "Magic") {
        if (selected_magic_idx >= 0 && selected_magic_idx < (int)active->get_magics().size()) {
            const Magic& m = active->get_magics()[selected_magic_idx];
            active->consume_mp(m.mana_cost);
            if (m.range == TargetRange::AOE) {
                bool is_healing = (m.type == MagicType::HEALING || m.type == MagicType::SUPPORT);
                add_log(active->get_name() + " casts " + m.name + " (AOE)!");
                
                // Determine if active is player party or enemy party to know who is 'ally'
                bool active_is_party = false;
                for(int i=0; i<4; i++) if (party_slots[i] == active) active_is_party = true;
                
                auto& my_team = active_is_party ? party_slots : enemy_slots;
                auto& opp_team = active_is_party ? enemy_slots : party_slots;

                for (int i = 0; i < 4; ++i) {
                    Entity* t = is_healing ? my_team[i] : opp_team[i];
                    if (t && t->get_hp() > 0) {
                        if (m.type == MagicType::HEALING) {
                            int base_heal = static_cast<int>(active->get_wis() * 1.5) + m.power;
                            t->heal_hp(base_heal);
                        } else if (m.type == MagicType::ATTACKING) {
                            int base_dmg = std::max(1, static_cast<int>(active->get_intl() * 1.5) + m.power - t->get_wis());
                            float mult = get_elemental_multiplier(m.elem, t->get_affinity(), t->get_weakness());
                            int total_dmg = static_cast<int>(base_dmg * mult);
                            t->take_damage(total_dmg);
                            animate_hit(t);
                        }
                    }
                }
            } else if (target) {
                if (m.type == MagicType::HEALING) {
                    int base_heal = static_cast<int>(active->get_wis() * 1.5) + m.power;
                    target->heal_hp(base_heal);
                    add_log(active->get_name() + " casts " + m.name + " on " + target->get_name() + " for " + std::to_string(base_heal) + " HP!");
                } else if (m.type == MagicType::ATTACKING) {
                    int base_dmg = std::max(1, static_cast<int>(active->get_intl() * 1.5) + m.power - target->get_wis());
                    float mult = get_elemental_multiplier(m.elem, target->get_affinity(), target->get_weakness());
                    int total_dmg = static_cast<int>(base_dmg * mult);
                    target->take_damage(total_dmg);
                    animate_hit(target);
                    std::string log_msg = active->get_name() + " casts " + m.name + " on " + target->get_name() + " for " + std::to_string(total_dmg) + " damage!";
                    if (mult > 1.1f) log_msg += " It's super effective!";
                    else if (mult < 0.9f) log_msg += " It's not very effective...";
                    add_log(log_msg);
                } else {
                    add_log(active->get_name() + " casts " + m.name + " on " + target->get_name() + "!");
                }
            }
        }
    } else if (pending_action == "Special") {
        if (active->has_special()) {
            const SpecialMove& sm = active->get_special_move();
            
            bool active_is_party = false;
            for(int i=0; i<4; i++) if (party_slots[i] == active) active_is_party = true;
            auto& opp_team = active_is_party ? enemy_slots : party_slots;

            if (sm.range == TargetRange::AOE) {
                add_log(active->get_name() + " uses Special Move: " + sm.name + " (AOE)!");
                for (int i = 0; i < 4; ++i) {
                    Entity* t = opp_team[i];
                    if (t && t->get_hp() > 0) {
                        int base_dmg = std::max(1, static_cast<int>(active->get_str() * 1.2) + sm.power - t->get_cons());
                        float mult = get_elemental_multiplier(sm.elem, t->get_affinity(), t->get_weakness());
                        int total_dmg = static_cast<int>(base_dmg * mult);
                        t->take_damage(total_dmg);
                        animate_hit(t);
                    }
                }
            } else if (target) {
                int base_dmg = std::max(1, static_cast<int>(active->get_str() * 1.2) + sm.power - target->get_cons());
                float mult = get_elemental_multiplier(sm.elem, target->get_affinity(), target->get_weakness());
                int total_dmg = static_cast<int>(base_dmg * mult);
                target->take_damage(total_dmg);
                animate_hit(target);
                std::string log_msg = active->get_name() + " uses Special Move: " + sm.name + " on " + target->get_name() + " for " + std::to_string(total_dmg) + " damage!";
                if (mult > 1.1f) log_msg += " Critical Hit!";
                else if (mult < 0.9f) log_msg += " Blocked!";
                add_log(log_msg);
            }
        }
    } else if (pending_action == "Item") {
        if (target) {
            target->heal_hp(50);
            add_log("Used Item on " + target->get_name() + ", healing 50 HP.");
        }
    } else if (pending_action == "Item") {
        if (!selected_item_id.empty()) {
            const Item* itm = engine->get_db().get_item(selected_item_id);
            if (itm) {
                bool effect_applied = false;
                for (const auto& effect : itm->on_use) {
                    if (effect.find("add_hp ") == 0 || effect.find("heal_hp ") == 0) {
                        int amount = std::stoi(effect.substr(effect.find(" ") + 1));
                        target->heal_hp(amount);
                        add_log(active->get_name() + " uses " + itm->name + " on " + target->get_name() + " to restore " + std::to_string(amount) + " HP!");
                        effect_applied = true;
                    } else if (effect.find("add_mp ") == 0 || effect.find("heal_mp ") == 0) {
                        int amount = std::stoi(effect.substr(effect.find(" ") + 1));
                        target->restore_mp(amount);
                        add_log(active->get_name() + " uses " + itm->name + " on " + target->get_name() + " to restore " + std::to_string(amount) + " MP!");
                        effect_applied = true;
                    }
                }
                if (!effect_applied) {
                    add_log(active->get_name() + " uses " + itm->name + " on " + target->get_name() + ", but nothing happened.");
                }
                // Decrement from player's inventory
                engine->get_player_manager().get_player()->remove_item(selected_item_id, 1);
            }
        }
    } else if (pending_action == "Defend") {
        add_log(active->get_name() + " is defending!");
    }

    turn_queue.advance();
    current_phase = Phase::WAITING_FOR_INPUT;
    menu_options = {"[Press any key to continue]"};
    current_menu_selection = 0;
}

void BattleState::handle_input(int ch) {
    if (ch == -1) return; // Ignore no-input

    if (ch == KEY_RESIZE) {
        engine->get_layout().resize();
        if (end_popup) end_popup->resize();
        return;
    }

    if (current_phase == Phase::BATTLE_END) {
        if (end_popup) {
            end_popup->handle_input(ch);
        }
        return;
    }

    if (current_phase == Phase::WAITING_FOR_INPUT) {
        if (ch == 10) { // Enter
            current_phase = Phase::PROCESSING_TURN;
            menu_options.clear();
        }
        return;
    }

    if (menu_options.empty()) return;
    
    if (ch == KEY_UP && current_menu_selection > 0) {
        current_menu_selection--;
        return;
    } else if (ch == KEY_DOWN && current_menu_selection < (int)menu_options.size() - 1) {
        current_menu_selection++;
        return;
    } else if (ch != 10) {
        return; // Only process enter key beyond this point
    }

    // Now we know it's Enter (10)
    Entity* active = turn_queue.front();

    if (current_phase == Phase::WAITING_FOR_ACTION) {
        std::string sel = menu_options[current_menu_selection];
        if (sel == "1. Attack") {
            int my_slot = -1;
            for (int i = 0; i < 4; ++i) if (party_slots[i] == active) my_slot = i;
            
            if (my_slot != -1 && enemy_slots[my_slot] && enemy_slots[my_slot]->get_hp() > 0) {
                pending_action = "Attack";
                execute_action(enemy_slots[my_slot]);
            } else {
                add_log("No target in front to attack!");
            }
        } else if (sel == "2. Magic") {
            pending_action = "Magic";
            current_phase = Phase::SELECTING_MAGIC;
            build_magic_menu();
        } else if (sel == "3. Special Move") {
            if (!active->has_special()) {
                add_log("No special move available!");
            } else {
                pending_action = "Special";
                const SpecialMove& sm = active->get_special_move();
                if (sm.range == TargetRange::AOE) {
                    execute_action(nullptr);
                } else if (sm.range == TargetRange::DIRECT) {
                    int my_slot = -1;
                    for (int i = 0; i < 4; ++i) if (party_slots[i] == active) my_slot = i;
                    if (my_slot != -1 && enemy_slots[my_slot] && enemy_slots[my_slot]->get_hp() > 0) {
                        execute_action(enemy_slots[my_slot]);
                    } else {
                        add_log("No target in front for direct special move!");
                    }
                } else {
                    current_phase = Phase::SELECTING_TARGET_ENEMY;
                    build_enemy_target_menu();
                }
            }
        } else if (sel == "4. Move (Swap Slot)") {
            current_phase = Phase::SELECTING_SWAP_SLOT;
            pending_action = "Swap";
            build_swap_slot_menu();
        } else if (sel == "5. Item") {
            pending_action = "Item";
            current_phase = Phase::SELECTING_ITEM;
            build_item_menu();
        } else if (sel == "6. Defend") {
            pending_action = "Defend";
            execute_action(active);
        } else if (sel == "7. Tactics") {
            current_phase = Phase::SELECTING_TACTIC_MEMBER;
            build_tactic_member_menu();
        } else if (sel == "8. Flee") {
            add_log(active->get_name() + " flees the battle!");
            engine->pop_state();
            return;
        }
    } else if (current_phase == Phase::SELECTING_TARGET_ENEMY) {
        if (current_menu_selection == (int)menu_options.size() - 1) { // Cancel
            current_phase = Phase::WAITING_FOR_ACTION;
            build_main_menu();
        } else {
            Entity* target = enemy_slots[current_menu_selection];
            if (target && target->get_hp() > 0) {
                execute_action(target);
            } else {
                add_log("Invalid target!");
            }
        }
    } else if (current_phase == Phase::SELECTING_TARGET_ALLY) {
        if (current_menu_selection == (int)menu_options.size() - 1) { // Cancel
            current_phase = Phase::WAITING_FOR_ACTION;
            build_main_menu();
        } else {
            Entity* target = party_slots[current_menu_selection];
            if (target) {
                execute_action(target);
            }
        }
    } else if (current_phase == Phase::SELECTING_SWAP_SLOT) {
        if (current_menu_selection == (int)menu_options.size() - 1) { // Cancel
            current_phase = Phase::WAITING_FOR_ACTION;
            build_main_menu();
        } else {
            int active_slot = -1;
            for (int i = 0; i < 4; ++i) if (party_slots[i] == active) active_slot = i;
            if (active_slot != -1) {
                engine->get_player_manager().swap_slots(active_slot, current_menu_selection);
                party_slots = engine->get_player_manager().get_party_slots();
                add_log(active->get_name() + " moves to Slot " + std::to_string(current_menu_selection + 1) + "!");
            }
            // Move is a free action, so go back to main menu
            current_phase = Phase::WAITING_FOR_ACTION;
            build_main_menu();
        }
    } else if (current_phase == Phase::SELECTING_ITEM) {
        if (current_menu_selection == (int)menu_options.size() - 1 || current_item_menu_ids.empty()) { // Cancel or empty
            current_phase = Phase::WAITING_FOR_ACTION;
            build_main_menu();
        } else if (current_menu_selection >= 0 && current_menu_selection < (int)current_item_menu_ids.size()) {
            selected_item_id = current_item_menu_ids[current_menu_selection];
            current_phase = Phase::SELECTING_TARGET_ALLY;
            build_ally_target_menu();
        }
    } else if (current_phase == Phase::SELECTING_MAGIC) {
        if (current_menu_selection == (int)menu_options.size() - 1) { // Cancel
            current_phase = Phase::WAITING_FOR_ACTION;
            build_main_menu();
        } else {
            selected_magic_idx = current_menu_selection;
            const Magic& m = active->get_magics()[selected_magic_idx];
            if (active->get_mp() >= m.mana_cost) {
                if (m.range == TargetRange::AOE) {
                    execute_action(nullptr);
                } else if (m.range == TargetRange::DIRECT) {
                    int my_slot = -1;
                    for (int i = 0; i < 4; ++i) if (party_slots[i] == active) my_slot = i;
                    Entity* target = nullptr;
                    if (my_slot != -1) {
                        target = (m.type == MagicType::HEALING || m.type == MagicType::SUPPORT) ? party_slots[my_slot] : enemy_slots[my_slot];
                    }
                    if (target && target->get_hp() > 0) {
                        execute_action(target);
                    } else {
                        add_log("No target in front for this direct magic!");
                    }
                } else {
                    if (m.type == MagicType::HEALING || m.type == MagicType::SUPPORT) {
                        current_phase = Phase::SELECTING_TARGET_ALLY;
                        build_ally_target_menu();
                    } else {
                        current_phase = Phase::SELECTING_TARGET_ENEMY;
                        build_enemy_target_menu();
                    }
                }
            } else {
                add_log("Not enough MP!");
            }
        }
    } else if (current_phase == Phase::SELECTING_SWAP_SLOT) {
        if (current_menu_selection == (int)menu_options.size() - 1) { // Cancel
            current_phase = Phase::WAITING_FOR_ACTION;
            build_main_menu();
        } else {
            int target_slot = current_menu_selection;
            int my_slot = -1;
            for (int i = 0; i < 4; ++i) if (party_slots[i] == active) my_slot = i;
            
            if (my_slot != -1 && my_slot != target_slot) {
                engine->get_player_manager().swap_slots(my_slot, target_slot);
                party_slots = engine->get_player_manager().get_party_slots();
                add_log(active->get_name() + " moved to Slot " + std::to_string(target_slot + 1) + "!");
            } else {
                add_log("Invalid move!");
            }
            // Move is a free action, do not erase from turn_queue!
            current_phase = Phase::WAITING_FOR_ACTION;
            build_main_menu();
        }
    } else if (current_phase == Phase::SELECTING_ITEM) {
        if (current_menu_selection == (int)menu_options.size() - 1) { // Cancel
            current_phase = Phase::WAITING_FOR_ACTION;
            build_main_menu();
        } else {
            selected_item_idx = current_menu_selection;
            current_phase = Phase::SELECTING_TARGET_ALLY;
            build_ally_target_menu();
        }
    } else if (current_phase == Phase::SELECTING_TACTIC_MEMBER) {
        if (current_menu_selection == (int)menu_options.size() - 1) { // Cancel
            current_phase = Phase::WAITING_FOR_ACTION;
            build_main_menu();
        } else {
            if (party_slots[current_menu_selection] && party_slots[current_menu_selection] != engine->get_player_manager().get_player()) {
                selected_tactic_member = current_menu_selection;
                current_phase = Phase::SELECTING_TACTIC_TYPE;
                build_tactic_type_menu();
            } else {
                // Invalid selection (player or empty)
                // Just stay here
            }
        }
    } else if (current_phase == Phase::SELECTING_TACTIC_TYPE) {
        if (current_menu_selection == (int)menu_options.size() - 1) { // Cancel
            current_phase = Phase::SELECTING_TACTIC_MEMBER;
            build_tactic_member_menu();
        } else {
            Tactic chosen = Tactic::ACT_FREELY;
            if (current_menu_selection == 0) chosen = Tactic::ACT_FREELY;
            else if (current_menu_selection == 1) chosen = Tactic::FULL_ASSAULT;
            else if (current_menu_selection == 2) chosen = Tactic::HEAL_SUPPORT;
            else if (current_menu_selection == 3) chosen = Tactic::CONSERVE_SP;
            
            party_slots[selected_tactic_member]->set_tactic(chosen);
            
            // Go back to main menu
            current_phase = Phase::WAITING_FOR_ACTION;
            build_main_menu();
        }
    }
}

void BattleState::update() {
    if (current_phase == Phase::BATTLE_END) {
        if (end_popup) {
            end_popup->update();
            if (end_popup->is_dismissed()) {
                engine->pop_state();
            }
        }
        return;
    }

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
        turn_queue.to_vector(),
        0, // active idx in party (we'll fix this)
        current_menu_selection,
        menu_options,
        battle_log,
        enemy_pool.size()
    );

    if (current_phase == Phase::BATTLE_END && end_popup) {
        end_popup->render();
    }

    doupdate();
}

void BattleState::on_exit() {
    clear();
}

void BattleState::add_log(const std::string& msg) {
    battle_log.push_back("");
    if (battle_log.size() > 50) {
        battle_log.erase(battle_log.begin());
    }

    // Typing animation
    for (char c : msg) {
        battle_log.back() += c;
        if (!skip_animations) {
            view.draw(
                party_slots, enemy_slots, turn_queue.to_vector(), 0,
                current_menu_selection, menu_options,
                battle_log, enemy_pool.size(), nullptr
            );
            doupdate();
            int ch = getch();
            if (ch != ERR) skip_animations = true;
            if (!skip_animations) napms(15);
        }
    }
    
    if (skip_animations) {
        view.draw(
            party_slots, enemy_slots, turn_queue.to_vector(), 0,
            current_menu_selection, menu_options,
            battle_log, enemy_pool.size(), nullptr
        );
        while (getch() != ERR) {} // flush remaining input so it doesn't instantly skip the WAIT phase
    }
}

void BattleState::animate_hit(Entity* target) {
    for (int i = 0; i < 3; ++i) {
        if (skip_animations) break;
        // Flash ON
        view.draw(
            party_slots, enemy_slots, turn_queue.to_vector(), 0,
            current_menu_selection, menu_options,
            battle_log, enemy_pool.size(), target
        );
        doupdate();
        int ch = getch();
        if (ch != ERR) skip_animations = true;
        if (!skip_animations) napms(60);
        
        if (skip_animations) break;
        // Flash OFF
        view.draw(
            party_slots, enemy_slots, turn_queue.to_vector(), 0,
            current_menu_selection, menu_options,
            battle_log, enemy_pool.size(), nullptr
        );
        doupdate();
        ch = getch();
        if (ch != ERR) skip_animations = true;
        if (!skip_animations) napms(60);
    }
    if (skip_animations) {
        view.draw(
            party_slots, enemy_slots, turn_queue.to_vector(), 0,
            current_menu_selection, menu_options,
            battle_log, enemy_pool.size(), nullptr
        );
        doupdate();
    }
}
