#include "TownState.hpp"
#include "DungeonState.hpp"
#include "../utils/components/Popup.hpp"
#include "../utils/components/ChoicePopup.hpp"
#include "../utils/components/LogPopup.hpp"
#include "BattleState.hpp"
#include "../GameEngine.hpp" 
#include "../utils/Logger.hpp"
#include <ncurses.h>
#include <queue>
#include <map>
#include <algorithm>
#include "StatAllocationState.hpp"

TownState::TownState(GameEngine* eng) : GameState(eng) {}

void TownState::on_enter() {
    Utils::Logger::log("TownState: Entering state.");
    selection_index = 0;
    
    // Test log integration
    engine->get_log_manager().add_log(engine->get_calendar().getTimeString(), "Entered town state.");
    
    engine->get_layout().resize(); // Ensure UI is drawn properly when entering TownState

    Place* cur = engine->get_places().get_current_place();
    if (!cur) return;

    engine->get_places().update_npc_locations(engine->get_calendar(), engine->get_quests());
    
    // Populate map
    map_places.clear();
    for (const Place* p : engine->get_db().get_all_places()) {
        map_places.push_back(const_cast<Place*>(p));
    }

    // Do NOT interrupt ongoing cutscenes with room entry triggers!
    bool dialog_active = engine->get_dialogs().has_queued_dialog() || !engine->get_dialogs().get_next_scene().empty();

    // Day 2 Trigger: Special story for morning in attic
    int day = engine->get_calendar().getDay();
    std::string phase = engine->get_calendar().getTimeString();
    int has_seen_day2 = engine->get_player_manager().get_player()->get_var("seen_day2_cutscene");
    if (day == 2 && phase == "Pagi" && cur->get_id() == "kamar_loteng" && !dialog_active && has_seen_day2 == 0) {
        const DialogScene* day2_start = engine->get_db().get_dialog_scene("start_day_2");
        if (day2_start) {
            engine->get_player_manager().get_player()->set_var("seen_day2_cutscene", 1);
            engine->get_log_manager().add_log(engine->get_calendar().getTimeString(), "Story event: Day 2 Morning.");
            engine->get_dialogs().start_scene(*day2_start, engine);
            this->render();
            return; 
        }
    }

    std::string scene_id = cur->get_on_enter();
    if (!cur->get_has_entered()) {
        engine->get_log_manager().add_log(engine->get_calendar().getTimeString(), "Discovered new location: " + cur->get_name());
        if (!cur->get_on_first_enter().empty()) {
            scene_id = cur->get_on_first_enter();
        }
        cur->set_has_entered(true);
    }

    if (!scene_id.empty() && !dialog_active) {
        const DialogScene* scene = engine->get_db().get_dialog_scene(scene_id);
        if (scene) {
            engine->get_dialogs().start_scene(*scene, engine);
        }
    }
    init_tabs();
    this->render();
    fast_travel_queue.clear();
    is_fast_traveling = false;
    is_confirming_fast_travel = false;
    fast_travel_target = nullptr;
    fast_travel_path_preview.clear();
}

// --- CORE LOOPS ---

