#include "MainPage.hpp"
#include <cstring>
#include <ncurses.h>
#include <sstream>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <cmath>

const char* MainPage::big_digits[10][5] = {
    {"███", "█ █", "█ █", "█ █", "███"}, // 0
    {" ██", "  █", "  █", "  █", "  █"}, // 1
    {"███", "  █", "███", "█  ", "███"}, // 2
    {"███", "  █", "███", "  █", "███"}, // 3
    {"█ █", "█ █", "███", "  █", "  █"}, // 4
    {"███", "█  ", "███", "  █", "███"}, // 5
    {"███", "█  ", "███", "█ █", "███"}, // 6
    {"███", "  █", "  █", "  █", "  █"}, // 7
    {"███", "█ █", "███", "█ █", "███"}, // 8
    {"███", "█ █", "███", "  █", "███"}  // 9
};

void MainPage::create_windows() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    if (max_y < 22 || max_x < 80) {
        win_thought = win_dialog = win_stat = win_hp = win_cal = win_menu = win_task = nullptr;
        return;
    }

    int left_w = max_x * 0.3;
    int right_w = max_x - left_w;
    int top_h = max_y * 0.4;
    int bot_h = max_y - top_h;
    int col1_w = right_w * 0.35;
    int col2_w = right_w * 0.35;
    int col3_w = right_w - col1_w - col2_w;
    int box_h = bot_h / 3;

    w_left = left_w; w_right = right_w;
    w_col1 = col1_w; w_col2 = col2_w; w_col3 = col3_w;

    win_dialog = newwin(max_y, left_w, 0, 0);
    win_thought = newwin(top_h, right_w, 0, left_w);
    win_stat = newwin(box_h, col1_w, top_h, left_w);
    win_hp = newwin(box_h, col1_w, top_h + box_h, left_w);
    win_cal = newwin(bot_h - (box_h * 2), col1_w, top_h + (box_h * 2), left_w);
    win_menu = newwin(bot_h, col2_w, top_h, left_w + col1_w);
    win_task = newwin(bot_h, col3_w, top_h, left_w + col1_w + col2_w);

    if (!win_dialog || !win_thought || !win_stat || !win_hp || !win_cal || !win_menu || !win_task) {
        destroy_windows();
    }
}

void MainPage::destroy_windows() {
    auto safe_del = [](WINDOW*& w) { if (w) { delwin(w); w = nullptr; } };
    safe_del(win_thought); safe_del(win_dialog); safe_del(win_stat);
    safe_del(win_hp); safe_del(win_cal); safe_del(win_menu); safe_del(win_task);
}

MainPage::MainPage() {
    win_thought = win_dialog = win_stat = win_hp = win_cal = win_menu = win_task = nullptr;
    create_windows();
}

void MainPage::draw_title(WINDOW *win, const char *title, int width, int color_pair_id) {
    if (!win) return;
    wattron(win, A_BOLD | A_REVERSE | COLOR_PAIR(color_pair_id));
    mvwprintw(win, 0, 1, " %s ", title);
    wattroff(win, A_BOLD | A_REVERSE | COLOR_PAIR(color_pair_id));
}

void MainPage::draw() {
    if (!is_initialized()) {
        erase();
        mvprintw(LINES / 2, (COLS - 30) / 2, "TERMINAL TERLALU KECIL (%dx%d)", COLS, LINES);
        mvprintw(LINES / 2 + 1, (COLS - 30) / 2, "Dibutuhkan: 80x22");
        refresh(); return;
    }

    wbkgdset(win_dialog, COLOR_PAIR(2)); werase(win_dialog); box(win_dialog, 0, 0);
    draw_title(win_dialog, "Dialog", w_left, 2); wnoutrefresh(win_dialog);

    wbkgdset(win_thought, COLOR_PAIR(2)); werase(win_thought); box(win_thought, 0, 0);
    draw_title(win_thought, "Peta Kota", w_right, 2); wnoutrefresh(win_thought);

    wbkgdset(win_stat, COLOR_PAIR(2)); werase(win_stat); box(win_stat, 0, 0);
    draw_title(win_stat, "Atribut", w_col1, 2); wnoutrefresh(win_stat);

    wbkgdset(win_hp, COLOR_PAIR(2)); werase(win_hp); box(win_hp, 0, 0);
    draw_title(win_hp, "Vitalitas", w_col1, 2); wnoutrefresh(win_hp);

    wbkgdset(win_cal, COLOR_PAIR(2)); werase(win_cal); box(win_cal, 0, 0);
    draw_title(win_cal, "Status Dunia", w_col1, 2); wnoutrefresh(win_cal);

    wbkgdset(win_menu, COLOR_PAIR(2)); werase(win_menu); box(win_menu, 0, 0);
    draw_title(win_menu, "Tindakan Tersedia", w_col2, 2); wnoutrefresh(win_menu);

    wbkgdset(win_task, COLOR_PAIR(2)); werase(win_task); box(win_task, 0, 0);
    draw_title(win_task, "Inventaris & Misi", w_col3, 2); wnoutrefresh(win_task);
}

