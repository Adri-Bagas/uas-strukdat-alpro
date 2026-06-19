#include "TownState.hpp"
#include "DungeonState.hpp"
#include "../utils/components/Popup.hpp"
#include "../utils/components/ChoicePopup.hpp"
#include "../GameEngine.hpp" 
#include "../utils/Logger.hpp"
#include <ncurses.h>
#include <queue>
#include <map>
#include <algorithm>

TownState::TownState(GameEngine* eng) : GameState(eng) {}

void TownState::on_enter() {
    Logger::log("TownState: Entering state.");
    selection_index = 0;
    Place* cur = engine->get_places().get_current_place();
    if (!cur) return;

    engine->get_places().update_npc_locations(engine->get_calendar(), engine->get_quests());
    
    // Populate map
    map_places.clear();
    for (const Place* p : engine->get_db().get_all_places()) {
        map_places.push_back(const_cast<Place*>(p));
    }

    // Day 2 Trigger: Special story for morning in attic
    int day = engine->get_calendar().getDay();
    std::string phase = engine->get_calendar().getTimeString();
    if (day == 2 && phase == "Pagi" && cur->get_id() == "kamar_loteng") {
        const DialogScene* day2_start = engine->get_db().get_dialog_scene("start_day_2");
        if (day2_start) {
            engine->get_dialogs().set_on_exit(day2_start->on_exit);
            engine->get_dialogs().set_next_scene(day2_start->next_scene_id);
            for (auto node : day2_start->nodes) {
                if (!node.npc_name.empty()) {
                    const NPC* npc = engine->get_db().get_npc(node.npc_name);
                    if (npc && !npc->known()) node.npc_name = "???";
                }
                engine->get_dialogs().queue_dialog(node);
            }
            this->render();
            return; 
        }
    }

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
                if (!node.npc_name.empty()) {
                    const NPC* npc = engine->get_db().get_npc(node.npc_name);
                    if (npc && !npc->known()) node.npc_name = "???";
                }
                engine->get_dialogs().queue_dialog(node);
            }
        }
    }
    this->render();
}

// --- CORE LOOPS ---

void TownState::handle_input(int ch) {
    if (engine->get_dialogs().has_queued_dialog()) return;

    if (ch == 'q') { engine->quit(); return; } 
    else if (ch == KEY_RESIZE) { 
        engine->get_layout().resize(); 
        if (current_choice_popup) current_choice_popup.reset();
        return; 
    }
    else if (ch == '\t') { 
        is_in_map_mode = !is_in_map_mode; 
        return; 
    }

    if (engine->get_dialogs().has_active_choices()) {
        int idx = engine->get_dialogs().get_selected_choice_index();
        int count = engine->get_dialogs().get_active_choices().size();
        if (ch == KEY_UP || ch == 'w' || ch == KEY_LEFT || ch == 'a') {
            idx--;
            if (idx < 0) idx = count - 1;
            engine->get_dialogs().set_selected_choice_index(idx);
        } else if (ch == KEY_DOWN || ch == 's' || ch == KEY_RIGHT || ch == 'd') {
            idx++;
            if (idx >= count) idx = 0;
            engine->get_dialogs().set_selected_choice_index(idx);
        } else if (ch == '\n' || ch == ' ') {
            engine->get_dialogs().select_choice(idx, engine);
        }
        return;
    }

    if (is_in_quest_menu) handle_quest_menu_input(ch);
    else if (is_in_map_mode) handle_map_menu_input(ch);
    else handle_world_menu_input(ch);
}

void TownState::update() {
    if (engine->get_dialogs().has_queued_dialog()) {
        process_dialogue_queue();
        if (!engine->get_dialogs().has_queued_dialog()) {
            handle_post_dialogue();
        }
    }
}

