#include "DungeonState.hpp"
#include "../GameEngine.hpp"
#include "../utils/Logger.hpp"
#include "../enums/Element.hpp"
#include "StatAllocationState.hpp"
#include "BattleState.hpp"
#include "InventoryState.hpp"
#include <ncurses.h>
#include <random>
#include <algorithm>
#include <string>
#include <functional>

DungeonState::DungeonState(GameEngine* eng) 
    : GameState(eng), has_won(false), active_tab(0) {
}

static int get_random_odd(int min_val, int max_val) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min_val, max_val);
    int val = dis(gen);
    if (val % 2 == 0) {
        val = (val == max_val) ? val - 1 : val + 1;
    }
    return val;
}

void DungeonState::on_enter() {

    // Putar musik dungeon
    engine->get_music_manager().playMusic("dungeon.mp3");

    Utils::Logger::log("DungeonState: Generating multi-floor dungeon with random odd dimensions between 20-100...");

    dungeon.clear();
    has_won = false;
    active_tab = 0; // Default to Party tab
    // Generate 3 floors for our dungeon
    for (int f = 1; f <= 3; ++f) {
        DungeonFloor floor;
        floor.floor_number = f;
        floor.height = get_random_odd(21, 99); // Random odd between 20-100
        floor.width = get_random_odd(21, 99);  // Random odd between 20-100
        
        floor.start_r = 1;
        floor.start_c = 1;
        floor.player_r = 1;
        floor.player_c = 1;
        floor.exit_r = floor.height - 2;
        floor.exit_c = floor.width - 2;
        
        // Initialize visited matrix
        floor.visited.assign(floor.height, std::vector<bool>(floor.width, false));
        
        floor.grid = generate_maze_grid(floor.height, floor.width, floor.exit_r, floor.exit_c);
        dungeon.add_floor(floor);
    }

    // Mark initial visited cells for Floor 1
    DungeonFloorNode* current_node = dungeon.get_current_node();
    if (current_node) {
        update_visited(current_node->floor);
        Player* p = engine->get_player_manager().get_player();
        if (p) {
            p->set_var("dungeon_floor", current_node->floor.floor_number);
        }
    }

    // Quest marker logic: spawn ? markers for IN_PROGRESS dungeon quests
    quest_markers.clear();
    for (const auto& pair : engine->get_quests().get_all_quests()) {
        Quest* q = pair.second;
        if (q->get_state() == QuestState::IN_PROGRESS) {
            const std::string& loc_id = q->get_target_location_id();
            if (loc_id.rfind("dungeon_f", 0) == 0 && !q->get_location_trigger_scene().empty()) {
                int floor_num = std::stoi(loc_id.substr(9));
                DungeonFloorNode* temp = current_node;
                while (temp && temp->floor.floor_number != floor_num) temp = temp->next;
                if (temp) {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dis_r(1, temp->floor.height - 2);
                    std::uniform_int_distribution<> dis_c(1, temp->floor.width - 2);
                    int mx, my;
                    do {
                        my = dis_r(gen);
                        mx = dis_c(gen);
                    } while (temp->floor.grid[my][mx] != 0 ||
                             (my == temp->floor.start_r && mx == temp->floor.start_c));
                    quest_markers.push_back({q->get_id(), q->get_location_trigger_scene(), floor_num, mx, my});
                }
            }
        }
    }
}

void DungeonState::on_resume() {
    // Putar kembali musik dungeon jika kembali dari BattleState
    engine->get_music_manager().playMusic("dungeon.mp3");
}