void MainPage::resize() {
    destroy_windows();
    resizeterm(0, 0); 
    endwin();
    refresh(); 
    clear();
    create_windows(); 
    draw();
}

bool MainPage::is_initialized() const { return (win_dialog != nullptr); }

MainPage::~MainPage() { destroy_windows(); }

void MainPage::type_new_text(WINDOW* win_in, const char* title, int width, 
                             const std::vector<DialogNode>& history, 
                             const DialogNode& new_text) {
    if (!win_in) return;
    
    WINDOW* win = win_in;
    int max_y, max_x; getmaxyx(win, max_y, max_x);
    int max_width = max_x - 4;

    auto wrap_node = [&](const DialogNode& node) {
        std::vector<std::string> lines;
        std::string name_line = node.npc_name.empty() ? "" : "[" + node.npc_name + "]";
        if (node.type == 2) {
            if (name_line.empty()) name_line = "(Pikiran)";
            else name_line += " (Pikiran)";
        }
        if (!name_line.empty()) lines.push_back(name_line);
        std::stringstream ss(node.value);
        std::string word, line = "";
        while (ss >> word) {
            if (line.empty()) line = word;
            else if (line.length() + 1 + word.length() <= (size_t)max_width) line += " " + word;
            else { lines.push_back(line); line = word; }
        }
        if (!line.empty()) lines.push_back(line);
        return lines;
    };

    auto redraw_all = [&]() {
        bool is_dialog = (win == win_dialog);
        bool is_thought = (win == win_thought);
        resize(); 
        if (is_dialog && win_dialog) win = win_dialog;
        else if (is_thought && win_thought) win = win_thought;
        else return;
        getmaxyx(win, max_y, max_x);
        max_width = max_x - 4;
        render_history(win, history);
        std::vector<std::string> lines = wrap_node(new_text);
        int start_y = (max_y - 1) - (int)lines.size();
        for (size_t i = 0; i < lines.size(); ++i) {
            if (start_y + i < max_y - 1) mvwprintw(win, start_y + i, 2, "%s", lines[i].c_str());
        }
        wnoutrefresh(win); doupdate();
    };

    wbkgdset(win, COLOR_PAIR(4) | A_BOLD); werase(win); box(win, 0, 0);
    draw_title(win, title, width, 4);

    std::vector<std::string> new_lines_all = wrap_node(new_text);
    int total_new_lines = new_lines_all.size();
    std::vector<std::string> history_all;
    for (const auto& node : history) {
        std::vector<std::string> node_lines = wrap_node(node);
        for (const auto& l : node_lines) history_all.push_back(l);
    }

    int available_history_lines = (max_y - 2) - total_new_lines;
    if (available_history_lines > 0) {
        int draw_count = std::min((int)history_all.size(), available_history_lines);
        int start_idx = history_all.size() - draw_count;
        int start_y = (max_y - 1) - total_new_lines - draw_count;
        for (int i = 0; i < draw_count; ++i) {
            if (win) mvwprintw(win, start_y + i, 2, "%s", history_all[start_idx + i].c_str());
        }
    }

    int anim_start_y = (max_y - 1) - total_new_lines;
    bool skipped = false;
    int current_line_in_block = 0;

    for (const auto& line_str : new_lines_all) {
        int current_y = anim_start_y + current_line_in_block;
        if (current_y >= max_y - 1) break;
        if (skipped) { if(win) mvwprintw(win, current_y, 2, "%s", line_str.c_str()); }
        else {
            int current_x = 2;
            for (char c : line_str) {
                if (!win) break;
                mvwaddch(win, current_y, current_x++, c);
                wnoutrefresh(win); doupdate();
                int ch = getch();
                if (ch != ERR) {
                    if (ch == KEY_RESIZE) { redraw_all(); skipped = true; break; }
                    skipped = true; mvwprintw(win, current_y, 2, "%s", line_str.c_str());
                    wnoutrefresh(win); doupdate(); break;
                }
                napms(20);
            }
        }
        current_line_in_block++;
    }
    napms(150); flushinp();
    while (true) {
        int ch = getch();
        if (ch == '\n' || ch == ' ') break;
        if (ch == KEY_RESIZE) redraw_all();
        napms(10);
    }
    if (win) { werase(win); wrefresh(win); }
}