void TownState::handle_input(int ch) {
    if (ch == KEY_RESIZE) { 
        engine->get_layout().resize(); 
        if (current_choice_popup) current_choice_popup->resize();
        return; 
    }

    if (is_fast_traveling) return; // Block input during travel
    if (is_confirming_fast_travel) {
        if (ch == 'y' || ch == 'Y') {
            is_confirming_fast_travel = false;
            is_fast_traveling = true;
            fast_travel_queue.clear();
            for (size_t i = 1; i < fast_travel_path_preview.size(); ++i) {
                fast_travel_queue.enqueue(fast_travel_path_preview[i]);
            }
            cycle_tab();
        } else if (ch == 'n' || ch == 'N' || ch == 27) { // 27 = ESC
            is_confirming_fast_travel = false;
        }
        return;
    }

    if (engine->get_dialogs().has_queued_dialog()) return;

    if (ch == 'q') { engine->quit(); return; } 
    else if (ch == 'c' || ch == 'C') {
        engine->push_state(new StatAllocationState(engine));
        return;
    } else if (ch == 'l' || ch == 'L') {
        engine->show_popup(std::make_unique<Utils::LogPopup>(engine->get_log_manager()));
        return;
    }

    else if (ch == '\t') { 
        cycle_tab(); 
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
    else if (current_tab == MenuTab::MAP) handle_map_menu_input(ch);
    else handle_world_menu_input(ch);
}

void TownState::update() {
    if (engine->get_dialogs().has_queued_dialog()) {
        process_dialogue_queue();
        if (!engine->get_dialogs().has_queued_dialog()) {
            handle_post_dialogue();
        }
    }
    
    if (is_fast_traveling) {
        execute_fast_travel_step();
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

    engine->get_layout().draw_map(engine->get_layout().win_thought, graph_nodes, edges, selected_id, current_tab == MenuTab::MAP, root_id);

    std::vector<std::string> menu_display;
    if (is_in_quest_menu && interacting_npc) {
        render_quest_menu(p, menu_display);
    } else if (current_tab == MenuTab::MAP) {
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
            current_choice_popup = std::make_unique<Utils::ChoicePopup>(latest_dialog, engine->get_dialogs().get_active_choices(), engine->get_dialogs().get_selected_choice_index());
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

void TownState::init_tabs() {
    current_tab = MenuTab::MAP;
    cycle_tab(); // Will naturally find the first valid tab
}

void TownState::cycle_tab() {
    auto is_tab_valid = [&](MenuTab tab) {
        if (tab == MenuTab::MAP) return !map_places.empty();
        Place* cur = engine->get_places().get_current_place();
        if (!cur) return false;
        if (tab == MenuTab::NPC) return !cur->get_npcs().empty();
        if (tab == MenuTab::EXIT) return !cur->get_walkable_places().empty();
        if (tab == MenuTab::ACTIVITY) {
            Player* p = engine->get_player_manager().get_player();
            int day = engine->get_calendar().getDay(); 
            std::string phase = engine->get_calendar().getTimeString();
            for (const auto& act : cur->get_activities()) {
                if (!act.visible_condition.evaluate(p, &engine->get_quests())) continue;
                bool day_ok = act.days.empty() || std::find(act.days.begin(), act.days.end(), day) != act.days.end();
                bool phase_ok = act.phases.empty() || std::find(act.phases.begin(), act.phases.end(), phase) != act.phases.end();
                if (day_ok && phase_ok) return true;
            }
            return false;
        }
        return false;
    };

    for (int i = 0; i < 4; ++i) {
        int next = (int)current_tab + 1;
        if (next > 3) next = 0;
        current_tab = (MenuTab)next;
        if (is_tab_valid(current_tab)) break;
    }
    selection_index = 0;
}

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
                    engine->get_dialogs().start_scene(*scene, engine);
                }
            } else {
                engine->get_dialogs().queue_popup(q->get_state() == QuestState::IN_PROGRESS ? "Kamu masih mengerjakan ini: " + q->get_description() : "Tahap misi ini belum memiliki cerita.");
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
            cycle_tab();
            return;
        }
        
        bool is_adjacent = false;
        for (const auto& [dir, exit] : cur->get_walkable_places()) {
            if (exit->get_id() == target->get_id()) {
                is_adjacent = true; break;
            }
        }
        
        if (is_adjacent) {
            cycle_tab();
            execute_movement(target);
        } else {
            fast_travel_path_preview = find_shortest_path(cur->get_id(), target->get_id());
            if (!fast_travel_path_preview.empty()) {
                fast_travel_target = target;
                is_confirming_fast_travel = true;
            } else {
                engine->get_dialogs().queue_popup("Jalur ke sana tidak ditemukan!");
            }
        }
    }
}

