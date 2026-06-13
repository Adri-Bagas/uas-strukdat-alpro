#include "MainPage.hpp"
#include <cstring>
#include <ncurses.h>
#include <sstream>

const char* MainPage::big_digits[10][5] = {
    {"███", "█ █", "█ █", "█ █", "███"}, // 0
    {" ██", "  █", "  █", "  █", "███"}, // 1
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

    // Guard against terminal being too small
    if (max_y < 20 || max_x < 70)
        return;

    int left_w = max_x * 0.3;
    int right_w = max_x - left_w;

    int top_h = max_y * 0.4;
    int bot_h = max_y - top_h;

    int col1_w = right_w * 0.25;
    int col2_w = right_w * 0.45;
    int col3_w = right_w - col1_w - col2_w;

    int box_h = bot_h / 3;

    w_left = left_w;
    w_right = right_w;
    w_col1 = col1_w;
    w_col2 = col2_w;
    w_col3 = col3_w;

    win_dialog = newwin(max_y, left_w, 0, 0);
    win_thought = newwin(top_h, right_w, 0, left_w);

    win_stat = newwin(box_h, col1_w, top_h, left_w);
    win_hp = newwin(box_h, col1_w, top_h + box_h, left_w);
    win_cal = newwin(bot_h - (box_h * 2), col1_w, top_h + (box_h * 2), left_w);

    win_menu = newwin(bot_h, col2_w, top_h, left_w + col1_w);
    win_task = newwin(bot_h, col3_w, top_h, left_w + col1_w + col2_w);
}

void MainPage::destroy_windows() {
    if (win_thought)
        delwin(win_thought);
    if (win_dialog)
        delwin(win_dialog);
    if (win_stat)
        delwin(win_stat);
    if (win_hp)
        delwin(win_hp);
    if (win_cal)
        delwin(win_cal);
    if (win_menu)
        delwin(win_menu);
    if (win_task)
        delwin(win_task);

    win_thought = win_dialog = win_stat = win_hp = win_cal = win_menu = win_task = nullptr;
}

MainPage::MainPage() {
    win_thought = win_dialog = win_stat = win_hp = win_cal = win_menu = win_task = nullptr;
    create_windows();
}

void MainPage::draw_title(WINDOW *win, const char *title, int width, int color_pair_id = 2) {
    int len = strlen(title);
    int x = 2;

    // Enable attributes
    wattron(win, A_BOLD | A_REVERSE | COLOR_PAIR(color_pair_id));

    // Print with padding spaces for wider visual block
    mvwprintw(win, 0, x - 1, " %s ", title);

    // Disable attributes
    wattroff(win, A_BOLD | A_REVERSE | COLOR_PAIR(color_pair_id));
}

void MainPage::draw() {

    if (!win_thought) {
        // Fallback if resize hasn't happened yet
        mvprintw(LINES / 2, (COLS - 24) / 2, "Please resize terminal.");
        refresh(); 
        return;
    }

    // Sequence MUST BE:
    // 1. wbkgdset()   : Set future background color
    // 2. werase()     : Wipe window clean using that color
    // 3. box()        : Draw borders on top
    // 4. draw_title() : Draw title on top
    // 5. wnoutrefresh(): Send to virtual screen (NO FLICKER)

    wbkgdset(win_dialog, COLOR_PAIR(2));
    werase(win_dialog);
    box(win_dialog, 0, 0);
    draw_title(win_dialog, "In World Dialog", w_left);
    wnoutrefresh(win_dialog);

    wbkgdset(win_thought, COLOR_PAIR(2));
    werase(win_thought);
    box(win_thought, 0, 0);
    draw_title(win_thought, "Inner Monolog", w_right);
    wnoutrefresh(win_thought);

    wbkgdset(win_stat, COLOR_PAIR(2));
    werase(win_stat);
    box(win_stat, 0, 0);
    draw_title(win_stat, "Stat Block", w_col1);
    wnoutrefresh(win_stat);

    wbkgdset(win_hp, COLOR_PAIR(2));
    werase(win_hp);
    box(win_hp, 0, 0);
    draw_title(win_hp, "HP/MP/Stamina", w_col1);
    wnoutrefresh(win_hp);

    wbkgdset(win_cal, COLOR_PAIR(2));
    werase(win_cal);
    box(win_cal, 0, 0);
    draw_title(win_cal, "Calendar", w_col1);
    wnoutrefresh(win_cal);

    wbkgdset(win_menu, COLOR_PAIR(2));
    werase(win_menu);
    box(win_menu, 0, 0);
    draw_title(win_menu, "Menu / Inventory", w_col2);
    wnoutrefresh(win_menu);

    wbkgdset(win_task, COLOR_PAIR(2));
    werase(win_task);
    box(win_task, 0, 0);
    draw_title(win_task, "Task List", w_col3);
    wnoutrefresh(win_task);
}

