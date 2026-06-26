#pragma once
#include "Popup.hpp"
#include "../../models/Dialog.hpp"
#include <vector>

namespace Utils {

class ChoicePopup : public Popup {
private:
    std::vector<DialogChoice> choices;
    int selected_index;

public:
    ChoicePopup(const std::string& header, const std::vector<DialogChoice>& choices, int selected_idx);
    void render() override;
    void resize() override;
    void set_selected_index(int idx) { selected_index = idx; }
};
} // namespace Utils
