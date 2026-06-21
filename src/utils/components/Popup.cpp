#include "Popup.hpp"
#include <sstream>
#include <vector>
#include <algorithm>

namespace Utils {

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

void Popup::resize() {
    y = (LINES - target_h) / 2;
    x = (COLS - target_w) / 2;
    mvwin(win, y, x);
}

void Popup::update() {
    if (state == PopupState::ANIMATING) {
        anim_step += 2;
        if (anim_step >= target_h && anim_step * 2 >= target_w) {
            state = PopupState::TYPING;
        }
    } else if (state == PopupState::TYPING) {
        if (type_line < (int)wrapped_lines.size()) {
            type_char += 2;
            if (type_char >= (int)wrapped_lines[type_line].length()) {
                type_line++;
                type_char = 0;
                if (type_line >= (int)wrapped_lines.size()) {
                    state = PopupState::WAITING;
                }
            }
        } else {
            state = PopupState::WAITING;
        }
    }
}

bool Popup::handle_input(int ch) {
    if (ch == KEY_RESIZE) {
        resize();
        return true;
    }
    
    if (state == PopupState::TYPING && ch != ERR) {
        state = PopupState::WAITING;
        type_line = wrapped_lines.size();
        return true;
    }
    
    if (state == PopupState::WAITING && (ch == '\n' || ch == ' ')) {
        state = PopupState::DISMISSED;
        return true;
    }
    
    return false;
}

void Popup::render() {
    if (state == PopupState::DISMISSED) return;

    if (state == PopupState::ANIMATING) {
        int h = (anim_step < target_h) ? anim_step : target_h;
        int w = (anim_step * 2 < target_w) ? anim_step * 2 : target_w;
        wresize(win, h, w);
        mvwin(win, y + (target_h - h) / 2, x + (target_w - w) / 2);
        werase(win);
        box(win, 0, 0);
        wnoutrefresh(win);
    } else {
        wresize(win, target_h, target_w);
        mvwin(win, y, x);
        werase(win);
        box(win, 0, 0);
        
        int start_y = (target_h - wrapped_lines.size()) / 2;
        
        for (int i = 0; i < (int)wrapped_lines.size(); ++i) {
            if (i > type_line) break;
            
            const auto& l = wrapped_lines[i];
            int start_x = (target_w - (int)l.length()) / 2;
            
            if (i == type_line) {
                if (type_char > 0) {
                    mvwprintw(win, start_y + i, start_x, "%.*s", type_char, l.c_str());
                }
            } else {
                mvwprintw(win, start_y + i, start_x, "%s", l.c_str());
            }
        }
        wnoutrefresh(win);
    }
}

void Popup::animate_blocking() {
    for (int i = 1; i <= target_h || i * 2 <= target_w; ++i) {
        int h = (i < target_h) ? i : target_h;
        int w = (i * 2 < target_w) ? i * 2 : target_w;
        wresize(win, h, w);
        mvwin(win, y + (target_h - h) / 2, x + (target_w - w) / 2);
        werase(win); box(win, 0, 0); wrefresh(win);
        napms(15);
    }
}

void Popup::type_text_blocking() {
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
                    resize();
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
            resize();
            refresh();
        }
        wrefresh(win);
        napms(10);
    }
    werase(win); wrefresh(win);
}
} // namespace Utils
