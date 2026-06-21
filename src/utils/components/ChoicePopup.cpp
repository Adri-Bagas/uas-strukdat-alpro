#include "ChoicePopup.hpp"
#include <sstream>

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
        mvwprintw(win, 1 + i, 2, "%s", wrapped_lines[i].c_str());
    }

    // Divider
    mvwhline(win, wrapped_lines.size() + 1, 1, ACS_HLINE, target_w - 2);

    // Choices
    int choice_start_y = wrapped_lines.size() + 2;
    for (size_t i = 0; i < choices.size(); ++i) {
        if (selected_index == (int)i) {
            wattron(win, A_REVERSE | COLOR_PAIR(4));
            mvwprintw(win, choice_start_y + i, 2, "> %s", choices[i].text.c_str());
            wattroff(win, A_REVERSE | COLOR_PAIR(4));
        } else {
            mvwprintw(win, choice_start_y + i, 2, "  %s", choices[i].text.c_str());
        }
    }

    wrefresh(win);
}
} // namespace Utils
