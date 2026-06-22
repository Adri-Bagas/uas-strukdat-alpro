// src/utils/components/EncyclopediaPopup.hpp
#pragma once
#include "Popup.hpp"
#include "../EncyclopediaBST.hpp"

namespace Utils {

class EncyclopediaPopup : public Popup {
private:
    EncyclopediaBST& bst;
    int current_selection = 0;
    std::string current_category = "all"; // "all", "npc", "monster"
    std::vector<const EncyclopediaEntry*> filtered_entries;
    int scroll_offset = 0;
    int display_lines;

    void update_filtered_list();

public:
    EncyclopediaPopup(EncyclopediaBST& bst, int w = 76, int h = 18);
    void update() override;
    bool handle_input(int ch) override;
    void render() override;
};

} // namespace Utils