std::vector<std::vector<int>> DungeonState::generate_maze_grid(int h, int w, int exit_r, int exit_c) {
    // Initialize the target grid completely filled with walls (unvisited cells).
    std::vector<std::vector<int>> grid(h, std::vector<int>(w, 1)); // 1 = WALL

    struct Cell {
        int r, c;
    };

    std::vector<Cell> frontier;
    std::vector<std::vector<bool>> in_frontier(h, std::vector<bool>(w, false));

    auto is_valid_cell = [&](int r, int c) {
        return r > 0 && r < h - 1 && c > 0 && c < w - 1;
    };

    std::random_device rd;
    std::mt19937 gen(rd());

    int start_r = 1;
    int start_c = 1;
    grid[start_r][start_c] = 0; // 0 = PASSAGE ("IN")

    int dr[] = {-2, 2, 0, 0};
    int dc[] = {0, 0, -2, 2};

    for (int i = 0; i < 4; ++i) {
        int nr = start_r + dr[i];
        int nc = start_c + dc[i];
        if (is_valid_cell(nr, nc) && grid[nr][nc] == 1) {
            frontier.push_back({nr, nc});
            in_frontier[nr][nc] = true;
        }
    }

    while (!frontier.empty()) {
        std::uniform_int_distribution<> dis_f(0, frontier.size() - 1);
        int idx = dis_f(gen);
        Cell current = frontier[idx];

        std::vector<Cell> in_neighbors;
        for (int i = 0; i < 4; ++i) {
            int nr = current.r + dr[i];
            int nc = current.c + dc[i];
            if (is_valid_cell(nr, nc) && grid[nr][nc] == 0) {
                in_neighbors.push_back({nr, nc});
            }
        }

        if (!in_neighbors.empty()) {
            std::uniform_int_distribution<> dis_n(0, in_neighbors.size() - 1);
            Cell chosen_in = in_neighbors[dis_n(gen)];

            int wall_r = (current.r + chosen_in.r) / 2;
            int wall_c = (current.c + chosen_in.c) / 2;
            grid[wall_r][wall_c] = 0; // Set to passage
        }

        grid[current.r][current.c] = 0;

        for (int i = 0; i < 4; ++i) {
            int nr = current.r + dr[i];
            int nc = current.c + dc[i];
            if (is_valid_cell(nr, nc) && grid[nr][nc] == 1 && !in_frontier[nr][nc]) {
                frontier.push_back({nr, nc});
                in_frontier[nr][nc] = true;
            }
        }

        if (idx != frontier.size() - 1) {
            std::swap(frontier[idx], frontier.back());
        }
        frontier.pop_back();
    }

    // Ensure the exit cell is open
    grid[exit_r][exit_c] = 0;
    return grid;
}

void DungeonState::update_visited(DungeonFloor& floor) {
    int pr = floor.player_r;
    int pc = floor.player_c;
    // Mark a radius of 2 as visited
    for (int dr = -2; dr <= 2; ++dr) {
        for (int dc = -2; dc <= 2; ++dc) {
            int vr = pr + dr;
            int vc = pc + dc;
            if (vr >= 0 && vr < floor.height && vc >= 0 && vc < floor.width) {
                floor.visited[vr][vc] = true;
            }
        }
    }
}