void TownState::render() {
    engine->get_layout().draw();
    Player* p = engine->get_player_manager().get_player();
    if (!p) return;

    render_player_status(p);

    // Render the Map
    std::map<std::string, std::pair<int, int>> coords;
    std::vector<GraphEdge> edges;
    std::queue<std::pair<std::string, std::pair<int, int>>> q;
    
    std::string root_id = engine->get_places().get_current_place()->get_id();
    q.push({root_id, {50, 50}});
    coords[root_id] = {50, 50};

    while (!q.empty()) {
        auto [curr_id, pos] = q.front(); q.pop();
        Place* curr_place = nullptr;
        for (Place* p : map_places) { if (p->get_id() == curr_id) { curr_place = p; break; } }
        if (!curr_place) continue;

        for (const auto& [dir, wp] : curr_place->get_walkable_places()) {
            std::string nbr_id = wp->get_id();
            if (curr_id < nbr_id) {
                edges.push_back({curr_id, nbr_id});
            }
            if (coords.find(nbr_id) == coords.end()) {
                int nx = pos.first;
                int ny = pos.second;
                if (dir == "north") ny -= 1;
                else if (dir == "south") ny += 1;
                else if (dir == "east") nx += 1;
                else if (dir == "west") nx -= 1;

                coords[nbr_id] = {nx, ny};
                q.push({nbr_id, {nx, ny}});
            }
        }
    }

    int min_x = 9999, min_y = 9999, max_x = -9999, max_y = -9999;
    for (const auto& [id, pos] : coords) {
        if (pos.first < min_x) min_x = pos.first;
        if (pos.first > max_x) max_x = pos.first;
        if (pos.second < min_y) min_y = pos.second;
        if (pos.second > max_y) max_y = pos.second;
    }

    std::vector<GraphNode> graph_nodes;
    for (const auto& [id, pos] : coords) {
        std::string n_name = "";
        for (Place* p : map_places) {
            if (p->get_id() == id) { n_name = p->get_name(); break; }
        }
        int norm_x = pos.first - min_x;
        int norm_y = pos.second - min_y;
        graph_nodes.push_back({id, n_name, norm_x, norm_y});
    }
    
    int overflow_y = (max_y == -9999) ? 0 : (max_y - min_y + 1);
    for (Place* p : map_places) {
        if (coords.find(p->get_id()) == coords.end()) {
            graph_nodes.push_back({p->get_id(), p->get_name(), 0, overflow_y++});
        }
    }

    std::string selected_id = "";
    if (map_selection_index >= 0 && map_selection_index < (int)map_places.size()) {
        selected_id = map_places[map_selection_index]->get_id();
    }

    engine->get_layout().draw_map(engine->get_layout().win_thought, graph_nodes, edges, selected_id, is_in_map_mode, root_id);

    std::vector<std::string> menu_display;
    if (is_in_quest_menu && interacting_npc) {
        render_quest_menu(p, menu_display);
    } else if (is_in_map_mode) {
        render_map_preview(p, menu_display);
    } else {
        render_world_menu(p, menu_display);
    }

    if (engine->get_dialogs().has_queued_dialog()) {
        menu_display.clear();
        menu_display.push_back("...");
    }
    engine->get_layout().draw_tasks(engine->get_layout().win_menu, menu_display);

    if (engine->get_dialogs().has_active_choices()) {
        if (!current_choice_popup) {
            auto log = engine->get_dialogs().get_combined_log();
            std::string latest_dialog = "(Pilih Salah Satu)";
            for (auto it = log.rbegin(); it != log.rend(); ++it) {
                if (it->value != "--------------------------------") {
                    latest_dialog = it->value;
                    if (!it->npc_name.empty()) {
                        latest_dialog = "[" + it->npc_name + "]: " + latest_dialog;
                    }
                    break;
                }
            }
            current_choice_popup = std::make_unique<ChoicePopup>(latest_dialog, engine->get_dialogs().get_active_choices(), engine->get_dialogs().get_selected_choice_index());
        } else {
            current_choice_popup->set_selected_index(engine->get_dialogs().get_selected_choice_index());
        }
        current_choice_popup->render();
    } else {
        if (current_choice_popup) current_choice_popup.reset();
        render_sidebars(p);
    }
}

