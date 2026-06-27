#include "Popup.hpp"
#include <sstream>
#include <vector>
#include <algorithm>

namespace Utils {

static void rewrap_text(const std::string& text, int max_width, std::vector<std::string>& out_lines, int& out_longest) {
    out_lines.clear();
    out_longest = 0;
    std::stringstream ss(text);
    std::string word;
    while (ss >> word) {
        while ((int)word.length() > max_width) {
            out_lines.push_back(word.substr(0, max_width));
            out_longest = max_width;
            word = word.substr(max_width);
        }
        if (out_lines.empty() || out_lines.back().length() + 1 + word.length() > (size_t)max_width) {
            out_lines.push_back(word);
            out_longest = std::max(out_longest, (int)word.length());
        } else {
            out_lines.back() += " " + word;
            out_longest = std::max(out_longest, (int)out_lines.back().length());
        }
    }
    if (out_lines.empty()) {
        out_lines.push_back("");
    }
}

Popup::Popup(const std::string &text) : original_text(text) {
    int max_width = std::min(50, COLS - 8);
    if (max_width < 10) max_width = 10;
    
    int longest_line = 0;
    rewrap_text(text, max_width, wrapped_lines, longest_line);

    target_h = wrapped_lines.size() + 4;
    target_w = longest_line + 6;
    if (target_w < 20) target_w = 20;
    if (target_w > COLS) target_w = COLS;

    y = (LINES - target_h) / 2;
    x = (COLS - target_w) / 2;
    
    win = newwin(2, 4, y + (target_h/2), x + (target_w/2));
    keypad(win, TRUE);
}

Popup::~Popup() { delwin(win); }

void Popup::resize() {
    int max_width = std::min(50, COLS - 8);
    if (max_width < 10) max_width = 10;
    
    int longest_line = 0;
    rewrap_text(original_text, max_width, wrapped_lines, longest_line);
    
    target_h = wrapped_lines.size() + 4;
    target_w = longest_line + 6;
    if (target_w < 20) target_w = 20;
    if (target_w > COLS) target_w = COLS;
    
    y = (LINES - target_h) / 2;
    x = (COLS - target_w) / 2;
    wresize(win, target_h, target_w);
    mvwin(win, y, x);
}

void Popup::update() {
    if (state == PopupState::ANIMATING) {
        anim_step += 2;
        if (anim_step >= target_h && anim_step * 2 >= target_w) {
            state = PopupState::TYPING;
        }
    } else if (state == PopupState::TYPING) {
        if (!typing_started) {
            if (on_type_start) on_type_start();
            typing_started = true;
        }

        if (type_line < (int)wrapped_lines.size()) {
            type_char += 2;
            if (type_char >= (int)wrapped_lines[type_line].length()) {
                type_line++;
                type_char = 0;
                if (type_line >= (int)wrapped_lines.size()) {
                    state = PopupState::WAITING;
                    if (on_type_stop) on_type_stop();
                }
            }
        } else {
            state = PopupState::WAITING;
            if (on_type_stop) on_type_stop();
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
        if (on_type_stop) on_type_stop();
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
    } else if (state == PopupState::TYPING) {
        box(win, 0, 0);
        int start_y = (target_h - wrapped_lines.size()) / 2;
        for (int i = 0; i < (int)wrapped_lines.size() && i <= type_line; ++i) {
            const auto& l = wrapped_lines[i];
            int sx = (target_w - (int)l.length()) / 2;
            if (i < type_line) {
                mvwprintw(win, start_y + i, sx, "%s", l.c_str());
            } else {
                mvwprintw(win, start_y + i, sx, "%.*s", type_char, l.c_str());
            }
        }
        wnoutrefresh(win);
    } else if (state == PopupState::WAITING) {
        werase(win);
        box(win, 0, 0);
        int start_y = (target_h - wrapped_lines.size()) / 2;
        for (int i = 0; i < (int)wrapped_lines.size(); ++i) {
            const auto& l = wrapped_lines[i];
            mvwprintw(win, start_y + i, (target_w - (int)l.length()) / 2, "%s", l.c_str());
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
    
    for (size_t li = 0; li < wrapped_lines.size(); ++li) {
        const auto& l = wrapped_lines[li];
        int start_x = (target_w - (int)l.length()) / 2;
        for (char c : l) {
            mvwaddch(win, start_y, start_x++, c);
            wrefresh(win);
            
            // Skip typing if key pressed
            int ch = getch();
            if (ch != ERR) {
                if (ch == KEY_RESIZE) {
                    resize();
                    werase(win);
                    box(win, 0, 0);
                }
                // Instantly draw ALL remaining text (current + subsequent lines)
                for (size_t ri = li; ri < wrapped_lines.size(); ++ri) {
                    int ry = start_y + ri - li;
                    mvwprintw(win, ry, (target_w - (int)wrapped_lines[ri].length()) / 2, "%s", wrapped_lines[ri].c_str());
                }
                wrefresh(win);
                goto done_typing;
            }
            napms(20);
        }
        start_y++;
    }
    
done_typing:
    // Anti-skip: Flush any keys pressed during animation and add a tiny delay
    napms(150);
    flushinp();

    // Wait for Space or Enter
    while (true) {
        int ch = getch();
        if (ch == '\n' || ch == ' ') break;
        if (ch == KEY_RESIZE) {
            resize();
            werase(win);
            box(win, 0, 0);
            int ry = (target_h - wrapped_lines.size()) / 2;
            for (const auto& ll : wrapped_lines) {
                mvwprintw(win, ry, (target_w - (int)ll.length()) / 2, "%s", ll.c_str());
                ry++;
            }
            wrefresh(win);
        }
        wrefresh(win);
        napms(10);
    }
    werase(win); wrefresh(win);
}
} // namespace Utils