void DungeonState::handle_input(int ch) {
    if (ch == KEY_RESIZE) {
        engine->get_layout().resize();
        if (current_choice_popup) current_choice_popup->resize();
        return;
    }

    if (engine->get_dialogs().has_active_choices()) {
        int idx = engine->get_dialogs().get_selected_choice_index();
        int count = engine->get_dialogs().get_active_choices().size();
        if (ch == KEY_UP || ch == 'w' || ch == KEY_LEFT || ch == 'a') {
            idx--;
            if (idx < 0) idx = count - 1;
            engine->get_dialogs().set_selected_choice_index(idx);
            engine->get_music_manager().playSfx("select_001.mp3");
        } else if (ch == KEY_DOWN || ch == 's' || ch == KEY_RIGHT || ch == 'd') {
            idx++;
            if (idx >= count) idx = 0;
            engine->get_dialogs().set_selected_choice_index(idx);
            engine->get_music_manager().playSfx("select_001.mp3");
        } else if (ch == '\n' || ch == ' ') {
            engine->get_dialogs().select_choice(idx, engine);
            if (!engine->get_dialogs().has_active_choices() && !engine->get_dialogs().has_queued_dialog()) {
                engine->get_dialogs().set_on_exit({});
                engine->get_dialogs().set_next_scene("");
                engine->get_music_manager().playMusic("dungeon.mp3");
            }
        }
        return;
    }

    if (engine->get_dialogs().has_queued_dialog() || engine->get_dialogs().has_queued_popup()) return;

    if (has_won) {
        engine->pop_state();
        return;
    }

    if (ch == 'q' || ch == 'Q') {
        Utils::Logger::log("DungeonState: Player chose to forfeit and return to town.");
        engine->pop_state();
        return;
    }
    
    if (ch == 'c' || ch == 'C') {
        engine->push_state(new StatAllocationState(engine));
        return;
    }
    
    if (ch == 'i' || ch == 'I') {
        engine->push_state(new InventoryState(engine));
        return;
    }

    // Tab toggling controls
    if (ch == '\t' || ch == 't' || ch == 'T') {
        active_tab = (active_tab + 1) % 2;
        return;
    } else if (ch == '1') {
        active_tab = 0;
        return;
    } else if (ch == '2') {
        active_tab = 1;
        return;
    }

    DungeonFloorNode* current_node = dungeon.get_current_node();
    if (!current_node) return;

    DungeonFloor& current_floor = current_node->floor;

    int next_r = current_floor.player_r;
    int next_c = current_floor.player_c;

    if (ch == KEY_UP || ch == 'w' || ch == 'W') {
        next_r--;
    } else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
        next_r++;
    } else if (ch == KEY_LEFT || ch == 'a' || ch == 'A') {
        next_c--;
    } else if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') {
        next_c++;
    }

    bool moved = false;
    // Verify cell boundaries and ensure the destination is not a wall
    if (next_r >= 0 && next_r < current_floor.height && next_c >= 0 && next_c < current_floor.width) {
        if (current_floor.grid[next_r][next_c] == 0) {
            moved = (current_floor.player_r != next_r || current_floor.player_c != next_c);
            current_floor.player_r = next_r;
            current_floor.player_c = next_c;
            update_visited(current_floor); // Update fog of war
            
            if (moved) {
                for (auto it = quest_markers.begin(); it != quest_markers.end(); ) {
                    if (it->floor_number == current_floor.floor_number &&
                        current_floor.player_r == it->y && current_floor.player_c == it->x) {
                        std::string scene_id = it->scene_id;
                        it = quest_markers.erase(it);
                        if (!scene_id.empty()) {
                            const DialogScene* scene = engine->get_db().get_dialog_scene(scene_id);
                            if (scene) {
                                engine->get_dialogs().start_scene(*scene, engine);
                            }
                        }
                        return;
                    } else {
                        ++it;
                    }
                }

                if (!(current_floor.player_r == current_floor.exit_r && current_floor.player_c == current_floor.exit_c) && !(current_floor.player_r == current_floor.start_r && current_floor.player_c == current_floor.start_c)) {
                    int encounter_chance = rand() % 100;
                    if (encounter_chance < 10) { // 10% chance to encounter enemies
                        engine->push_state(new BattleState(engine, ""));
                        return;
                    }
                }
            }
        }
    }

    // Manual stair interaction instead of automatic
    if (ch == 'e' || ch == 'E' || ch == '\n') {
        if (current_floor.player_r == current_floor.exit_r && current_floor.player_c == current_floor.exit_c) {
            if (current_node->next != nullptr) {
                dungeon.go_to_next_floor();
                DungeonFloor& next_floor = dungeon.get_current_node()->floor;
                next_floor.player_r = next_floor.start_r;
                next_floor.player_c = next_floor.start_c;
                update_visited(next_floor);
                Player* p = engine->get_player_manager().get_player();
                if (p) {
                    p->set_var("dungeon_floor", next_floor.floor_number);
                }
                Utils::Logger::log("DungeonState: Player descended to Floor " + std::to_string(next_floor.floor_number));
            } else {
                has_won = true;
                Player* p = engine->get_player_manager().get_player();
                if (p) p->add_gold(50);
                Utils::Logger::log("DungeonState: Player cleared the final floor and won the dungeon!");
            }
        }
        else if (current_floor.player_r == current_floor.start_r && current_floor.player_c == current_floor.start_c) {
            if (current_node->prev != nullptr) {
                dungeon.go_to_prev_floor();
                DungeonFloor& prev_floor = dungeon.get_current_node()->floor;
                prev_floor.player_r = prev_floor.exit_r;
                prev_floor.player_c = prev_floor.exit_c;
                update_visited(prev_floor);
                Player* p = engine->get_player_manager().get_player();
                if (p) {
                    p->set_var("dungeon_floor", prev_floor.floor_number);
                }
                Utils::Logger::log("DungeonState: Player ascended back to Floor " + std::to_string(prev_floor.floor_number));
            }
        }
    }
}

void DungeonState::update() {
    if (engine->get_dialogs().has_queued_dialog()) {
        process_dialogue_queue();
        if (!engine->get_dialogs().has_queued_dialog()) {
            handle_post_dialogue();
        }
    }
}