void TownState::handle_world_menu_input(int ch) {
    int total_options = 0;
    if (current_tab == MenuTab::NPC) total_options = current_npcs.size();
    else if (current_tab == MenuTab::ACTIVITY) total_options = current_activities.size();
    else if (current_tab == MenuTab::EXIT) total_options = current_exits.size();
    
    if (total_options == 0) return;
    
    if (ch == KEY_UP || ch == 'w' || ch == KEY_LEFT || ch == 'a') {
        selection_index--;
        if (selection_index < 0) selection_index = total_options - 1;
    } else if (ch == KEY_DOWN || ch == 's' || ch == KEY_RIGHT || ch == 'd') {
        selection_index++;
        if (selection_index >= total_options) selection_index = 0;
    } else if (ch == '\n' || ch == ' ') {
        if (selection_index < 0) return;
        if (current_tab == MenuTab::NPC && selection_index < (int)current_npcs.size()) {
            execute_npc_interaction(current_npcs[selection_index]);
        } else if (current_tab == MenuTab::ACTIVITY && selection_index < (int)current_activities.size()) {
            execute_activity(current_activities[selection_index]);
        } else if (current_tab == MenuTab::EXIT && selection_index < (int)current_exits.size()) {
            execute_movement(current_exits[selection_index]);
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
            engine->get_dialogs().start_scene(*scene, engine);
        }
    } else {
        if (available_quests.empty()) {
            engine->get_dialogs().queue_popup("Karakter ini tidak memiliki apa pun untuk dikatakan."); interacting_npc = nullptr;
        } else is_in_quest_menu = true;
    }
}

