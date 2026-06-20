#pragma once
#include <ncurses.h>
#include <string>
#include <vector>

class StartMenuPage {
public:
    void draw(int selection_index, const std::vector<std::string>& menu_options);
};
