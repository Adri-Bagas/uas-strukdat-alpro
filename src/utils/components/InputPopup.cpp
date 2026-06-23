#include "InputPopup.hpp"
#include <ncurses.h>

namespace Utils {

InputPopup::InputPopup(const std::string& prompt_text, std::function<void(const std::string&)> submit_cb)
    : Popup(""), prompt(prompt_text), on_submit(std::move(submit_cb)) {
    state = PopupState::WAITING; // Skip animation for input
    target_w = 40;
    target_h = 5;
    resize();
}

void InputPopup::update() {
    Popup::update();
}

bool InputPopup::handle_input(int ch) {
    if (state == PopupState::DISMISSED) return false;
    if (ch == '\n' || ch == KEY_ENTER) {
        if (!input_text.empty()) {
            state = PopupState::DISMISSED;
            if (on_submit) on_submit(input_text);
        }
        return true;
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
        if (!input_text.empty()) input_text.pop_back();
        return true;
    } else if (ch >= 32 && ch <= 126 && input_text.length() < 20) {
        input_text += static_cast<char>(ch);
        return true;
    } else if (ch == 27) { // ESC
        state = PopupState::DISMISSED;
        return true;
    }
    return false;
}

void InputPopup::render() {
    if (state == PopupState::DISMISSED) return;
    
    wresize(win, target_h, target_w);
    mvwin(win, y, x);
    wbkgdset(win, COLOR_PAIR(1));
    werase(win);
    box(win, 0, 0);
    
    mvwprintw(win, 1, 2, "%s", prompt.c_str());
    mvwprintw(win, 3, 2, "> %s_", input_text.c_str());
    
    wnoutrefresh(win);
}

} // namespace Utils