// --- INPUT HELPERS ---

void TownState::handle_quest_menu_input(int ch) {
    int total_quest_options = available_quests.size() + 1; 
    if (ch == KEY_UP || ch == 'w' || ch == KEY_LEFT || ch == 'a') {
        quest_selection_index--;
        if (quest_selection_index < 0) quest_selection_index = total_quest_options - 1;
    } else if (ch == KEY_DOWN || ch == 's' || ch == KEY_RIGHT || ch == 'd') {
        quest_selection_index++;
        if (quest_selection_index >= total_quest_options) quest_selection_index = 0;
    } else if (ch == '\n' || ch == ' ') {
        if (quest_selection_index == (int)available_quests.size()) {
            interacting_npc = nullptr; available_quests.clear(); is_in_quest_menu = false; selection_index = 0;
        } else {
            Quest* q = available_quests[quest_selection_index];
            Player* p = engine->get_player_manager().get_player();
            std::string scene_id = q->can_complete(p) ? q->get_complete_scene() : (q->get_state() == QuestState::AVAILABLE ? q->get_start_scene() : "");

            if (!scene_id.empty()) {
                const DialogScene* scene = engine->get_db().get_dialog_scene(scene_id);
                if (scene) {
                    is_in_quest_menu = false; 
                    if (q->can_complete(p)) engine->get_actions().execute("complete_quest " + q->get_id());
                    for (auto node : scene->nodes) {
                        if (!node.npc_name.empty()) {
                            const NPC* n_ptr = engine->get_db().get_npc(node.npc_name);
                            if (n_ptr && !n_ptr->known()) node.npc_name = "???";
                        }
                        engine->get_dialogs().queue_dialog(node);
                    }
                }
            } else {
                Popup p_box {q->get_state() == QuestState::IN_PROGRESS ? "Kamu masih mengerjakan ini: " + q->get_description() : "Tahap misi ini belum memiliki cerita."};
                p_box.animate(); p_box.type_text();
            }
        }
    }
}

void TownState::handle_map_menu_input(int ch) {
    if (map_places.empty()) return;
    
    if (ch == KEY_UP || ch == 'w' || ch == KEY_LEFT || ch == 'a') {
        map_selection_index--;
        if (map_selection_index < 0) map_selection_index = (int)map_places.size() - 1;
    } else if (ch == KEY_DOWN || ch == 's' || ch == KEY_RIGHT || ch == 'd') {
        map_selection_index++;
        if (map_selection_index >= (int)map_places.size()) map_selection_index = 0;
    } else if (ch == '\n' || ch == ' ') {
        Place* cur = engine->get_places().get_current_place();
        Place* target = map_places[map_selection_index];
        
        if (cur->get_id() == target->get_id()) {
            is_in_map_mode = false;
            return;
        }
        
        bool is_adjacent = false;
        for (const auto& [dir, exit] : cur->get_walkable_places()) {
            if (exit->get_id() == target->get_id()) {
                is_adjacent = true; break;
            }
        }
        
        if (is_adjacent) {
            is_in_map_mode = false;
            execute_movement(target);
        } else {
            Popup pop {"Kamu tidak bisa langsung pergi ke sana dari sini."};
            pop.animate();
            pop.type_text();
        }
    }
}

void TownState::handle_world_menu_input(int ch) {
    int total_options = current_npcs.size() + current_activities.size() + current_exits.size();
    if (total_options == 0) return;
    
    if (ch == KEY_UP || ch == 'w' || ch == KEY_LEFT || ch == 'a') {
        selection_index--;
        if (selection_index < 0) selection_index = total_options - 1;
    } else if (ch == KEY_DOWN || ch == 's' || ch == KEY_RIGHT || ch == 'd') {
        selection_index++;
        if (selection_index >= total_options) selection_index = 0;
    } else if (ch == '\n' || ch == ' ') {
        if (selection_index < 0) return;
        if (selection_index < (int)current_npcs.size()) {
            execute_npc_interaction(current_npcs[selection_index]);
        } else if (selection_index < (int)(current_npcs.size() + current_activities.size())) {
            execute_activity(current_activities[selection_index - current_npcs.size()]);
        } else {
            execute_movement(current_exits[selection_index - (current_npcs.size() + current_activities.size())]);
        }
    }
}

