#pragma once
#include <ncurses.h>
#include <string>


class Popup {
    int target_h, target_w, y, x;
    WINDOW *win;
public:
    Popup(int h, int w);
    ~Popup();
    void animate();
    void type_text(const std::string &text);
};