void MainPage::resize() {
    destroy_windows();
    create_windows();
    draw();
}

MainPage::~MainPage() {
    destroy_windows();
}

void MainPage::type_new_text(WINDOW* win, const char* title, int width, 
                             const std::vector<DialogNode>& history, 
                             const DialogNode& new_text) {
    if (!win) return;
    
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    int max_width = max_x - 4;

    // 1. Setup fresh window
    wbkgdset(win, COLOR_PAIR(4) | A_BOLD);
    werase(win);
    box(win, 0, 0);
    draw_title(win, title, width, 4);

    // 2. Pre-wrap the NEW text so we know how many lines it needs
    std::vector<std::string> new_lines;
    std::stringstream ss_new(new_text.value);
    std::string word, line = "";
    while (ss_new >> word) {
        if (line.empty()) { line = word; } 
        else if (line.length() + 1 + word.length() <= max_width) { line += " " + word; } 
        else { new_lines.push_back(line); line = word; }
    }
    if (!line.empty()) { new_lines.push_back(line); }

    // Calculate entire incoming block height first
    int required_lines = new_lines.size();
    bool has_name = (new_text.npc_name != "");
    int total_new_block_lines = required_lines + (has_name ? 1 : 0);

    // 3. Draw history, shifted up to make room for the name AND new multi-line text
    if (!history.empty()) {
        std::vector<std::string> wrapped;
        
        // Quick wrap for history
        for (const auto& entry : history) {
            std::stringstream ss(entry.value);
            std::string w, l = "";
            while (ss >> w) {
                if (l.empty()) { l = w; } 
                else if (l.length() + 1 + w.length() <= max_width) { l += " " + w; } 
                else { wrapped.push_back(l); l = w; }
            }
            if (!l.empty()) { wrapped.push_back(l); }
        }

        // Calculate how many history lines fit below top border and above new block
        int available_history_lines = (max_y - 2) - total_new_block_lines;
        if (available_history_lines > 0) {
            int draw_count = std::min((int)wrapped.size(), available_history_lines); 
            int start_idx = wrapped.size() - draw_count;
            
            // Push history up by total incoming content size
            int start_y = (max_y - 1) - total_new_block_lines - draw_count; 

            for(int i = 0; i < draw_count; i++) {
                mvwprintw(win, start_y + i, 2, "%s", wrapped[start_idx + i].c_str());
            }
        }
    }

    // 4. Draw content layout blocks
    int block_start_y = (max_y - 1) - total_new_block_lines;

    // Print NPC name at absolute top of current dialog chunk
    if (has_name) {
        mvwprintw(win, block_start_y, 2, "%s", new_text.npc_name.c_str());
    }

    // Text animation starts directly below name block
    int anim_start_y = block_start_y + (has_name ? 1 : 0);
    bool skipped = false;

    for (size_t i = 0; i < new_lines.size(); ++i) {
        int current_y = anim_start_y + i;
        
        // Safeguard against drawing over bottom border if text is massive
        if (current_y >= max_y - 1) break; 
        
        int current_x = 2;

        if (skipped) {
            // If skipped, instantly print remaining lines
            mvwprintw(win, current_y, 2, "%s", new_lines[i].c_str());
            continue;
        }

        for (char c : new_lines[i]) {
            mvwaddch(win, current_y, current_x++, c);
            wnoutrefresh(win); 
            doupdate();        

            // Fast forward skip
            int ch = getch();
            if (ch != ERR) {
                skipped = true;
                // REMOVED ungetch(ch); so skip key consumed
                
                // Instantly finish the current line
                mvwprintw(win, current_y, 2, "%s", new_lines[i].c_str());
                wnoutrefresh(win);
                doupdate();
                break; // Break character loop, move to fast-forward next lines
            }
            
            napms(30); // Typing speed
        }
    }

    // NEW: Wait for Space or Enter to continue
    while (true) {
        int ch = getch();
        if (ch == '\n' || ch == ' ') {
            break;
        }
        napms(10);
    }
}