void DungeonState::render() {
    int my, mx;
    getmaxyx(stdscr, my, mx);
    if (my < 24 || mx < 80) {
        mvprintw(my / 2, std::max(0, (mx - 30) / 2), "TERMINAL TERLALU KECIL (%dx%d)", mx, my);
        refresh();
        return;
    }

    DungeonFloorNode* current_node = dungeon.get_current_node();
    if (!current_node) return;

    DungeonFloor& current_floor = current_node->floor;

    // 1. Draw layout grids of MainPage
    engine->get_layout().draw();

    // 2. Render Dialogue Panel
    engine->get_layout().render_history(
        engine->get_layout().win_dialog, 
        engine->get_dialogs().get_combined_log()
    );

    // 3. Render Player stats
    Player* p = engine->get_player_manager().get_player();
    if (p) {
        std::vector<std::string> equip_info;
        const auto& equips = p->get_all_equipped();
        auto format_equip = [&](const std::string& slot, const std::string& label) {
            auto it = equips.find(slot);
            equip_info.push_back(label + ": " + (it != equips.end() && it->second ? it->second->name : "(Kosong)"));
        };
        format_equip("weapon", "Sjt"); 
        format_equip("armor", "Zir"); 
        format_equip("boots", "Sep"); 
        format_equip("ring", "Cin");

        engine->get_layout().draw_player_stats(
            engine->get_layout().win_stat, 
            p->get_str(), p->get_cons(), p->get_agi(), p->get_intl(), p->get_wis(), 
            p->get_stat_points(),
            element_to_string(p->get_affinity()), p->get_gold(), equip_info
        );
        
        engine->get_layout().draw_vitals(
            engine->get_layout().win_hp, 
            p->get_hp(), p->get_max_hp(), p->get_mp(), p->get_max_mp()
        );
    }

    // 4. Render Calendar
    int day = engine->get_calendar().getDay();
    engine->get_layout().draw_calendar(
        engine->get_layout().win_cal, 
        std::max(0, 15 - day), 
        engine->get_calendar().getMonth(), 
        day, 
        engine->get_calendar().getTimeString(), 
        "Dungeon F" + std::to_string(current_floor.floor_number)
    );

    // 5. Render active quests in task panel
    std::vector<std::string> task_list;
    task_list.push_back("Misi Aktif:");
    bool has_quests = false;
    for (const auto& pair : engine->get_quests().get_all_quests()) {
        Quest* q = pair.second;
        if (q && q->get_state() == QuestState::IN_PROGRESS) {
            task_list.push_back("- " + q->get_name());
            has_quests = true;
        }
    }
    if (!has_quests) {
        task_list.push_back("- Tidak ada misi");
    }
    engine->get_layout().draw_tasks(engine->get_layout().win_task, task_list);

    // 6. Render Party and Map tabs in Thoughts container (win_thought)
    render_thought_tabs();

    // 7. Render Viewport Maze inside actions container (win_menu)
    WINDOW* win = engine->get_layout().win_menu;
    if (win) {
        int wy, wx;
        getmaxyx(win, wy, wx);

        // Clear and draw active box
        wattron(win, COLOR_PAIR(4) | A_BOLD);
        werase(win);
        box(win, 0, 0);
        engine->get_layout().draw_title(win, "Dungeon", 4);
        wattroff(win, COLOR_PAIR(4) | A_BOLD);

        int view_h = wy - 2;
        int view_w = (wx - 2) / 2;

        int pr = current_floor.player_r;
        int pc = current_floor.player_c;

        int camera_r = pr - view_h / 2;
        int camera_c = pc - view_w / 2;

        // Clamp camera to boundaries
        if (camera_r < 0) camera_r = 0;
        if (camera_c < 0) camera_c = 0;
        if (camera_r + view_h > current_floor.height) camera_r = std::max(0, current_floor.height - view_h);
        if (camera_c + view_w > current_floor.width) camera_c = std::max(0, current_floor.width - view_w);

        for (int vr = 0; vr < view_h; ++vr) {
            int r = camera_r + vr;
            if (r >= current_floor.height) break;

            for (int vc = 0; vc < view_w; ++vc) {
                int c = camera_c + vc;
                if (c >= current_floor.width) break;

                int draw_y = 1 + vr;
                int draw_x = 1 + (vc * 2);

                if (r == pr && c == pc) {
                    wattron(win, COLOR_PAIR(4) | A_BOLD);
                    mvwprintw(win, draw_y, draw_x, "@ ");
                    wattroff(win, COLOR_PAIR(4) | A_BOLD);
                } else if (auto m_it = std::find_if(quest_markers.begin(), quest_markers.end(),
                    [&](const QuestMarker& m) { return m.floor_number == current_floor.floor_number && m.y == r && m.x == c; });
                    m_it != quest_markers.end()) {
                    wattron(win, COLOR_PAIR(5) | A_BOLD);
                    mvwprintw(win, draw_y, draw_x, "? ");
                    wattroff(win, COLOR_PAIR(5) | A_BOLD);
                } else if (r == current_floor.exit_r && c == current_floor.exit_c) {
                    wattron(win, COLOR_PAIR(5) | A_BOLD);
                    if (current_node->next != nullptr) {
                        mvwprintw(win, draw_y, draw_x, "DN");
                    } else {
                        mvwprintw(win, draw_y, draw_x, "EX");
                    }
                    wattroff(win, COLOR_PAIR(5) | A_BOLD);
                } else if (r == current_floor.start_r && c == current_floor.start_c && current_node->prev != nullptr) {
                    wattron(win, COLOR_PAIR(4) | A_BOLD);
                    mvwprintw(win, draw_y, draw_x, "UP");
                    wattroff(win, COLOR_PAIR(4) | A_BOLD);
                } else if (current_floor.grid[r][c] == 1) {
                    wattron(win, COLOR_PAIR(3));
                    mvwprintw(win, draw_y, draw_x, "██");
                    wattroff(win, COLOR_PAIR(3));
                } else {
                    wattron(win, COLOR_PAIR(2));
                    mvwprintw(win, draw_y, draw_x, "  ");
                    wattroff(win, COLOR_PAIR(2));
                }
            }
        }

        // Draw Interaction Hint if on stairs
        int hint_max = wx - 4;
        if (hint_max < 4) hint_max = 4;
        if (pr == current_floor.exit_r && pc == current_floor.exit_c) {
            wattron(win, COLOR_PAIR(5) | A_BOLD);
            if (current_node->next != nullptr) {
                mvwprintw(win, wy - 1, 2, "%.*s", hint_max, " [E/Enter] Turun Lantai ");
            } else {
                mvwprintw(win, wy - 1, 2, "%.*s", hint_max, " [E/Enter] Keluar Dungeon ");
            }
            wattroff(win, COLOR_PAIR(5) | A_BOLD);
        } else if (pr == current_floor.start_r && pc == current_floor.start_c && current_node->prev != nullptr) {
            wattron(win, COLOR_PAIR(4) | A_BOLD);
            mvwprintw(win, wy - 1, 2, "%.*s", hint_max, " [E/Enter] Naik Lantai ");
            wattroff(win, COLOR_PAIR(4) | A_BOLD);
        }

        wnoutrefresh(win);
    }

    // Render Win Modal Overlay if player won
    if (has_won) {
        int popup_w = 40;
        int popup_h = 7;
        int popup_y = (LINES - popup_h) / 2;
        int popup_x = (COLS - popup_w) / 2;

        WINDOW* pop_win = newwin(popup_h, popup_w, popup_y, popup_x);
        if (pop_win) {
            wbkgdset(pop_win, COLOR_PAIR(2));
            werase(pop_win);
            box(pop_win, 0, 0);

            wattron(pop_win, COLOR_PAIR(4) | A_BOLD);
            mvwprintw(pop_win, 2, (popup_w - 18) / 2, "DUNGEON COMPLETED!");
            wattroff(pop_win, COLOR_PAIR(4) | A_BOLD);

            wattron(pop_win, COLOR_PAIR(2));
            mvwprintw(pop_win, 3, (popup_w - 24) / 2, "Reward: +50 Gold Coins!");
            mvwprintw(pop_win, 4, (popup_w - 28) / 2, "Press any key to return...");
            wattroff(pop_win, COLOR_PAIR(2));

            wrefresh(pop_win);
            delwin(pop_win);
        }
    }

    // Render Choice Popup
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
    }
}