void TownState::execute_npc_interaction(NPC* npc) {
    interacting_npc = npc;
    Player* p = engine->get_player_manager().get_player();
    engine->get_quests().check_npc_quests(interacting_npc, p);
    available_quests = engine->get_quests().get_available_quests_for_npc(interacting_npc->get_id());
    is_in_quest_menu = false; quest_selection_index = 0;

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
            Popup pop {"Karakter ini tidak memiliki apa pun untuk dikatakan."}; pop.animate(); pop.type_text(); interacting_npc = nullptr;
        } else is_in_quest_menu = true;
    }
}

void TownState::execute_activity(const Activity& act) {
    if (act.id == "masuk_dungeon") { engine->push_state(new DungeonState(engine)); return; }

    bool actually_locked = act.is_locked || (engine->get_calendar().getDay() == 1);
    if (actually_locked) {
        Popup p_box {act.dialog_locked.empty() ? "Aktivitas ini sedang terkunci." : act.dialog_locked};
        p_box.animate(); p_box.type_text(); return;
    }

    Player* p = engine->get_player_manager().get_player();
    bool can_do = true; std::string fail_msg = act.dialog_fail;
    for (const auto& [stat, req_val] : act.req_stats) {
        int v = (stat=="str") ? p->get_str() : (stat=="cons") ? p->get_cons() : (stat=="agi") ? p->get_agi() : (stat=="intl") ? p->get_intl() : (stat=="wis") ? p->get_wis() : 0;
        if (v < req_val) {
            can_do = false; if (fail_msg.empty()) fail_msg = "Status tidak mencukupi! Butuh " + stat + " " + std::to_string(req_val);
            break;
        }
    }
    if (!can_do) { Popup pop {fail_msg}; pop.animate(); pop.type_text(); return; }

    if (!act.dialog_success.empty()) { Popup pop {act.dialog_success}; pop.animate(); pop.type_text(); }
    for (const auto& action : act.on_execute) engine->get_actions().execute(action);
}

void TownState::execute_movement(Place* target) {
    if (engine->get_places().travel(target->get_id())) on_enter();
}

// --- UPDATE HELPERS ---

void TownState::process_dialogue_queue() {
    DialogNode node = engine->get_dialogs().pop_dialog();
    if (node.type == 1 || node.type == 2) {
        engine->get_layout().type_new_text(engine->get_layout().win_dialog, "Dialog", engine->get_layout().w_left, engine->get_dialogs().get_combined_log(), node);
        if (node.type == 1) engine->get_dialogs().add_dialog(node);
        else engine->get_dialogs().add_thought(node);
    } else if (node.type == 3) {
        Popup otp {node.value}; otp.animate(); otp.type_text(); engine->get_dialogs().add_popup(node);
    }
    this->render();
}

void TownState::handle_post_dialogue() {
    DialogNode separator {"--------------------------------", "", 0};
    engine->get_dialogs().add_dialog(separator);

    for (const auto& action : engine->get_dialogs().get_on_exit()) engine->get_actions().execute(action);

    if (engine->get_dialogs().has_pending_choices()) {
        engine->get_dialogs().activate_choices(engine);
    } else {
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
            engine->get_dialogs().set_on_exit({}); engine->get_dialogs().set_next_scene("");
        }
    }
    if (interacting_npc) {
        if (!available_quests.empty()) is_in_quest_menu = true;
        else interacting_npc = nullptr;
    }
}