void TownState::execute_activity(const Activity& act) {
    if (act.id == "masuk_dungeon") { engine->push_state(new DungeonState(engine)); return; }

    bool actually_locked = act.is_locked || (engine->get_calendar().getDay() == 1 && act.id != "tidur");
    if (actually_locked) {
        engine->get_dialogs().queue_popup(act.dialog_locked.empty() ? "Aktivitas ini sedang terkunci." : act.dialog_locked); return;
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
    if (!can_do) { engine->get_dialogs().queue_popup(fail_msg); return; }

    if (!act.dialog_success.empty()) { engine->get_dialogs().queue_popup(act.dialog_success); }
    for (const auto& action : act.on_execute) engine->get_actions().execute(action);
}

void TownState::execute_movement(Place* target) {
    if (engine->get_places().travel(target->get_id())) on_enter();
}

std::vector<std::string> TownState::find_shortest_path(const std::string& start, const std::string& target) {
    Utils::Queue<std::string> q;
    std::map<std::string, std::string> parent;
    std::map<std::string, bool> visited;

    q.enqueue(start);
    visited[start] = true;

    bool found = false;
    while (!q.is_empty()) {
        std::string current = q.front();
        q.dequeue();

        if (current == target) {
            found = true;
            break;
        }

        Place* p = nullptr;
        for (Place* mp : map_places) { if (mp->get_id() == current) p = mp; }
        
        if (p) {
            for (const auto& [dir, exit] : p->get_walkable_places()) {
                std::string neighbor = exit->get_id();
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    parent[neighbor] = current;
                    q.enqueue(neighbor);
                }
            }
        }
    }

    std::vector<std::string> path;
    if (!found) return path;

    std::string curr = target;
    while (curr != start) {
        path.push_back(curr);
        curr = parent[curr];
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

void TownState::execute_fast_travel_step() {
    if (engine->get_dialogs().has_queued_dialog() || engine->get_dialogs().has_queued_popup() || engine->get_dialogs().has_active_choices()) return;

    if (fast_travel_queue.is_empty()) {
        is_fast_traveling = false;
        engine->get_dialogs().queue_popup("Kamu telah tiba di " + fast_travel_target->get_name() + ".");
        return;
    }

    std::string next_loc = fast_travel_queue.front();
    fast_travel_queue.dequeue();
    
    engine->get_places().set_current_place(next_loc);
    on_enter(); // refresh NPCs etc.

    // Random Encounter Chance
    int roll = rand() % 100;
    if (roll < 30) { // 30% chance for encounter
        if (rand() % 2 == 0) {
            engine->get_dialogs().queue_popup("Ada monster menyerangmu saat di perjalanan!");
            std::string group = (rand() % 2 == 0) ? "mg_slime" : "mg_goblin";
            engine->push_state(new BattleState(engine, group));
        } else {
            engine->get_dialogs().queue_popup("Sebuah kejadian acak terjadi di perjalanan...");
            std::vector<std::string> events = {
                "fast_travel_event_1", 
                "fast_travel_event_pickpocket", 
                "fast_travel_event_gossip"
            };
            std::string selected_event = events[rand() % events.size()];
            if (const DialogScene* scene = engine->get_db().get_dialog_scene(selected_event)) {
                engine->get_dialogs().start_scene(*scene, engine);
            }
        }
    }
}

// --- UPDATE HELPERS ---

void TownState::process_dialogue_queue() {
    DialogNode node = engine->get_dialogs().pop_dialog();
    if (node.type == 1 || node.type == 2) {
        engine->get_layout().type_new_text(engine->get_layout().win_dialog, "Dialog", engine->get_layout().w_left, engine->get_dialogs().get_combined_log(), node);
        if (node.type == 1) engine->get_dialogs().add_dialog(node);
        else engine->get_dialogs().add_thought(node);
    } else if (node.type == 3) {
        engine->get_dialogs().queue_popup(node.value); engine->get_dialogs().add_popup(node);
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
                engine->get_dialogs().start_scene(*next_scene, engine);
            }
        } else {
            engine->get_dialogs().set_on_exit({}); engine->get_dialogs().set_next_scene("");
            this->on_enter();
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

    engine->get_layout().draw_player_stats(engine->get_layout().win_stat, p->get_str(), p->get_cons(), p->get_agi(), p->get_intl(), p->get_wis(), p->get_stat_points(), element_to_string(p->get_affinity()), p->get_gold(), equip_info);
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

    auto is_tab_valid = [&](MenuTab tab) {
        if (tab == MenuTab::MAP) return !map_places.empty();
        if (tab == MenuTab::NPC) return !cur->get_npcs().empty();
        if (tab == MenuTab::EXIT) return !cur->get_walkable_places().empty();
        if (tab == MenuTab::ACTIVITY) {
            for (const auto& act : cur->get_activities()) {
                if (!act.visible_condition.evaluate(p, &engine->get_quests())) continue;
                bool day_ok = act.days.empty() || std::find(act.days.begin(), act.days.end(), day) != act.days.end();
                bool phase_ok = act.phases.empty() || std::find(act.phases.begin(), act.phases.end(), phase) != act.phases.end();
                if (day_ok && phase_ok) return true;
            }
            return false;
        }
        return false;
    };

    std::string tab_header = "";
    if (is_tab_valid(MenuTab::NPC)) tab_header += (current_tab == MenuTab::NPC ? "[Orang] " : " Orang  ");
    if (is_tab_valid(MenuTab::ACTIVITY)) tab_header += (current_tab == MenuTab::ACTIVITY ? "[Aktivitas] " : " Aktivitas  ");
    if (is_tab_valid(MenuTab::EXIT)) tab_header += (current_tab == MenuTab::EXIT ? "[Jalan Keluar] " : " Jalan Keluar  ");
    if (is_tab_valid(MenuTab::MAP)) tab_header += (current_tab == MenuTab::MAP ? "[Peta] " : " Peta  ");
    menu_display.push_back("");
    menu_display.push_back(tab_header);
    menu_display.push_back("");

    int item_idx = 0;

    if (current_tab == MenuTab::NPC && !cur->get_npcs().empty()) {
        menu_display.push_back("--- Bicara dengan Orang ---");
        for (auto* npc : cur->get_npcs()) {
            current_npcs.push_back(npc);
            std::string name = npc->known() ? npc->get_name() : "??? (" + npc->get_role() + ")";
            std::string pref = (selection_index == item_idx ? "> " : "  ");
            menu_display.push_back(pref + "[Bicara] " + name);
            item_idx++;
        }
    }

    if (current_tab == MenuTab::ACTIVITY) {
        std::vector<Activity> valid_acts;
        for (const auto& act : cur->get_activities()) {
            if (!act.visible_condition.evaluate(p, &engine->get_quests())) continue;
            bool day_ok = act.days.empty() || std::find(act.days.begin(), act.days.end(), day) != act.days.end();
            bool phase_ok = act.phases.empty() || std::find(act.phases.begin(), act.phases.end(), phase) != act.phases.end();
            if (day_ok && phase_ok) valid_acts.push_back(act);
        }
        if (!valid_acts.empty()) {
            menu_display.push_back("--- Lakukan Aktivitas ---");
            for (const auto& act : valid_acts) {
                current_activities.push_back(act);
                std::string pref = (selection_index == item_idx ? "> " : "  ");
                std::string lock = (act.is_locked || (day == 1 && act.id != "tidur") ? "[TERKUNCI] " : "");
                menu_display.push_back(pref + lock + act.name);
                item_idx++;
            }
        }
    }

    if (current_tab == MenuTab::EXIT && !cur->get_walkable_places().empty()) {
        menu_display.push_back("--- Pergi ke Lokasi ---");
        for (const auto& [dir, exit] : cur->get_walkable_places()) {
            current_exits.push_back(exit);
            std::string pref = (selection_index == item_idx ? "> " : "  ");
            std::string dir_id = dir;
            if (dir == "north") dir_id = "Utara";
            else if (dir == "south") dir_id = "Selatan";
            else if (dir == "east") dir_id = "Timur";
            else if (dir == "west") dir_id = "Barat";
            menu_display.push_back(pref + "[Pergi ke " + dir_id + "] " + exit->get_name());
            item_idx++;
        }
    }
}

void TownState::render_map_preview(Player* p, std::vector<std::string>& menu_display) {
    Place* cur = engine->get_places().get_current_place();
    int day = engine->get_calendar().getDay(); 
    std::string phase = engine->get_calendar().getTimeString();

    auto is_tab_valid = [&](MenuTab tab) {
        if (tab == MenuTab::MAP) return !map_places.empty();
        if (tab == MenuTab::NPC) return cur && !cur->get_npcs().empty();
        if (tab == MenuTab::EXIT) return cur && !cur->get_walkable_places().empty();
        if (tab == MenuTab::ACTIVITY) {
            if (!cur) return false;
            for (const auto& act : cur->get_activities()) {
                if (!act.visible_condition.evaluate(p, &engine->get_quests())) continue;
                bool day_ok = act.days.empty() || std::find(act.days.begin(), act.days.end(), day) != act.days.end();
                bool phase_ok = act.phases.empty() || std::find(act.phases.begin(), act.phases.end(), phase) != act.phases.end();
                if (day_ok && phase_ok) return true;
            }
            return false;
        }
        return false;
    };

    std::string tab_header = "";
    if (is_tab_valid(MenuTab::NPC)) tab_header += (current_tab == MenuTab::NPC ? "[Orang] " : " Orang  ");
    if (is_tab_valid(MenuTab::ACTIVITY)) tab_header += (current_tab == MenuTab::ACTIVITY ? "[Aktivitas] " : " Aktivitas  ");
    if (is_tab_valid(MenuTab::EXIT)) tab_header += (current_tab == MenuTab::EXIT ? "[Jalan Keluar] " : " Jalan Keluar  ");
    if (is_tab_valid(MenuTab::MAP)) tab_header += (current_tab == MenuTab::MAP ? "[Peta] " : " Peta  ");
    menu_display.push_back("");
    menu_display.push_back(tab_header);
    menu_display.push_back("");

    if (map_places.empty() || map_selection_index >= (int)map_places.size()) return;
    
    Place* target = map_places[map_selection_index];

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
            std::string lock = (act.is_locked || (day == 1 && act.id != "tidur") ? "[TERKUNCI] " : "");
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
    // Render Fast Travel Confirmation
    if (is_confirming_fast_travel) {
        int pw = 60;
        int ph = 7;
        int px = (COLS - pw) / 2;
        int py = (LINES - ph) / 2;
        WINDOW* win_confirm = newwin(ph, pw, py, px);
        box(win_confirm, 0, 0);
        wattron(win_confirm, COLOR_PAIR(4));
        mvwprintw(win_confirm, 0, 2, " FAST TRAVEL ");
        wattroff(win_confirm, COLOR_PAIR(4));

        std::string msg = "Perjalanan ke " + fast_travel_target->get_name() + " melewati " + std::to_string(fast_travel_path_preview.size() - 1) + " area.";
        mvwprintw(win_confirm, 2, 2, "%s", msg.c_str());
        mvwprintw(win_confirm, 3, 2, "Ada kemungkinan random encounter di jalan.");
        
        wattron(win_confirm, A_BOLD);
        mvwprintw(win_confirm, 5, 2, "[Y] Lanjutkan     [N] Batal");
        wattroff(win_confirm, A_BOLD);

        wrefresh(win_confirm);
        delwin(win_confirm);
    }
    engine->get_layout().render_history(engine->get_layout().win_dialog, engine->get_dialogs().get_combined_log());
}