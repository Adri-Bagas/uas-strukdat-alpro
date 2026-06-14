#include "TownState.hpp"
#include "../utils/components/Popup.hpp"
#include "../GameEngine.hpp" // Full include allowed in .cpp
#include "../utils/Logger.hpp"
#include <ncurses.h>

TownState::TownState(GameEngine* eng) : GameState(eng) {}

void TownState::on_enter() {
    Logger::log("TownState: Entering state.");
    selection_index = 0;
    Place* cur = engine->get_places().get_current_place();
    if (!cur) return;

    engine->get_places().update_npc_locations(engine->get_calendar(), engine->get_quests());
    
    // Identity discovery for on_enter scenes
    std::string scene_id = cur->get_on_enter();
    if (!cur->get_has_entered()) {
        if (!cur->get_on_first_enter().empty()) {
            scene_id = cur->get_on_first_enter();
        }
        cur->set_has_entered(true);
    }

    if (!scene_id.empty()) {
        const DialogScene* scene = engine->get_db().get_dialog_scene(scene_id);
        if (scene) {
            engine->get_dialogs().set_on_exit(scene->on_exit);
            engine->get_dialogs().set_next_scene(scene->next_scene_id);
            for (auto node : scene->nodes) {
                // Identity discovery logic
                if (!node.npc_name.empty()) {
                    const NPC* npc = engine->get_db().get_npc(node.npc_name);
                    if (npc && !npc->known()) {
                        node.npc_name = "???";
                    }
                }
                engine->get_dialogs().queue_dialog(node);
            }
        } else {
            Logger::log("TownState ERROR: Entry scene '" + scene_id + "' not found!");
        }
    }
    this->render();
}