// --- RENDER HELPERS ---

void TownState::render_player_status(Player* p) {
    std::vector<std::string> equip_info;
    const auto& equips = p->get_all_equipped();
    auto format_equip = [&](const std::string& slot, const std::string& label) {
        auto it = equips.find(slot);
        equip_info.push_back(label + ": " + (it != equips.end() && it->second ? it->second->name : "(Kosong)"));
    };
    format_equip("weapon", "Sjt"); format_equip("armor", "Zir"); format_equip("boots", "Sep"); format_equip("ring", "Cin");

    engine->get_layout().draw_player_stats(engine->get_layout().win_stat, p->get_str(), p->get_cons(), p->get_agi(), p->get_intl(), p->get_wis(), element_to_string(p->get_affinity()), p->get_gold(), equip_info);
    engine->get_layout().draw_vitals(engine->get_layout().win_hp, p->get_hp(), p->get_max_hp(), p->get_mp(), p->get_max_mp());

    int day = engine->get_calendar().getDay();
    engine->get_layout().draw_calendar(engine->get_layout().win_cal, std::max(0, 15 - day), engine->get_calendar().getMonth(), day, engine->get_calendar().getTimeString(), engine->get_places().get_current_place() ? engine->get_places().get_current_place()->get_name() : "Tidak Diketahui");
}

void TownState::render_quest_menu(Player* p, std::vector<std::string>& menu_display) {
    for (size_t i = 0; i < available_quests.size(); ++i) {
        Quest* q = available_quests[i];
        std::string prefix = (quest_selection_index == (int)i) ? "> " : "  ";
        if (q->get_state() == QuestState::AVAILABLE) menu_display.push_back(prefix + "Baru: " + q->get_name());
        else if (q->can_complete(p)) menu_display.push_back(prefix + "Selesai: " + q->get_name());
        else menu_display.push_back(prefix + "Berlangsung: " + q->get_name());
    }
    std::string ex_p = (quest_selection_index == (int)available_quests.size() ? "> " : "  ");
    menu_display.push_back(ex_p + "[Keluar]");
    engine->get_layout().draw_title(engine->get_layout().win_menu, ("Interaksi dengan " + interacting_npc->get_name()).c_str(), engine->get_layout().w_col2, 4);
}

void TownState::render_world_menu(Player* p, std::vector<std::string>& menu_display) {
    current_activities.clear(); current_npcs.clear(); current_exits.clear();
    Place* cur = engine->get_places().get_current_place();
    if (!cur) return;
    int day = engine->get_calendar().getDay(); std::string phase = engine->get_calendar().getTimeString();

    if (!cur->get_npcs().empty()) {
        menu_display.push_back("--- Orang ---");
        for (auto* npc : cur->get_npcs()) {
            current_npcs.push_back(npc);
            std::string name = npc->known() ? npc->get_name() : "??? (" + npc->get_role() + ")";
            std::string pref = (selection_index == (int)menu_display.size() - 1 ? "> " : "  ");
            menu_display.push_back(pref + "[Bicara] " + name);
        }
    }

    std::vector<Activity> valid_acts;
    for (const auto& act : cur->get_activities()) {
        if (!act.visible_condition.evaluate(p, &engine->get_quests())) continue;
        bool day_ok = act.days.empty() || std::find(act.days.begin(), act.days.end(), day) != act.days.end();
        bool phase_ok = act.phases.empty() || std::find(act.phases.begin(), act.phases.end(), phase) != act.phases.end();
        if (day_ok && phase_ok) valid_acts.push_back(act);
    }
    if (!valid_acts.empty()) {
        menu_display.push_back("--- Aktivitas ---");
        for (const auto& act : valid_acts) {
            current_activities.push_back(act);
            std::string pref = (selection_index == (int)menu_display.size() - 1 ? "> " : "  ");
            std::string lock = (act.is_locked || day == 1 ? "[TERKUNCI] " : "");
            menu_display.push_back(pref + lock + act.name);
        }
    }

    if (!cur->get_walkable_places().empty()) {
        menu_display.push_back("--- Jalan Keluar ---");
        for (const auto& [dir, exit] : cur->get_walkable_places()) {
            current_exits.push_back(exit);
            std::string pref = (selection_index == (int)menu_display.size() - 1 ? "> " : "  ");
            std::string dir_id = dir;
            if (dir == "north") dir_id = "Utara";
            else if (dir == "south") dir_id = "Selatan";
            else if (dir == "east") dir_id = "Timur";
            else if (dir == "west") dir_id = "Barat";
            menu_display.push_back(pref + "[Pergi ke " + dir_id + "] " + exit->get_name());
        }
    }
}

