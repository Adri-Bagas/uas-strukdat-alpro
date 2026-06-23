#pragma once
#include "Popup.hpp"
#include <vector>
#include <string>
#include <functional>

namespace Utils {

class MenuPopup : public Popup {
private:
    std::string header;
    std::vector<std::string> options;
    std::function<void(int)> on_select;
    int selected_index = 0;

public:
    MenuPopup(const std::string& title, const std::vector<std::string>& opts, std::function<void(int)> callback);
    void update() override;
    bool handle_input(int ch) override;
    void render() override;
};

} // namespace Utils
