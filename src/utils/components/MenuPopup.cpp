#include "MenuPopup.hpp"
#include <ncurses.h>

namespace Utils {

MenuPopup::MenuPopup(const std::string& title, const std::vector<std::string>& opts, std::function<void(int)> callback)
    : Popup(""), header(title), options(opts), on_select(std::move(callback)) {
    state = PopupState::WAITING; // Skip animation
    target_w = std::min(50, COLS - 2);
    target_h = std::min((int)(4 + options.size()), LINES - 2);
    y = (LINES - target_h) / 2;
    x = (COLS - target_w) / 2;
    wresize(win, target_h, target_w);
    mvwin(win, y, x);
}

void MenuPopup::update() {
    Popup::update();
}

bool MenuPopup::handle_input(int ch) {
    if (state == PopupState::DISMISSED) return false;
    if (ch == KEY_RESIZE) {
        resize();
        return true;
    }
    
    if (ch == KEY_UP || ch == 'w') {
        selected_index--;
        if (selected_index < 0) selected_index = options.size() - 1;
        return true;
    } else if (ch == KEY_DOWN || ch == 's') {
        selected_index++;
        if (selected_index >= (int)options.size()) selected_index = 0;
        return true;
    } else if (ch == '\n' || ch == KEY_ENTER) {
        state = PopupState::DISMISSED;
        if (on_select) on_select(selected_index);
        return true;
    } else if (ch == 27) { // ESC
        state = PopupState::DISMISSED;
        if (on_select) on_select(-1);
        return true;
    }
    return false;
}

void MenuPopup::resize() {
    target_w = std::min(50, COLS - 2);
    target_h = std::min((int)(4 + options.size()), LINES - 2);
    y = (LINES - target_h) / 2;
    x = (COLS - target_w) / 2;
    wresize(win, target_h, target_w);
    mvwin(win, y, x);
}

void MenuPopup::render() {
    if (state == PopupState::DISMISSED) return;
    
    wresize(win, target_h, target_w);
    mvwin(win, y, x);
    wbkgdset(win, COLOR_PAIR(1));
    werase(win);
    box(win, 0, 0);
    
    int hdr_w = target_w - 4;
    std::string h = header;
    if ((int)h.length() > hdr_w) h = h.substr(0, hdr_w - 1) + "~";
    mvwprintw(win, 1, 2, "%s", h.c_str());
    
    for (size_t i = 0; i < options.size(); ++i) {
        std::string opt = options[i];
        int opt_max = target_w - 6;
        if ((int)opt.length() > opt_max) opt = opt.substr(0, opt_max - 1) + "~";
        if ((int)i == selected_index) {
            wattron(win, A_REVERSE | A_BOLD | COLOR_PAIR(4));
            mvwprintw(win, 3 + i, 4, "> %s", opt.c_str());
            wattroff(win, A_REVERSE | A_BOLD | COLOR_PAIR(4));
        } else {
            mvwprintw(win, 3 + i, 4, "  %s", opt.c_str());
        }
    }
    
    wnoutrefresh(win);
}

} // namespace Utils
