#include "Popup.hpp"
#include <sstream>
#include <vector>

Popup::Popup(int h, int w) : target_h(h), target_w(w) {
    y = (LINES - h) / 2;
    x = (COLS - w) / 2;
    win = newwin(2, 4, y + (h/2), x + (w/2));
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
        napms(20);
    }
}

void Popup::type_text(const std::string& text) {
    std::stringstream ss(text);
    std::string line;
    std::vector<std::string> lines;
    while(std::getline(ss, line, '\n')) lines.push_back(line);

    int start_y = (target_h - lines.size()) / 2;
    for (const auto& l : lines) {
        int start_x = (target_w - (int)l.length()) / 2;
        for (char c : l) {
            mvwaddch(win, start_y, start_x++, c);
            wrefresh(win);
            napms(40);
        }
        start_y++;
    }
    while (wgetch(win) != '\n');
    werase(win); wrefresh(win);
}