void TownState::render_map_preview(Player* p, std::vector<std::string>& menu_display) {
    if (map_places.empty() || map_selection_index >= (int)map_places.size()) return;
    
    Place* target = map_places[map_selection_index];
    Place* cur = engine->get_places().get_current_place();
    int day = engine->get_calendar().getDay(); 
    std::string phase = engine->get_calendar().getTimeString();

    menu_display.push_back("--- Pratinjau: " + target->get_name() + " ---");
    
    bool can_walk = false;
    for (const auto& [dir, exit] : cur->get_walkable_places()) {
        if (exit->get_id() == target->get_id()) can_walk = true;
    }
    
    if (cur->get_id() == target->get_id()) {
        menu_display.push_back("[Kamu berada di sini]");
    } else if (can_walk) {
        menu_display.push_back("[Bisa diakses dari sini]");
    } else {
        menu_display.push_back("[Tidak bisa diakses langsung]");
    }
    
    menu_display.push_back("");

    if (!target->get_npcs().empty()) {
        menu_display.push_back("- Orang di sana -");
        for (auto* npc : target->get_npcs()) {
            std::string name = npc->known() ? npc->get_name() : "??? (" + npc->get_role() + ")";
            menu_display.push_back("  " + name);
        }
    }

    std::vector<Activity> valid_acts;
    for (const auto& act : target->get_activities()) {
        if (!act.visible_condition.evaluate(p, &engine->get_quests())) continue;
        bool day_ok = act.days.empty() || std::find(act.days.begin(), act.days.end(), day) != act.days.end();
        bool phase_ok = act.phases.empty() || std::find(act.phases.begin(), act.phases.end(), phase) != act.phases.end();
        if (day_ok && phase_ok) valid_acts.push_back(act);
    }
    
    if (!valid_acts.empty()) {
        menu_display.push_back("- Aktivitas -");
        for (const auto& act : valid_acts) {
            std::string lock = (act.is_locked || day == 1 ? "[TERKUNCI] " : "");
            menu_display.push_back("  " + lock + act.name);
        }
    }
    
    engine->get_layout().draw_title(engine->get_layout().win_menu, "Informasi Lokasi", engine->get_layout().w_col2, 4);
}

void TownState::render_sidebars(Player* p) {
    std::vector<std::string> info; info.push_back("--- Misi ---");
    for (auto& pair : engine->get_quests().get_all_quests()) {
        if (pair.second && (pair.second->get_state() == QuestState::AVAILABLE || pair.second->get_state() == QuestState::IN_PROGRESS)) info.push_back(" Q: " + pair.second->get_name());
    }
    info.push_back(""); info.push_back("--- Inventaris ---");
    for (const auto& pair : p->get_inventory()) {
        const Item* item = engine->get_db().get_item(pair.first);
        info.push_back(" - " + (item ? item->name : pair.first) + " x" + std::to_string(pair.second));
    }

    info.push_back(""); info.push_back("--- Pintasan ---");
    info.push_back(" [TAB] Buka Peta");

    engine->get_layout().draw_tasks(engine->get_layout().win_task, info);
    engine->get_layout().render_history(engine->get_layout().win_dialog, engine->get_dialogs().get_combined_log());
}