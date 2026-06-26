#pragma once
#include "Popup.hpp"
#include "../../managers/LogManager.hpp"

namespace Utils {

class LogPopup : public Popup {
private:
    LogManager& log_manager;
    DoubleLinkedList<LogManager::LogEntry>::Iterator current_top;
    int display_lines;

public:
    LogPopup(LogManager& manager, int w = 60, int h = 15);
    void update() override;
    bool handle_input(int ch) override;
    void render() override;
    void resize() override;
};

} // namespace Utils
