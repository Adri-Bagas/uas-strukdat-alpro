#include "LogPopup.hpp"
#include <algorithm>

namespace Utils {

LogPopup::LogPopup(LogManager& manager, int w, int h)
    : Popup(""), log_manager(manager), current_top(manager.get_logs().begin()), display_lines(h - 4) {
    target_w = w;
    target_h = h;
    
    delwin(win); // Destroy base class tiny window
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    y = (max_y - target_h) / 2;
    x = (max_x - target_w) / 2;
    win = newwin(target_h, target_w, y, x);
    keypad(win, TRUE);
    
    // Set to WAITING immediately so it doesn't animate/type
    state = PopupState::WAITING;
    anim_step = target_w; // Skip animation
    
    // Initialize current_top to the first element if not empty
    if (!log_manager.get_logs().empty()) {
        current_top = log_manager.get_logs().begin();
    }
}

void LogPopup::update() {
    // Override to skip typing animation
}

bool LogPopup::handle_input(int ch) {
    if (ch == KEY_RESIZE) {
        resize();
        return true;
    }
    
    if (ch == 'q' || ch == 27 || ch == '\n' || ch == ' ') { // Esc, Enter, Space, Q
        state = PopupState::DISMISSED;
        return true;
    }
    
    if (log_manager.get_logs().empty()) return true;

    if (ch == KEY_UP || ch == 'w') {
        // Move current_top backwards (towards older logs if new logs are at the end)
        // Wait, begin() is the oldest log, end() is the newest.
        // If we press UP, we probably want to see older logs.
        if (current_top != log_manager.get_logs().begin()) {
            --current_top;
        }
    } else if (ch == KEY_DOWN || ch == 's') {
        // Move current_top forwards (towards newer logs)
        // Check if we have room to scroll down
        auto temp = current_top;
        int count = 0;
        while (temp != log_manager.get_logs().end() && count <= display_lines) {
            ++temp;
            count++;
        }
        
        // If we can still see items past current_top + display_lines, we can scroll down
        if (temp != log_manager.get_logs().end() || count > display_lines) {
            ++current_top;
        }
    }
    
    return true;
}

void LogPopup::render() {
    if (state == PopupState::DISMISSED) return;

    if (!win) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        y = (max_y - target_h) / 2;
        x = (max_x - target_w) / 2;
        win = newwin(target_h, target_w, y, x);
    }

    werase(win);
    wbkgdset(win, COLOR_PAIR(1));
    box(win, 0, 0);

    // Draw Title
    wattron(win, A_BOLD | A_REVERSE | COLOR_PAIR(1));
    mvwprintw(win, 0, (target_w - 13) / 2, " Message Log ");
    wattroff(win, A_BOLD | A_REVERSE | COLOR_PAIR(1));

    if (log_manager.get_logs().empty()) {
        mvwprintw(win, target_h / 2, (target_w - 18) / 2, "No messages yet...");
    } else {
        int line = 2;
        auto it = current_top;
        while (it != log_manager.get_logs().end() && line < target_h - 1) {
            const auto& entry = *it;
            // Print timestamp
            wattron(win, COLOR_PAIR(3)); // Assuming 3 is a nice color (cyan/yellow)
            mvwprintw(win, line, 2, "[%s]", entry.timestamp.c_str());
            wattroff(win, COLOR_PAIR(3));
            
            // Print message
            mvwprintw(win, line, 2 + entry.timestamp.length() + 3, "%s", entry.message.c_str());
            
            ++it;
            ++line;
        }
        
        // Draw scroll indicators
        if (current_top != log_manager.get_logs().begin()) {
            mvwprintw(win, 1, target_w / 2, "^");
        }
        
        if (it != log_manager.get_logs().end()) {
            mvwprintw(win, target_h - 2, target_w / 2, "v");
        }
    }

    wrefresh(win);
}

} // namespace Utils
