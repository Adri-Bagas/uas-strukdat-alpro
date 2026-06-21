#pragma once
#include <ncurses.h>
#include <string>
#include <vector>

namespace Utils {


enum class PopupState { ANIMATING, TYPING, WAITING, DISMISSED };

class Popup {
protected:
    int target_h, target_w, y, x;
    WINDOW *win;
    std::vector<std::string> wrapped_lines;
    
    PopupState state = PopupState::ANIMATING;
    int anim_step = 1;
    int type_line = 0;
    int type_char = 0;
    int frame_counter = 0;

public:
    Popup(const std::string &text);
    virtual ~Popup();
    
    virtual void update();
    virtual bool handle_input(int ch);
    virtual void render();
    void resize();
    
    bool is_dismissed() const { return state == PopupState::DISMISSED; }

    // Kept for backward compatibility with ErrorPopup
    void animate_blocking();
    void type_text_blocking();
};
} // namespace Utils
