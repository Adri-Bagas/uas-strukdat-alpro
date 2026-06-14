#include "DungeonState.hpp"
#include "../GameEngine.hpp"
#include "../utils/Logger.hpp"
#include <ncurses.h>
#include <random>
#include <algorithm>

DungeonState::DungeonState(GameEngine* eng) 
    : GameState(eng), height(17), width(41), player_r(1), player_c(1), exit_r(15), exit_c(39), has_won(false) {
    grid.assign(height, std::vector<int>(width, 1));
}

void DungeonState::on_enter() {
    Logger::log("DungeonState: Generating a new maze via Randomized Prim's Algorithm...");
    generate_maze();
    player_r = 1;
    player_c = 1;
    has_won = false;
}

void DungeonState::generate_maze() {
    // Step 1: Initialize the target grid completely filled with walls (unvisited cells).
    grid.assign(height, std::vector<int>(width, 1)); // 1 = WALL

    struct Cell {
        int r, c;
    };

    std::vector<Cell> frontier;
    // Auxiliary 2D grid to quickly check if a cell is already in the frontier list
    std::vector<std::vector<bool>> in_frontier(height, std::vector<bool>(width, false));

    // Lambda to check if coordinates are within the maze boundary (leaving index 0 and size-1 as permanent outer borders)
    auto is_valid_cell = [&](int r, int c) {
        return r > 0 && r < height - 1 && c > 0 && c < width - 1;
    };

    std::random_device rd;
    std::mt19937 gen(rd());

    // Step 2: Pick a random starting cell and mark it as "IN" the maze.
    // We choose (1, 1) as our starting cell to guarantee it's open and start the player there.
    int start_r = 1;
    int start_c = 1;
    grid[start_r][start_c] = 0; // 0 = PASSAGE ("IN")

    // Directions for cell movement at distance 2 (up, down, left, right)
    int dr[] = {-2, 2, 0, 0};
    int dc[] = {0, 0, -2, 2};

    // Step 3: Identify all valid neighboring wall cells of this start cell and add them to a "Frontier" collection.
    for (int i = 0; i < 4; ++i) {
        int nr = start_r + dr[i];
        int nc = start_c + dc[i];
        if (is_valid_cell(nr, nc) && grid[nr][nc] == 1) {
            frontier.push_back({nr, nc});
            in_frontier[nr][nc] = true;
        }
    }

    // Step 4: Loop until the Frontier collection is empty.
    while (!frontier.empty()) {
        // Pick a random cell from the Frontier.
        std::uniform_int_distribution<> dis_f(0, frontier.size() - 1);
        int idx = dis_f(gen);
        Cell current = frontier[idx];

        // Find all of its valid neighbors that are already marked as "IN".
        std::vector<Cell> in_neighbors;
        for (int i = 0; i < 4; ++i) {
            int nr = current.r + dr[i];
            int nc = current.c + dc[i];
            if (is_valid_cell(nr, nc) && grid[nr][nc] == 0) {
                in_neighbors.push_back({nr, nc});
            }
        }

        if (!in_neighbors.empty()) {
            // Pick one of those "IN" neighbors at random.
            std::uniform_int_distribution<> dis_n(0, in_neighbors.size() - 1);
            Cell chosen_in = in_neighbors[dis_n(gen)];

            // Carve a path (remove the wall) between the chosen Frontier cell and the chosen "IN" neighbor.
            // The wall cell is at the midpoint between the current frontier node and the chosen in-node.
            int wall_r = (current.r + chosen_in.r) / 2;
            int wall_c = (current.c + chosen_in.c) / 2;
            grid[wall_r][wall_c] = 0; // Set to passage
        }

        // Mark the chosen Frontier cell as "IN".
        grid[current.r][current.c] = 0;

        // Add any of its valid unvisited wall neighbors to the Frontier collection.
        for (int i = 0; i < 4; ++i) {
            int nr = current.r + dr[i];
            int nc = current.c + dc[i];
            if (is_valid_cell(nr, nc) && grid[nr][nc] == 1 && !in_frontier[nr][nc]) {
                frontier.push_back({nr, nc});
                in_frontier[nr][nc] = true;
            }
        }

        // Remove the processed cell from the Frontier collection.
        // Memory efficient swap-and-pop to avoid O(N) element shifting.
        if (idx != frontier.size() - 1) {
            std::swap(frontier[idx], frontier.back());
        }
        frontier.pop_back();
    }

    // Ensure the exit cell is open (odd coordinates should be open, but we force it just in case)
    grid[exit_r][exit_c] = 0;
}