void TownState::handle_input(int ch) {
    // Block input during dialogue sequence
    if (engine->get_dialogs().has_queued_dialog()) return;

    if (ch == 'q') {
        engine->quit(); 
    } 
    else if (ch == KEY_RESIZE) {
        engine->get_layout().resize();
    }

    // A. QUEST MENU MODE
    if (is_in_quest_menu) {
        int total_quest_options = available_quests.size() + 1; // +1 for Exit

        if (ch == KEY_UP || ch == 'w') {
            if (quest_selection_index > 0) quest_selection_index--;
        }
        else if (ch == KEY_DOWN || ch == 's') {
            if (quest_selection_index < total_quest_options - 1) quest_selection_index++;
        }
        else if (ch == '\n' || ch == ' ') {
            // Exit option (last one)
            if (quest_selection_index == (int)available_quests.size()) {
                interacting_npc = nullptr;
                available_quests.clear();
                is_in_quest_menu = false;
                selection_index = 0;
            } 
            // Select a quest
            else {
                Quest* q = available_quests[quest_selection_index];
                Player* p = engine->get_player_manager().get_player();
                
                std::string scene_id = "";
                if (q->can_complete(p)) {
                    scene_id = q->get_complete_scene();
                } else if (q->get_state() == QuestState::AVAILABLE) {
                    scene_id = q->get_start_scene();
                }

                if (!scene_id.empty()) {
                    const DialogScene* scene = engine->get_db().get_dialog_scene(scene_id);
                    if (scene) {
                        is_in_quest_menu = false; 
                        if (q->can_complete(p)) {
                            engine->get_actions().execute("complete_quest " + q->get_id());
                        }

                        for (auto node : scene->nodes) {
                            if (!node.npc_name.empty()) {
                                const NPC* n_ptr = engine->get_db().get_npc(node.npc_name);
                                if (n_ptr && !n_ptr->known()) node.npc_name = "???";
                            }
                            engine->get_dialogs().queue_dialog(node);
                        }
                    }
                } else {
                    if (q->get_state() == QuestState::IN_PROGRESS) {
                        Popup p_box {"Kamu masih mengerjakan ini: " + q->get_description()};
                        p_box.animate(); p_box.type_text();
                    } else {
                        Popup p_box {"Tahap misi ini belum memiliki cerita."};
                        p_box.animate(); p_box.type_text();
                    }
                }
            }
        }
        return; 
    }

    // B. STANDARD WORLD MODE
    int total_options = current_npcs.size() + current_activities.size() + current_exits.size();

    if (ch == KEY_UP || ch == 'w') {
        if (selection_index > 0) selection_index--;
    }
    else if (ch == KEY_DOWN || ch == 's') {
        if (selection_index < total_options - 1) selection_index++;
    }
    else if (ch == '\n' || ch == ' ') {
        if (selection_index < 0) return;

        // 1. Check if NPC was selected
        if (selection_index < (int)current_npcs.size()) {
            interacting_npc = current_npcs[selection_index];
            Logger::log("TownState: Talking to " + interacting_npc->get_name());

            // Check for quests
            Player* p = engine->get_player_manager().get_player();
            engine->get_quests().check_npc_quests(interacting_npc, p);
            
            available_quests = engine->get_quests().get_available_quests_for_npc(interacting_npc->get_id());
            is_in_quest_menu = false;
            quest_selection_index = 0;

            // Play dialogue
            std::string dialog_id = interacting_npc->get_default_dialog();
            if (!dialog_id.empty()) {
                const DialogScene* scene = engine->get_db().get_dialog_scene(dialog_id);
                if (scene) {
                    engine->get_dialogs().set_on_exit(scene->on_exit);
                    engine->get_dialogs().set_next_scene(scene->next_scene_id);
                    for (auto node : scene->nodes) {
                        if (!node.npc_name.empty()) {
                            const NPC* n_ptr = engine->get_db().get_npc(node.npc_name);
                            if (n_ptr && !n_ptr->known()) node.npc_name = "???";
                        }
                        engine->get_dialogs().queue_dialog(node);
                    }
                }
            } else {
                if (available_quests.empty()) {
                    Popup pop {"Karakter ini tidak memiliki apa pun untuk dikatakan."};
                    pop.animate(); pop.type_text();
                    interacting_npc = nullptr;
                } else {
                    is_in_quest_menu = true;
                }
            }
        }
        // 2. Check if Activity was selected
        else if (selection_index < (int)(current_npcs.size() + current_activities.size())) {
            int act_idx = selection_index - current_npcs.size();
            const auto& act = current_activities[act_idx];
            
            bool actually_locked = act.is_locked || (engine->get_calendar().getDay() == 1);
            if (actually_locked) {
                std::string msg = act.dialog_locked.empty() ? "Aktivitas ini sedang terkunci." : act.dialog_locked;
                Popup p {msg}; p.animate(); p.type_text(); return;
            }

            Player* p = engine->get_player_manager().get_player();
            bool can_do = true;
            std::string fail_msg = act.dialog_fail;
            for (const auto& [stat, req_val] : act.req_stats) {
                int current_val = 0;
                if (stat == "str") current_val = p->get_str();
                else if (stat == "cons") current_val = p->get_cons();
                else if (stat == "agi") current_val = p->get_agi();
                else if (stat == "intl") current_val = p->get_intl();
                else if (stat == "wis") current_val = p->get_wis();
                if (current_val < req_val) {
                    can_do = false;
                    if (fail_msg.empty()) fail_msg = "Status tidak mencukupi! Butuh " + stat + " " + std::to_string(req_val);
                    break;
                }
            }
            if (!can_do) { Popup pop {fail_msg}; pop.animate(); pop.type_text(); return; }

            Logger::log("TownState: Executing activity " + act.id);
            if (!act.dialog_success.empty()) { Popup pop {act.dialog_success}; pop.animate(); pop.type_text(); }
            for (const auto& action : act.on_execute) engine->get_actions().execute(action);
        }
        // 3. Check if Exit (Movement) was selected
        else {
            int exit_idx = selection_index - (current_npcs.size() + current_activities.size());
            Place* target = current_exits[exit_idx];
            Logger::log("TownState: Traveling to " + target->get_id());
            
            if (engine->get_places().travel(target->get_id())) {
                on_enter();
            }
        }
    }
}