void MainPage::render_history(WINDOW* win, const std::vector<DialogNode>& history) {
    if (!win || history.empty()) return;

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    int max_width = max_x - 4;
    int available_lines = max_y - 2; 

    // 1. Word wrap all history
    std::vector<std::string> wrapped;
    for (const auto& entry : history) {

        if (!entry.npc_name.empty()) {
            wrapped.push_back(entry.npc_name);
        }

        std::stringstream ss(entry.value);
        std::string word, line = "";
        while (ss >> word) {
            if (line.empty()) { line = word; } 
            else if (line.length() + 1 + word.length() <= max_width) { line += " " + word; } 
            else { wrapped.push_back(line); line = word; }
        }
        if (!line.empty()) { wrapped.push_back(line); }
    }

    // 2. Chat-style Math: Calculate lines to draw and pin them to the bottom
    int num_lines = wrapped.size();
    int draw_count = std::min(num_lines, available_lines);
    int start_idx = num_lines - draw_count;
    
    // Calculate starting Y so the last line sits exactly at (max_y - 2)
    int start_y = (max_y - 2) - draw_count; 

    // 3. Draw text
    for (int i = 0; i < draw_count; ++i) {
        mvwprintw(win, start_y + i, 2, "%s", wrapped[start_idx + i].c_str());
    }
    
    // 4. CRITICAL: Stage changes to virtual screen
    wnoutrefresh(win); 
}