void DungeonState::render_thought_tabs() {
    WINDOW* win = engine->get_layout().win_thought;
    if (!win) return;

    int wy, wx;
    getmaxyx(win, wy, wx);

    wattron(win, COLOR_PAIR(2));
    werase(win);
    box(win, 0, 0);
    engine->get_layout().draw_title(win, "Pikiran / Log", 2);
    wattroff(win, COLOR_PAIR(2));

    // Render Tab bar
    int tab_party_x = 4;
    int tab_map_x = 22;

    if (active_tab == 0) {
        wattron(win, A_REVERSE | COLOR_PAIR(4) | A_BOLD);
        mvwprintw(win, 1, tab_party_x, " [1] PARTY ");
        wattroff(win, A_REVERSE | COLOR_PAIR(4) | A_BOLD);
    } else {
        wattron(win, COLOR_PAIR(2));
        mvwprintw(win, 1, tab_party_x, "  [1] Party  ");
        wattroff(win, COLOR_PAIR(2));
    }

    if (active_tab == 1) {
        wattron(win, A_REVERSE | COLOR_PAIR(4) | A_BOLD);
        mvwprintw(win, 1, tab_map_x, " [2] MAP ");
        wattroff(win, A_REVERSE | COLOR_PAIR(4) | A_BOLD);
    } else {
        wattron(win, COLOR_PAIR(2));
        mvwprintw(win, 1, tab_map_x, "  [2] Map  ");
        wattroff(win, COLOR_PAIR(2));
    }

    // Draw horizontal separator line
    wattron(win, COLOR_PAIR(2));
    for (int x = 1; x < wx - 1; ++x) {
        mvwaddch(win, 2, x, '-');
    }
    wattroff(win, COLOR_PAIR(2));

    // Render selected tab content
    if (active_tab == 0) {
        render_party_tab(win);
    } else {
        DungeonFloorNode* current_node = dungeon.get_current_node();
        if (current_node) {
            render_map_tab(win, current_node->floor);
        }
    }
    wnoutrefresh(win);
}