void TownState::update() {
    if (engine->get_dialogs().has_queued_dialog()) {
        Logger::log("TownState: Processing dialog queue node.");
        DialogNode node = engine->get_dialogs().pop_dialog();
        
        if (node.type == 1) {
            engine->get_layout().type_new_text(
                engine->get_layout().win_dialog, "Dialog",
                engine->get_layout().w_left, engine->get_dialogs().get_combined_log(), node
            );
            engine->get_dialogs().add_dialog(node);
        } else if (node.type == 2) {
            engine->get_layout().type_new_text(
                engine->get_layout().win_dialog, "Dialog",
                engine->get_layout().w_left, engine->get_dialogs().get_combined_log(), node
            );
            engine->get_dialogs().add_thought(node);
        } else if (node.type == 3) {
            Popup otp {node.value};
            otp.animate();
            otp.type_text();
            engine->get_dialogs().add_popup(node);
        }
        
        this->render();

        if (!engine->get_dialogs().has_queued_dialog()) {
            DialogNode separator {"--------------------------------", "", 0};
            engine->get_dialogs().add_dialog(separator);

            std::vector<std::string> exit_actions = engine->get_dialogs().get_on_exit();
            for (const auto& action : exit_actions) {
                engine->get_actions().execute(action);
            }

            std::string next_id = engine->get_dialogs().get_next_scene();
            if (!next_id.empty()) {
                const DialogScene* next_scene = engine->get_db().get_dialog_scene(next_id);
                if (next_scene) {
                    engine->get_dialogs().set_on_exit(next_scene->on_exit);
                    engine->get_dialogs().set_next_scene(next_scene->next_scene_id);
                    for (auto next_node : next_scene->nodes) {
                        if (!next_node.npc_name.empty()) {
                            const NPC* n_ptr = engine->get_db().get_npc(next_node.npc_name);
                            if (n_ptr && !n_ptr->known()) next_node.npc_name = "???";
                        }
                        engine->get_dialogs().queue_dialog(next_node);
                    }
                }
            } else {
                engine->get_dialogs().set_on_exit({});
                engine->get_dialogs().set_next_scene("");
            }

            if (interacting_npc) {
                if (!available_quests.empty()) {
                    is_in_quest_menu = true;
                } else {
                    interacting_npc = nullptr;
                }
            }
        }
    }
}