void MainPage::render_history(WINDOW* win, const std::vector<DialogNode>& history) {
    if (!win) return;
    int max_y, max_x; getmaxyx(win, max_y, max_x);
    int max_width = max_x - 4;
    int available_lines = max_y - 2;

    auto wrap_node = [&](const DialogNode& node) {
        std::vector<std::string> lines;
        std::string name_line = node.npc_name.empty() ? "" : "[" + node.npc_name + "]";
        if (node.type == 2) {
            if (name_line.empty()) name_line = "(Pikiran)";
            else name_line += " (Pikiran)";
        }
        if (!name_line.empty()) lines.push_back(name_line);
        std::stringstream ss(node.value);
        std::string word, line = "";
        while (ss >> word) {
            if (line.empty()) line = word;
            else if (line.length() + 1 + word.length() <= (size_t)max_width) line += " " + word;
            else { lines.push_back(line); line = word; }
        }
        if (!line.empty()) lines.push_back(line);
        return lines;
    };

    std::vector<std::string> all_lines;
    for (const auto& node : history) {
        std::vector<std::string> node_lines = wrap_node(node);
        for (const auto& l : node_lines) all_lines.push_back(l);
    }

    int draw_count = std::min((int)all_lines.size(), available_lines);
    int start_idx = all_lines.size() - draw_count;
    int start_y = (max_y - 1) - draw_count;
    for (int i = 0; i < draw_count; ++i) mvwprintw(win, start_y + i, 2, "%s", all_lines[start_idx + i].c_str());
    wnoutrefresh(win); 
}