void DungeonState::render_party_tab(WINDOW* win) {
    int wy, wx;
    getmaxyx(win, wy, wx);

    int col_w = (wx - 5) / 4; // 4 slots with borders in between

    // Draw vertical column separators
    wattron(win, COLOR_PAIR(2));
    for (int y = 3; y < wy - 1; ++y) {
        mvwaddch(win, y, 2 + col_w, '|');
        mvwaddch(win, y, 2 + col_w * 2 + 1, '|');
        mvwaddch(win, y, 2 + col_w * 3 + 2, '|');
    }
    wattroff(win, COLOR_PAIR(2));

    const auto& party = engine->get_player_manager().get_party_slots();

    for (int i = 0; i < 4; ++i) {
        int x_offset = 2 + (col_w + 1) * i + 2;
        int slot_w = col_w - 2;
        
        if (party[i] == nullptr) {
            wattron(win, COLOR_PAIR(2));
            mvwprintw(win, 3, x_offset, "%.*s", slot_w, ("Slot " + std::to_string(i + 1)).c_str());
            mvwprintw(win, 4, x_offset, "%.*s", slot_w, "(Kosong)");
            wattroff(win, COLOR_PAIR(2));
        } else {
            if (i == 0 && party[i]->get_id() == "hero") {
                wattron(win, COLOR_PAIR(4) | A_BOLD);
                mvwprintw(win, 3, x_offset, "%.*s", slot_w, "Leader");
                wattroff(win, COLOR_PAIR(4) | A_BOLD);
            } else {
                wattron(win, COLOR_PAIR(3) | A_BOLD);
                mvwprintw(win, 3, x_offset, "%.*s", slot_w, ("Ally " + std::to_string(i + 1)).c_str());
                wattroff(win, COLOR_PAIR(3) | A_BOLD);
            }
            
            mvwprintw(win, 4, x_offset, "%.*s", slot_w, party[i]->get_name().c_str());
            std::string hp_str = "HP:" + std::to_string(party[i]->get_hp()) + "/" + std::to_string(party[i]->get_max_hp());
            mvwprintw(win, 5, x_offset, "%.*s", slot_w, hp_str.c_str());
            std::string mp_str = "MP:" + std::to_string(party[i]->get_mp()) + "/" + std::to_string(party[i]->get_max_mp());
            mvwprintw(win, 6, x_offset, "%.*s", slot_w, mp_str.c_str());
        }
    }
}

