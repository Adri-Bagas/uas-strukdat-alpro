#pragma once
#include "Popup.hpp"
#include <functional>

namespace Utils {

class InputPopup : public Popup {
private:
    std::string prompt;
    std::string input_text;
    std::function<void(const std::string&)> on_submit;

public:
    InputPopup(const std::string& prompt_text, std::function<void(const std::string&)> submit_cb);
    void update() override;
    bool handle_input(int ch) override;
    void render() override;
    void resize() override;
};

} // namespace Utils
