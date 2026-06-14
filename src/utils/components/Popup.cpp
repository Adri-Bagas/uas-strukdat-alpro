#include "Popup.hpp"
#include <sstream>
#include <vector>
#include <algorithm>

Popup::Popup(const std::string &text) {
    int max_width = 50;
    
    // 1. Word wrap logic
    std::stringstream ss(text);
    std::string word, current_line = "";
    int longest_line = 0;

    while (ss >> word) {
        if (current_line.empty()) {
            current_line = word;
        } else if (current_line.length() + 1 + word.length() <= (size_t)max_width) {
            current_line += " " + word;
        } else {
            wrapped_lines.push_back(current_line);
            longest_line = std::max(longest_line, (int)current_line.length());
            current_line = word;
        }
    }
    if (!current_line.empty()) {
        wrapped_lines.push_back(current_line);
        longest_line = std::max(longest_line, (int)current_line.length());
    }

    // 2. Calculate dynamic dimensions
    target_h = wrapped_lines.size() + 4; // Top/bottom border + padding
    target_w = longest_line + 6;         // Left/right border + padding
    if (target_w < 20) target_w = 20;    // Minimum width

    y = (LINES - target_h) / 2;
    x = (COLS - target_w) / 2;
    
    win = newwin(2, 4, y + (target_h/2), x + (target_w/2));
    keypad(win, TRUE);
}

Popup::~Popup() { delwin(win); }

void Popup::animate() {
    for (int i = 1; i <= target_h || i * 2 <= target_w; ++i) {
        int h = (i < target_h) ? i : target_h;
        int w = (i * 2 < target_w) ? i * 2 : target_w;
        wresize(win, h, w);
        mvwin(win, y + (target_h - h) / 2, x + (target_w - w) / 2);
        werase(win); box(win, 0, 0); wrefresh(win);
        napms(15);
    }
}

void Popup::type_text() {
    int start_y = (target_h - wrapped_lines.size()) / 2;
    
    for (const auto& l : wrapped_lines) {
        int start_x = (target_w - (int)l.length()) / 2;
        for (char c : l) {
            mvwaddch(win, start_y, start_x++, c);
            wrefresh(win);
            
            // Skip typing if key pressed
            int ch = getch();
            if (ch != ERR) {
                if (ch == KEY_RESIZE) {
                    resizeterm(0, 0);
                    y = (LINES - target_h) / 2;
                    x = (COLS - target_w) / 2;
                    mvwin(win, y, x);
                    refresh();
                }
                // Instantly draw remaining characters in line
                mvwprintw(win, start_y, (target_w - (int)l.length()) / 2, "%s", l.c_str());
                wrefresh(win);
                break;
            }
            napms(20);
        }
        start_y++;
    }
    
    // Anti-skip: Flush any keys pressed during animation and add a tiny delay
    napms(150);
    flushinp();

    // Wait for Space or Enter
    while (true) {
        int ch = getch();
        if (ch == '\n' || ch == ' ') break;
        if (ch == KEY_RESIZE) {
            resizeterm(0, 0);
            y = (LINES - target_h) / 2;
            x = (COLS - target_w) / 2;
            mvwin(win, y, x);
            refresh();
            wrefresh(win);
        }
        napms(10);
    }
    werase(win); wrefresh(win);
}