void TownState::render() {
    engine->get_layout().draw();

    Player* p = engine->get_player_manager().get_player();
    if (!p) return;

    std::vector<std::string> equip_info;
    const auto& equips = p->get_all_equipped();
    auto format_equip = [&](const std::string& slot_name, const std::string& display_name) {
        auto it = equips.find(slot_name);
        if (it != equips.end() && it->second) equip_info.push_back(display_name + ": " + it->second->name);
        else equip_info.push_back(display_name + ": (Kosong)");
    };
    format_equip("weapon", "Sjt"); format_equip("armor", "Zir"); format_equip("boots", "Sep"); format_equip("ring", "Cin");

    engine->get_layout().draw_player_stats(
        engine->get_layout().win_stat,
        p->get_str(), p->get_cons(), p->get_agi(), p->get_intl(), p->get_wis(),
        element_to_string(p->get_affinity()), p->get_gold(), equip_info
    );

    engine->get_layout().draw_vitals(engine->get_layout().win_hp, p->get_hp(), p->get_max_hp(), p->get_mp(), p->get_max_mp());

    int current_month = engine->get_calendar().getMonth();
    int current_day = engine->get_calendar().getDay();
    std::string current_time = engine->get_calendar().getTimeString();
    int days_left = std::max(0, 15 - current_day);

    std::string loc_name = "Tidak Diketahui";
    Place* cur_place = engine->get_places().get_current_place();
    if (cur_place) loc_name = cur_place->get_name();

    engine->get_layout().draw_calendar(engine->get_layout().win_cal, days_left, current_month, current_day, current_time, loc_name);

    // 7. CATEGORIZED MENU RENDER
    std::vector<std::string> menu_display;

    if (is_in_quest_menu && interacting_npc) {
        Player* p_ptr = engine->get_player_manager().get_player();
        for (size_t i = 0; i < available_quests.size(); ++i) {
            Quest* q = available_quests[i];
            std::string prefix = (quest_selection_index == (int)i) ? "> " : "  ";
            if (q->get_state() == QuestState::AVAILABLE) menu_display.push_back(prefix + "Baru: " + q->get_name());
            else if (q->can_complete(p_ptr)) menu_display.push_back(prefix + "Selesai: " + q->get_name());
            else menu_display.push_back(prefix + "Berlangsung: " + q->get_name());
        }
        std::string exit_prefix = (quest_selection_index == (int)available_quests.size()) ? "> " : "  ";
        menu_display.push_back(exit_prefix + "[Keluar]");
        engine->get_layout().draw_title(engine->get_layout().win_menu, ("Interaksi dengan " + interacting_npc->get_name()).c_str(), engine->get_layout().w_col2, 4);
    } else {
        current_activities.clear(); current_npcs.clear(); current_exits.clear();
        Place* cur = engine->get_places().get_current_place();
        if (cur) {
            int day = engine->get_calendar().getDay();
            std::string phase = engine->get_calendar().getTimeString();

            if (!cur->get_npcs().empty()) {
                menu_display.push_back("--- Orang ---");
                for (auto* npc : cur->get_npcs()) {
                    current_npcs.push_back(npc);
                    std::string prefix = (selection_index == (int)menu_display.size() - 1) ? "> " : "  ";
                    std::string name = npc->known() ? npc->get_name() : "??? (" + npc->get_role() + ")";
                    menu_display.push_back(prefix + "[Bicara] " + name);
                }
            }

            std::vector<Activity> valid_acts;
            for (const auto& act : cur->get_activities()) {
                // A. Check Visibility Condition
                if (!act.visible_condition.evaluate(p, &engine->get_quests())) continue;

                bool day_ok = act.days.empty(); for (int d : act.days) if (d == day) day_ok = true;
                bool phase_ok = act.phases.empty(); for (const auto& ph : act.phases) if (ph == phase) phase_ok = true;
                if (day_ok && phase_ok) valid_acts.push_back(act);
            }

            if (!valid_acts.empty()) {
                menu_display.push_back("--- Aktivitas ---");
                for (const auto& act : valid_acts) {
                    current_activities.push_back(act);
                    std::string prefix = (selection_index == (int)menu_display.size() - 1) ? "> " : "  ";
                    bool actually_locked = act.is_locked || (day == 1);
                    menu_display.push_back(prefix + (actually_locked ? "[TERKUNCI] " : "") + act.name);
                }
            }

            if (!cur->get_walkable_places().empty()) {
                menu_display.push_back("--- Pergerakan ---");
                for (auto* exit : cur->get_walkable_places()) {
                    current_exits.push_back(exit);
                    std::string prefix = (selection_index == (int)menu_display.size() - 1) ? "> " : "  ";
                    menu_display.push_back(prefix + "[Pergi] " + exit->get_name());
                }
            }
        }
    }

    if (engine->get_dialogs().has_queued_dialog()) {
        menu_display.clear();
        menu_display.push_back("...");
    }

    engine->get_layout().draw_tasks(engine->get_layout().win_menu, menu_display);

    std::vector<std::string> info_display;
    info_display.push_back("--- Misi ---");
    for (auto& pair : engine->get_quests().get_all_quests()) {
        Quest* q = pair.second;
        if (q && (q->get_state() == QuestState::AVAILABLE || q->get_state() == QuestState::IN_PROGRESS)) info_display.push_back(" Q: " + q->get_name());
    }

    info_display.push_back(""); info_display.push_back("--- Inventaris ---");
    for (const auto& pair : p->get_inventory()) {
        const std::string& item_id = pair.first; int amount = pair.second;
        const Item* item_data = engine->get_db().get_item(item_id);
        if (item_data) info_display.push_back(" - " + item_data->name + " x" + std::to_string(amount));
        else info_display.push_back(" - " + item_id + " x" + std::to_string(amount));
    }
    
    engine->get_layout().draw_tasks(engine->get_layout().win_task, info_display);
    auto dialog_log = engine->get_dialogs().get_combined_log();
    engine->get_layout().render_history(engine->get_layout().win_dialog, dialog_log);
}