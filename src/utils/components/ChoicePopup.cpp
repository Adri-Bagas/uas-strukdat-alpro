#include "ChoicePopup.hpp"

namespace Utils {

ChoicePopup::ChoicePopup(const std::string& header, const std::vector<DialogChoice>& choices, int selected_idx)
    : Popup(header), choices(choices), selected_index(selected_idx) {
    
    // Adjust dimensions to account for choices
    target_w = 60;
    target_h = wrapped_lines.size() + choices.size() + 4;
    
    y = (LINES - target_h) / 2;
    x = (COLS - target_w) / 2;
    
    // Re-initialize window with new size
    delwin(win);
    win = newwin(target_h, target_w, y, x);
    keypad(win, TRUE);
}

void ChoicePopup::resize() {
    target_w = 60;
    if (target_w > COLS - 2) target_w = COLS - 2;
    target_h = wrapped_lines.size() + choices.size() + 4;
    if (target_h > LINES - 2) target_h = LINES - 2;

    y = std::max(0, (LINES - target_h) / 2);
    x = std::max(0, (COLS - target_w) / 2);
    wresize(win, target_h, target_w);
    mvwin(win, y, x);
}

void ChoicePopup::render() {
    if (!win) return;

    wbkgd(win, COLOR_PAIR(2));
    werase(win);
    box(win, 0, 0);
    
    // Title
    wattron(win, A_BOLD | COLOR_PAIR(4));
    mvwprintw(win, 0, 2, " PILIHAN ");
    wattroff(win, A_BOLD | COLOR_PAIR(4));

    // Header text
    for (size_t i = 0; i < wrapped_lines.size(); ++i) {
        mvwprintw(win, 1 + i, 2, "%.*s", target_w - 4, wrapped_lines[i].c_str());
    }

    // Divider
    mvwhline(win, wrapped_lines.size() + 1, 1, ACS_HLINE, target_w - 2);

    // Choices
    int choice_start_y = wrapped_lines.size() + 2;
    int opt_max = target_w - 4;
    for (size_t i = 0; i < choices.size(); ++i) {
        std::string txt = choices[i].text;
        if ((int)txt.length() > opt_max) txt = txt.substr(0, opt_max - 1) + "~";
        if (selected_index == (int)i) {
            wattron(win, A_REVERSE | COLOR_PAIR(4));
            mvwprintw(win, choice_start_y + i, 2, "> %s", txt.c_str());
            wattroff(win, A_REVERSE | COLOR_PAIR(4));
        } else {
            mvwprintw(win, choice_start_y + i, 2, "  %s", txt.c_str());
        }
    }

    wrefresh(win);
}
} // namespace Utils