void MainPage::draw_calendar(WINDOW* win, int days_left, int month, int day, std::string time, std::string location_name) {
    if (!win) return;
    int max_y, max_x; getmaxyx(win, max_y, max_x);
    int total_height = 11;
    int start_y = std::max(1, (max_y - total_height) / 2);
    wattron(win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(win, start_y, std::max(1, (max_x - (int)location_name.length()) / 2), "%s", location_name.c_str());
    wattroff(win, COLOR_PAIR(4) | A_BOLD);
    start_y += 1;
    int tens = days_left / 10, ones = days_left % 10;
    int digits_width = (tens > 0) ? 7 : 3; 
    int digits_start_x = (max_x - digits_width) / 2;
    wattron(win, COLOR_PAIR(2) | A_BOLD);
    for (int row = 0; row < 5; ++row) {
        if (tens > 0) mvwprintw(win, start_y + row, digits_start_x, "%s", big_digits[tens][row]);
        mvwprintw(win, start_y + row, digits_start_x + (tens > 0 ? 4 : 0), "%s", big_digits[ones][row]);
    }
    wattroff(win, COLOR_PAIR(2) | A_BOLD);
    std::string title = "SISA HARI";
    mvwprintw(win, start_y + 6, (max_x - (int)title.length()) / 2, "%s", title.c_str());
    std::string date_str = "Tgl: " + std::string(month < 10 ? "0" : "") + std::to_string(month) + " / " + std::string(day < 10 ? "0" : "") + std::to_string(day);
    mvwprintw(win, start_y + 8, (max_x - (int)date_str.length()) / 2, "%s", date_str.c_str());
    std::string phase_str = "Fase: " + time;
    mvwprintw(win, start_y + 9, (max_x - (int)phase_str.length()) / 2, "%s", phase_str.c_str());
    wnoutrefresh(win);
}

void MainPage::draw_map(WINDOW* win, const std::vector<GraphNode>& nodes, const std::vector<GraphEdge>& edges, std::string_view selected_id, bool is_focused, std::string_view current_id) {
    if (!win) return;
    int max_y, max_x; getmaxyx(win, max_y, max_x);
    
    int title_color = is_focused ? 4 : 2;
    wbkgdset(win, COLOR_PAIR(2));
    werase(win);
    box(win, 0, 0);
    draw_title(win, "Peta Kota", w_right, title_color);

    if (nodes.empty()) {
        wnoutrefresh(win);
        return;
    }

    int max_layer = 0;
    int max_cols = 0;
    for (const auto& n : nodes) {
        if (n.ly > max_layer) max_layer = n.ly;
        if (n.lx > max_cols) max_cols = n.lx;
    }

    int cell_width = 12; // 12 chars wide per column
    int cell_height = 3; // 3 rows per layer

    int grid_w = (max_cols + 1) * cell_width;
    int grid_h = (max_layer + 1) * cell_height;

    int offset_x = (max_x - grid_w) / 2;
    if (offset_x < 2) offset_x = 2;
    int offset_y = (max_y - grid_h) / 2;
    if (offset_y < 1) offset_y = 1;

    std::unordered_map<std::string, std::pair<int, int>> screen_coords;
    for (const auto& n : nodes) {
        int sy = offset_y + (n.ly * cell_height);
        int sx = offset_x + (n.lx * cell_width);
        screen_coords[n.id] = {sy, sx};
    }

    wattron(win, COLOR_PAIR(2));
    for (const auto& e : edges) {
        auto it_u = screen_coords.find(e.u);
        auto it_v = screen_coords.find(e.v);
        if (it_u != screen_coords.end() && it_v != screen_coords.end()) {
            auto p1 = it_u->second;
            auto p2 = it_v->second;
            int y1 = p1.first, x1 = p1.second + 1;
            int y2 = p2.first, x2 = p2.second + 1;

            if (y1 == y2) { // East/West
                int min_x = std::min(x1, x2) + 2;
                int max_x_draw = std::max(x1, x2) - 2;
                for (int x = min_x; x <= max_x_draw; ++x) {
                    if (y1 >= 0 && y1 < max_y && x >= 0 && x < max_x) {
                        mvwprintw(win, y1, x, "-");
                    }
                }
            } else if (x1 == x2) { // North/South
                int min_y = std::min(y1, y2) + 1;
                int max_y_draw = std::max(y1, y2) - 1;
                for (int y = min_y; y <= max_y_draw; ++y) {
                    if (y >= 0 && y < max_y && x1 >= 0 && x1 < max_x) {
                        mvwprintw(win, y, x1, "|");
                    }
                }
            } else {
                int dx = std::abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
                int dy = std::abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
                int err = (dx > dy ? dx : -dy) / 2, e2;
                
                int cx = x1, cy = y1;
                while (true) {
                    if ((cx < x1 - 1 || cx > x1 + 1 || cy != y1) && 
                        (cx < x2 - 1 || cx > x2 + 1 || cy != y2)) {
                        char c = '.';
                        if (dx > dy * 2) c = '-';
                        else if (dy > dx * 2) c = '|';
                        else if ((sx > 0 && sy > 0) || (sx < 0 && sy < 0)) c = '\\';
                        else c = '/';
                        if (cy >= 0 && cy < max_y && cx >= 0 && cx < max_x) {
                            mvwaddch(win, cy, cx, c);
                        }
                    }
                    if (cx == x2 && cy == y2) break;
                    e2 = err;
                    if (e2 > -dx) { err -= dy; cx += sx; }
                    if (e2 < dy) { err += dx; cy += sy; }
                }
            }
        }
    }
    wattroff(win, COLOR_PAIR(2));

    for (const auto& n : nodes) {
        auto p = screen_coords[n.id];
        int sy = p.first;
        int sx = p.second;

        bool is_selected = (n.id == selected_id);
        bool is_current = (n.id == current_id);

        std::string marker;
        if (is_selected) marker = is_current ? "[@]" : "[*]";
        else marker = is_current ? "[O]" : "[ ]";

        if (sy >= 0 && sy < max_y && sx >= 0 && sx < max_x) {
            if (is_selected && is_focused) {
                wattron(win, A_REVERSE | COLOR_PAIR(4) | A_BOLD);
                mvwprintw(win, sy, sx, "%s", marker.c_str());
                wattroff(win, A_REVERSE | COLOR_PAIR(4) | A_BOLD);
            } else if (is_selected) {
                wattron(win, A_BOLD | COLOR_PAIR(4));
                mvwprintw(win, sy, sx, "%s", marker.c_str());
                wattroff(win, A_BOLD | COLOR_PAIR(4));
            } else if (is_current) {
                wattron(win, A_BOLD | COLOR_PAIR(3));
                mvwprintw(win, sy, sx, "%s", marker.c_str());
                wattroff(win, A_BOLD | COLOR_PAIR(3));
            } else {
                mvwprintw(win, sy, sx, "%s", marker.c_str());
            }
        }
    }
    wnoutrefresh(win);
}

void MainPage::draw_player_stats(WINDOW* win, int str, int cons, int agi, int intl, int wis, std::string affinity, int gold, const std::vector<std::string>& equipped_info) {
    if (!win) return;
    int max_y, max_x; getmaxyx(win, max_y, max_x);
    int start_y = std::max(1, (max_y - 8) / 2);
    mvwprintw(win, start_y,     2, "STR : %d", str);
    mvwprintw(win, start_y + 1, 2, "CON : %d", cons);
    mvwprintw(win, start_y + 2, 2, "AGI : %d", agi);
    mvwprintw(win, start_y + 3, 2, "INT : %d", intl);
    mvwprintw(win, start_y + 4, 2, "WIS : %d", wis);
    int equip_x = std::max(15, max_x / 2 - 2);
    for (size_t i = 0; i < equipped_info.size() && (int)(start_y + i) < max_y - 2; ++i) {
        mvwprintw(win, start_y + i, equip_x, "%s", equipped_info[i].c_str());
    }
    wattron(win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(win, max_y - 2, 2, "Afin: %s", affinity.c_str());
    wattroff(win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(win, max_y - 2, max_x - 12, "G: %d", gold);
    wnoutrefresh(win);
}

void draw_bar(WINDOW* win, int y, int x, int width, int current, int max, int color_pair) {
    float fill = (max > 0) ? (float) current / max : 0;
    int filled_width = width * fill;
    mvwprintw(win, y, x, "[");
    wattron(win, COLOR_PAIR(color_pair) | A_REVERSE);
    for (int i = 0; i < filled_width; ++i) waddch(win, ' ');
    wattroff(win, COLOR_PAIR(color_pair) | A_REVERSE);
    for (int i = 0; i < width - filled_width; ++i) waddch(win, '.');
    waddch(win, ']');
    wprintw(win, " %d/%d", current, max);
}

void MainPage::draw_vitals(WINDOW* win, int hp, int max_hp, int mp, int max_mp) {
    if (!win) return;
    int max_y, max_x; getmaxyx(win, max_y, max_x);
    int bar_width = std::max(5, max_x - 20);
    int start_y = std::max(1, (max_y - 2) / 2);
    mvwprintw(win, start_y, 2, "HP "); draw_bar(win, start_y, 5, bar_width, hp, max_hp, 5);
    mvwprintw(win, start_y + 1, 2, "MP "); draw_bar(win, start_y + 1, 5, bar_width, mp, max_mp, 3);
    wnoutrefresh(win);
}

void MainPage::draw_inventory(WINDOW* win, const std::vector<std::string>& item_names) {
    if (!win) return;
    int max_y, max_x; getmaxyx(win, max_y, max_x);
    for (size_t i = 0; i < item_names.size() && (int)i < max_y - 2; ++i) mvwprintw(win, 1 + i, 2, "- %s", item_names[i].c_str());
    wnoutrefresh(win);
}

void MainPage::draw_tasks(WINDOW* win, const std::vector<std::string>& tasks) {
    if (!win) return;
    int max_y, max_x; getmaxyx(win, max_y, max_x);
    for (size_t i = 0; i < tasks.size() && (int)i < max_y - 2; ++i) mvwprintw(win, 1 + i, 2, "%s", tasks[i].c_str());
    wnoutrefresh(win);
}