void MainPage::draw_calendar(WINDOW* win, int days_left, int month, int day, std::string time) {
    if (!win) return;

    // 1. Get dynamic window size
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    // 2. Calculate Y offset
    int total_height = 10;
    int start_y = (max_y - total_height) / 2;
    if (start_y < 1) start_y = 1; 

    // 3. Draw Big Digits (Centered X)
    int tens = days_left / 10;
    int ones = days_left % 10;

    int digits_width = (tens > 0) ? 7 : 3; 
    int digits_start_x = (max_x - digits_width) / 2;

    wattron(win, COLOR_PAIR(2) | A_BOLD);
    for (int row = 0; row < 5; ++row) {
        if (tens > 0) {
            mvwprintw(win, start_y + row, digits_start_x, "%s", big_digits[tens][row]);
        }
        int ones_offset = (tens > 0) ? 4 : 0;
        mvwprintw(win, start_y + row, digits_start_x + ones_offset, "%s", big_digits[ones][row]);
    }
    wattroff(win, COLOR_PAIR(2) | A_BOLD);

    // 4. Draw Text (Centered individually on X using std::string)
    std::string title = "DAYS REMAINING";
    int title_x = (max_x - title.length()) / 2;
    mvwprintw(win, start_y + 6, title_x, "%s", title.c_str());

    // Format Date string: Add leading zeros manually
    std::string str_month = (month < 10 ? "0" : "") + std::to_string(month);
    std::string str_day = (day < 10 ? "0" : "") + std::to_string(day);
    
    std::string date_str = "Date : " + str_month + " / " + str_day;
    int date_x = (max_x - date_str.length()) / 2;
    mvwprintw(win, start_y + 8, date_x, "%s", date_str.c_str());

    // Format Phase string
    std::string phase_str = "Phase: " + time;
    int phase_x = (max_x - phase_str.length()) / 2;
    mvwprintw(win, start_y + 9, phase_x, "%s", phase_str.c_str());

    wnoutrefresh(win);
}
void MainPage::draw_player_stats(WINDOW* win, int str, int cons, int agi, int intl, int wis, std::string affinity, int gold) {
    if (!win) return;
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    int start_y = (max_y - 8) / 2;
    if (start_y < 1) start_y = 1;

    mvwprintw(win, start_y,     2, "STR : %d", str);
    mvwprintw(win, start_y + 1, 2, "CON : %d", cons);
    mvwprintw(win, start_y + 2, 2, "AGI : %d", agi);
    mvwprintw(win, start_y + 3, 2, "INT : %d", intl);
    mvwprintw(win, start_y + 4, 2, "WIS : %d", wis);
    
    wattron(win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(win, start_y + 6, 2, "Affinity: %-8s", affinity.c_str());
    wattroff(win, COLOR_PAIR(4) | A_BOLD);

    mvwhline(win, start_y - 7, 1, ACS_HLINE, max_x - 2);
    mvwprintw(win, start_y + 8, 2, "Gold: %d G", gold);
    // mvwhline(win, 2, 1, ACS_HLINE, max_x - 2);

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

void MainPage::draw_vitals(WINDOW* win, int hp, int max_hp, int mp, int max_mp, int stamina, int max_stamina) {
    if (!win) return;
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    int bar_width = max_x - 20;
    if (bar_width < 5) bar_width = 5;

    int start_y = (max_y - 8) / 2;
    if (start_y < 1) start_y = 1;

    mvwprintw(win, start_y,     2, "HP "); draw_bar(win, start_y,     5, bar_width, 50, max_hp, 5); // Red
    mvwprintw(win, start_y + 1, 2, "MP "); draw_bar(win, start_y + 1, 5, bar_width, 25, max_mp, 3); // Blue
    mvwprintw(win, start_y + 2, 2, "ST "); draw_bar(win, start_y + 2, 5, bar_width, 30, max_stamina, 4); // Yellow

    wnoutrefresh(win);
}

void MainPage::draw_inventory(WINDOW* win, const std::vector<std::string>& item_names) {
    if (!win) return;
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    for (size_t i = 0; i < item_names.size() && (int)i < max_y - 4; ++i) {
        mvwprintw(win, 3 + i, 2, "- %s", item_names[i].c_str());
    }

    wnoutrefresh(win);
}

void MainPage::draw_tasks(WINDOW* win, const std::vector<std::string>& tasks) {
    if (!win) return;
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    for (size_t i = 0; i < tasks.size() && (int)i < max_y - 2; ++i) {
        mvwprintw(win, 1 + i, 2, "%s", tasks[i].c_str());
    }

    wnoutrefresh(win);
}

void MainPage::draw_choices(WINDOW* win, const std::vector<DialogChoice>& choices, int selected_idx) {
    if (!win) return;
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    
    wbkgdset(win, COLOR_PAIR(2));
    werase(win);
    box(win, 0, 0);
    draw_title(win, "Make Your Choice", max_x, 4);

    int start_y = 2;
    for (size_t i = 0; i < choices.size(); ++i) {
        if (start_y + (int)i >= max_y - 2) break;
        if ((int)i == selected_idx) {
            wattron(win, A_REVERSE | COLOR_PAIR(4) | A_BOLD);
            mvwprintw(win, start_y + (int)i, 2, "> %s", choices[i].text.c_str());
            wattroff(win, A_REVERSE | COLOR_PAIR(4) | A_BOLD);
        } else {
            wattron(win, COLOR_PAIR(2));
            mvwprintw(win, start_y + (int)i, 2, "  %s", choices[i].text.c_str());
            wattroff(win, COLOR_PAIR(2));
        }
    }
    wnoutrefresh(win);
}