void DungeonState::render_map_tab(WINDOW* win, const DungeonFloor& floor) {
    int wy, wx;
    getmaxyx(win, wy, wx);

    int map_h = wy - 4; // Height from row 3 to wy-2
    int map_w = (wx - 4) / 2;

    int pr = floor.player_r;
    int pc = floor.player_c;

    int camera_r = pr - map_h / 2;
    int camera_c = pc - map_w / 2;

    if (camera_r < 0) camera_r = 0;
    if (camera_c < 0) camera_c = 0;
    if (camera_r + map_h > floor.height) camera_r = floor.height - map_h;
    if (camera_c + map_w > floor.width) camera_c = floor.width - map_w;

    if (camera_r < 0) camera_r = 0;
    if (camera_c < 0) camera_c = 0;

    for (int vr = 0; vr < map_h; ++vr) {
        int r = camera_r + vr;
        if (r >= floor.height) break;

        for (int vc = 0; vc < map_w; ++vc) {
            int c = camera_c + vc;
            if (c >= floor.width) break;

            int draw_y = 3 + vr;
            int draw_x = 2 + (vc * 2);

            if (floor.visited[r][c]) {
                if (r == pr && c == pc) {
                    wattron(win, COLOR_PAIR(4) | A_BOLD);
                    mvwprintw(win, draw_y, draw_x, "@ ");
                    wattroff(win, COLOR_PAIR(4) | A_BOLD);
                } else if (auto m_it = std::find_if(quest_markers.begin(), quest_markers.end(),
                    [&](const QuestMarker& m) { return m.floor_number == floor.floor_number && m.y == r && m.x == c; });
                    m_it != quest_markers.end()) {
                    wattron(win, COLOR_PAIR(5) | A_BOLD);
                    mvwprintw(win, draw_y, draw_x, "? ");
                    wattroff(win, COLOR_PAIR(5) | A_BOLD);
                } else if (r == floor.exit_r && c == floor.exit_c) {
                    wattron(win, COLOR_PAIR(5) | A_BOLD);
                    if (dungeon.get_current_node()->next != nullptr) {
                        mvwprintw(win, draw_y, draw_x, "DN");
                    } else {
                        mvwprintw(win, draw_y, draw_x, "EX");
                    }
                    wattroff(win, COLOR_PAIR(5) | A_BOLD);
                } else if (r == floor.start_r && c == floor.start_c && dungeon.get_current_node()->prev != nullptr) {
                    wattron(win, COLOR_PAIR(4) | A_BOLD);
                    mvwprintw(win, draw_y, draw_x, "UP");
                    wattroff(win, COLOR_PAIR(4) | A_BOLD);
                } else if (floor.grid[r][c] == 1) {
                    wattron(win, COLOR_PAIR(2));
                    mvwprintw(win, draw_y, draw_x, "░░");
                    wattroff(win, COLOR_PAIR(2));
                } else {
                    wattron(win, COLOR_PAIR(2));
                    mvwprintw(win, draw_y, draw_x, "· ");
                    wattroff(win, COLOR_PAIR(2));
                }
            } else {
                mvwprintw(win, draw_y, draw_x, "  ");
            }
        }
    }
}

void DungeonState::process_dialogue_queue() {
    DialogNode node = engine->get_dialogs().pop_dialog();
    if (node.type == 1 || node.type == 2) {
        std::function<void()> start_cb = [this]() {
            this->engine->get_music_manager().startTypingSfx("typingText.mp3");
        };
        std::function<void()> stop_cb = [this]() {
            this->engine->get_music_manager().stopTypingSfx();
        };
        engine->get_layout().type_new_text(engine->get_layout().win_dialog, "Dialog", engine->get_layout().w_left, engine->get_dialogs().get_combined_log(), node, start_cb, stop_cb);
        if (node.type == 1) engine->get_dialogs().add_dialog(node);
        else engine->get_dialogs().add_thought(node);
    } else if (node.type == 3) {
        engine->get_dialogs().queue_popup(node.value, node.npc_name == "Narrator"); 
        engine->get_dialogs().add_popup(node);
    }
    this->render();
}

void DungeonState::handle_post_dialogue() {
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
            engine->get_music_manager().playMusic("dungeon.mp3");
        }
    }
}