void DungeonState::handle_input(int ch) {
    if (has_won) {
        // Any key returns to town state after winning
        engine->pop_state();
        return;
    }

    if (ch == 'q' || ch == 'Q') {
        Logger::log("DungeonState: Player chose to forfeit and return to town.");
        engine->pop_state();
        return;
    }

    int next_r = player_r;
    int next_c = player_c;

    if (ch == KEY_UP || ch == 'w' || ch == 'W') {
        next_r--;
    } else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
        next_r++;
    } else if (ch == KEY_LEFT || ch == 'a' || ch == 'A') {
        next_c--;
    } else if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') {
        next_c++;
    } else if (ch == KEY_RESIZE) {
        clear();
    }

    // Verify cell boundaries and ensure the destination is not a wall
    if (next_r >= 0 && next_r < height && next_c >= 0 && next_c < width) {
        if (grid[next_r][next_c] == 0) {
            player_r = next_r;
            player_c = next_c;
        }
    }

    // Check if player reached the exit
    if (player_r == exit_r && player_c == exit_c) {
        has_won = true;
        Player* p = engine->get_player_manager().get_player();
        if (p) {
            p->add_gold(50); // Reward the player with 50 gold for escaping the maze
        }
        Logger::log("DungeonState: Player reached the exit and won the maze!");
    }
}

void DungeonState::update() {
    // Animation updates, trap checks or other engine events
}

