#include "ErrorPopup.hpp"
#include <ncurses.h>
#include <cstdlib>

ErrorPopup::ErrorPopup(const std::string &error_msg) 
    : Popup("FATAL ERROR: " + error_msg + " [Press any key to exit]") {
}

void ErrorPopup::show_fatal() {
    if (isendwin()) {
        initscr();
        cbreak();
        noecho();
    }
    
    start_color();
    init_pair(99, COLOR_WHITE, COLOR_RED);
    
    this->animate();
    wbkgd(win, COLOR_PAIR(99)); 
    wattron(win, COLOR_PAIR(99));
    wrefresh(win);
    
    this->type_text();
    wattroff(win, COLOR_PAIR(99));
    
    endwin();
    exit(1);
}