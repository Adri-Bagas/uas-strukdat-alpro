#pragma once
#include "Popup.hpp"
#include "../EncyclopediaBST.hpp"

class GameEngine;

namespace Utils {

class EncyclopediaPopup : public Popup {
private:
    EncyclopediaBST& bst;
    int current_selection = 0;
    std::string current_category = "all";
    std::vector<const EncyclopediaEntry*> filtered_entries;
    int scroll_offset = 0;
    int display_lines;

    void update_filtered_list();

public:
    EncyclopediaPopup(EncyclopediaBST& bst, int w = 76, int h = 18);
    EncyclopediaPopup(::GameEngine* engine, int w = 76, int h = 18);
    void update() override;
    bool handle_input(int ch) override;
    void render() override;
    void resize() override;
};

} // namespace Utils