void DungeonState::render() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Clear the stdscr buffer to prevent rendering artifacts
    erase();

    // Guard against small terminal windows
    if (max_y < 22 || max_x < 110) {
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(max_y / 2, (max_x - 70) / 2, "Please resize terminal to at least 110x24 to play the Dungeon Maze.");
        mvprintw(max_y / 2 + 1, (max_x - 30) / 2, "Current size: %dx%d", max_x, max_y);
        attroff(COLOR_PAIR(5) | A_BOLD);
        return;
    }

    // 1. Draw outer double/single border around screen edges
    attron(COLOR_PAIR(2));
    for (int x = 0; x < max_x; ++x) {
        mvaddch(0, x, '-');
        mvaddch(max_y - 1, x, '-');
    }
    for (int y = 0; y < max_y; ++y) {
        mvaddch(y, 0, '|');
        mvaddch(y, max_x - 1, '|');
    }
    attroff(COLOR_PAIR(2));

    // 2. Draw Header Game Title
    attron(COLOR_PAIR(4) | A_BOLD);
    std::string title = "N I R V A   D U N G E O N   M A Z E";
    mvprintw(1, (max_x - title.length()) / 2, "%s", title.c_str());
    attroff(COLOR_PAIR(4) | A_BOLD);

    attron(COLOR_PAIR(2));
    for (int x = 1; x < max_x - 1; ++x) {
        mvaddch(2, x, '=');
    }
    attroff(COLOR_PAIR(2));

    // 3. Draw Maze (Left Side, centered vertically, starting at row 4, column 4)
    int start_y = 4;
    int start_x = 4;

    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            int draw_y = start_y + r;
            int draw_x = start_x + (c * 2); // 2 character width for square-like ratio

            if (r == player_r && c == player_c) {
                // Render the player
                attron(COLOR_PAIR(4) | A_BOLD);
                mvprintw(draw_y, draw_x, "@ ");
                attroff(COLOR_PAIR(4) | A_BOLD);
            } else if (r == exit_r && c == exit_c) {
                // Render the exit point
                attron(COLOR_PAIR(5) | A_BOLD);
                mvprintw(draw_y, draw_x, "EX");
                attroff(COLOR_PAIR(5) | A_BOLD);
            } else if (grid[r][c] == 1) {
                // Render the walls using double solid blocks
                attron(COLOR_PAIR(3));
                mvprintw(draw_y, draw_x, "██");
                attroff(COLOR_PAIR(3));
            } else {
                // Render the passage
                attron(COLOR_PAIR(2));
                mvprintw(draw_y, draw_x, "  ");
                attroff(COLOR_PAIR(2));
            }
        }
    }

    // 4. Draw vertical divider between the maze and the info panel
    attron(COLOR_PAIR(2));
    for (int y = 3; y < max_y - 1; ++y) {
        mvaddch(y, start_x + (width * 2) + 2, '|');
    }
    attroff(COLOR_PAIR(2));

    // 5. Draw Info Panel (Right Side of divider)
    int panel_x = start_x + (width * 2) + 5;

    // A. Player Information
    Player* p = engine->get_player_manager().get_player();
    if (p) {
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(4, panel_x, "--- PLAYER STATUS ---");
        attroff(COLOR_PAIR(4) | A_BOLD);

        attron(COLOR_PAIR(2));
        mvprintw(5, panel_x, "Name:    %s", p->get_name().c_str());
        mvprintw(6, panel_x, "HP:      %d / %d", p->get_hp(), p->get_max_hp());
        mvprintw(7, panel_x, "MP:      %d / %d", p->get_mp(), p->get_max_mp());
        mvprintw(8, panel_x, "Gold:    %d GP", p->get_gold());
        attroff(COLOR_PAIR(2));
    }

    // B. Dungeon Instructions
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(11, panel_x, "--- CONTROLS ---");
    attroff(COLOR_PAIR(4) | A_BOLD);

    attron(COLOR_PAIR(2));
    mvprintw(12, panel_x, "Move:    [W][A][S][D] or Arrow Keys");
    mvprintw(13, panel_x, "Escape:  Reach the 'EX' (Exit)");
    mvprintw(14, panel_x, "Forfeit: Press [Q] to return to town");
    attroff(COLOR_PAIR(2));

    // C. Legend info
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(16, panel_x, "--- LEGEND ---");
    attroff(COLOR_PAIR(4) | A_BOLD);

    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(17, panel_x, "@ ");
    attroff(COLOR_PAIR(4) | A_BOLD);
    attron(COLOR_PAIR(2));
    mvprintw(17, panel_x + 3, ": Player");
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(18, panel_x, "EX");
    attroff(COLOR_PAIR(5) | A_BOLD);
    attron(COLOR_PAIR(2));
    mvprintw(18, panel_x + 3, ": Exit");
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(3));
    mvprintw(19, panel_x, "██");
    attroff(COLOR_PAIR(3));
    attron(COLOR_PAIR(2));
    mvprintw(19, panel_x + 3, ": Wall");
    attroff(COLOR_PAIR(2));

    // D. Win screen overlay modal
    if (has_won) {
        int popup_w = 40;
        int popup_h = 7;
        int popup_y = (max_y - popup_h) / 2;
        int popup_x = (max_x - popup_w) / 2;

        // Draw overlay box background
        attron(COLOR_PAIR(2));
        for (int y = popup_y; y < popup_y + popup_h; ++y) {
            for (int x = popup_x; x < popup_x + popup_w; ++x) {
                mvaddch(y, x, ' ');
            }
        }
        // Draw popup borders
        for (int x = popup_x; x < popup_x + popup_w; ++x) {
            mvaddch(popup_y, x, '*');
            mvaddch(popup_y + popup_h - 1, x, '*');
        }
        for (int y = popup_y; y < popup_y + popup_h; ++y) {
            mvaddch(y, popup_x, '*');
            mvaddch(y, popup_x + popup_w - 1, '*');
        }
        attroff(COLOR_PAIR(2));

        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(popup_y + 2, popup_x + (popup_w - 18) / 2, "DUNGEON COMPLETED!");
        attroff(COLOR_PAIR(4) | A_BOLD);

        attron(COLOR_PAIR(2));
        mvprintw(popup_y + 3, popup_x + (popup_w - 24) / 2, "Reward: +50 Gold Coins!");
        mvprintw(popup_y + 4, popup_x + (popup_w - 28) / 2, "Press any key to return...");
        attroff(COLOR_PAIR(2));
    }
}