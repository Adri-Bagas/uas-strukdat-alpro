#include "DungeonState.hpp"
#include "../GameEngine.hpp"
#include "../utils/Logger.hpp"
#include "../enums/Element.hpp"
#include <ncurses.h>
#include <random>
#include <algorithm>
#include <string>

DungeonState::DungeonState(GameEngine* eng) 
    : GameState(eng), has_won(false), active_tab(0) {
}

int get_random_odd(int min_val, int max_val) {
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
    Logger::log("DungeonState: Generating multi-floor dungeon with random odd dimensions between 20-100...");
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
    }
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
    if (has_won) {
        engine->pop_state();
        return;
    }

    if (ch == 'q' || ch == 'Q') {
        Logger::log("DungeonState: Player chose to forfeit and return to town.");
        engine->pop_state();
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
    } else if (ch == KEY_RESIZE) {
        engine->get_layout().resize();
    }

    // Verify cell boundaries and ensure the destination is not a wall
    if (next_r >= 0 && next_r < current_floor.height && next_c >= 0 && next_c < current_floor.width) {
        if (current_floor.grid[next_r][next_c] == 0) {
            current_floor.player_r = next_r;
            current_floor.player_c = next_c;
            update_visited(current_floor); // Update fog of war
        }
    }

    // Check if player reached the exit
    if (current_floor.player_r == current_floor.exit_r && current_floor.player_c == current_floor.exit_c) {
        if (current_node->next != nullptr) {
            dungeon.go_to_next_floor();
            DungeonFloor& next_floor = dungeon.get_current_node()->floor;
            next_floor.player_r = next_floor.start_r;
            next_floor.player_c = next_floor.start_c;
            update_visited(next_floor);
            Logger::log("DungeonState: Player descended to Floor " + std::to_string(next_floor.floor_number));
        } else {
            has_won = true;
            Player* p = engine->get_player_manager().get_player();
            if (p) {
                p->add_gold(50);
            }
            Logger::log("DungeonState: Player cleared the final floor and won the dungeon!");
        }
    }
    // Check if player reached the start/entrance (to go back up)
    else if (current_floor.player_r == current_floor.start_r && current_floor.player_c == current_floor.start_c) {
        if (current_node->prev != nullptr) {
            dungeon.go_to_prev_floor();
            DungeonFloor& prev_floor = dungeon.get_current_node()->floor;
            prev_floor.player_r = prev_floor.exit_r;
            prev_floor.player_c = prev_floor.exit_c;
            update_visited(prev_floor);
            Logger::log("DungeonState: Player ascended back to Floor " + std::to_string(prev_floor.floor_number));
        }
    }
}

void DungeonState::update() {
}

void DungeonState::render() {
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
        engine->get_layout().draw_title(win, "Dungeon", wx, 4);
        wattroff(win, COLOR_PAIR(4) | A_BOLD);

        int view_h = (wy - 2) / 2;
        int view_w = (wx - 2) / 2;

        int pr = current_floor.player_r;
        int pc = current_floor.player_c;

        int camera_r = pr - view_h / 2;
        int camera_c = pc - view_w / 2;

        // Clamp camera to boundaries
        if (camera_r < 0) camera_r = 0;
        if (camera_c < 0) camera_c = 0;
        if (camera_r + view_h > current_floor.height) camera_r = current_floor.height - view_h;
        if (camera_c + view_w > current_floor.width) camera_c = current_floor.width - view_w;

        if (camera_r < 0) camera_r = 0;
        if (camera_c < 0) camera_c = 0;

        int start_draw_h = view_h / 2;

        for (int vr = 0; vr < view_h; ++vr) {
            int r = camera_r + vr;
            if (r >= current_floor.height) break;

            for (int vc = 0; vc < view_w; ++vc) {
                int c = camera_c + vc;
                if (c >= current_floor.width) break;

                int draw_y = start_draw_h + 1 + vr;
                int draw_x = 1 + (vc * 2);

                if (r == pr && c == pc) {
                    wattron(win, COLOR_PAIR(4) | A_BOLD);
                    mvwprintw(win, draw_y, draw_x, "@ ");
                    wattroff(win, COLOR_PAIR(4) | A_BOLD);
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
}

void DungeonState::render_thought_tabs() {
    WINDOW* win = engine->get_layout().win_thought;
    if (!win) return;

    int wy, wx;
    getmaxyx(win, wy, wx);

    wattron(win, COLOR_PAIR(2));
    werase(win);
    box(win, 0, 0);
    engine->get_layout().draw_title(win, "Pikiran / Log", wx, 2);
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
        
        if (party[i] == nullptr) {
            wattron(win, COLOR_PAIR(2));
            mvwprintw(win, 3, x_offset, "Slot %d", i + 1);
            mvwprintw(win, 4, x_offset, "(Kosong)");
            wattroff(win, COLOR_PAIR(2));
        } else {
            if (i == 0 && party[i]->get_id() == "hero") { // Just a visual highlight for the player
                wattron(win, COLOR_PAIR(4) | A_BOLD);
                mvwprintw(win, 3, x_offset, "Leader");
                wattroff(win, COLOR_PAIR(4) | A_BOLD);
            } else {
                wattron(win, COLOR_PAIR(3) | A_BOLD);
                mvwprintw(win, 3, x_offset, "Ally %d", i + 1);
                wattroff(win, COLOR_PAIR(3) | A_BOLD);
            }
            
            mvwprintw(win, 4, x_offset, "%s", party[i]->get_name().c_str());
            mvwprintw(win, 5, x_offset, "HP: %d/%d", party[i]->get_hp(), party[i]->get_max_hp());
            mvwprintw(win, 6, x_offset, "MP: %d/%d", party[i]->get_mp(), party[i]->get_max_mp());
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