#include "LogPopup.hpp"


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

void LogPopup::resize() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    if (target_w > max_x - 2) target_w = max_x - 2;
    if (target_h > max_y - 2) target_h = max_y - 2;
    y = std::max(0, (max_y - target_h) / 2);
    x = std::max(0, (max_x - target_w) / 2);
    wresize(win, target_h, target_w);
    mvwin(win, y, x);
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
        y = std::max(0, (max_y - target_h) / 2);
        x = std::max(0, (max_x - target_w) / 2);
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
        int ts_max = target_w - 4;
        auto it = current_top;
        while (it != log_manager.get_logs().end() && line < target_h - 1) {
            const auto& entry = *it;
            int ts_len = entry.timestamp.length();
            // Print timestamp (truncated if needed)
            wattron(win, COLOR_PAIR(3));
            std::string ts_disp = "[" + entry.timestamp + "]";
            if ((int)ts_disp.length() > ts_max) ts_disp = ts_disp.substr(0, ts_max - 1) + "~";
            mvwprintw(win, line, 2, "%s", ts_disp.c_str());
            wattroff(win, COLOR_PAIR(3));
            
            // Print message (truncated to remaining width)
            int msg_max = target_w - 2 - ts_disp.length();
            if (msg_max > 0) {
                mvwprintw(win, line, 2 + ts_disp.length(), "%.*s", msg_max, entry.message.c_str());
            }
            
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
