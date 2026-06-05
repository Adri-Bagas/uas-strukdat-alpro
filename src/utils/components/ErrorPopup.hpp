#pragma once
#include "Popup.hpp"
#include <string>

class ErrorPopup : public Popup {
public:
    ErrorPopup(const std::string &error_msg);
    void show_fatal();
};