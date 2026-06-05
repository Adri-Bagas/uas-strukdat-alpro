#pragma once
#include <ncurses.h>
#include <string>
#include <vector>


class Popup {
protected:
    int target_h, target_w, y, x;
    WINDOW *win;
    std::vector<std::string> wrapped_lines;
public:
    Popup(const std::string &text);
    ~Popup();
    void animate();
    void type_text();
};