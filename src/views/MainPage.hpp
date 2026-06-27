#pragma once
#include <ncurses.h>
#include <string>
#include <vector>
#include <functional>
#include "../models/Dialog.hpp"
#include <string_view>

struct GraphNode {
    std::string id;
    std::string name;
    int lx; 
    int ly; 
};

struct GraphEdge {
    std::string u;
    std::string v;
};

class MainPage {
    static const char* big_digits[10][5];
    void create_windows();
    void destroy_windows();

  public:
    WINDOW *win_thought, *win_dialog, *win_stat, *win_hp, *win_cal, *win_menu, *win_task;
    int w_left, w_right, w_col1, w_col2, w_col3;
    
    MainPage();
    void draw_title(WINDOW *win, const char *title, int color_pair_id);
    void draw();
    void resize();
    void type_new_text(WINDOW* win, const char* title, int width, 
                       const std::vector<DialogNode>& history, 
                       const DialogNode& new_text,
                       std::function<void()> on_type_start = nullptr,
                       std::function<void()> on_type_stop = nullptr);
    
    void render_history(WINDOW* win, const std::vector<DialogNode>& history);
    
    void draw_calendar(WINDOW* win, int days_left, int month, int day, std::string time, std::string location_name);
    
    void draw_map(WINDOW* win, const std::vector<GraphNode>& nodes, const std::vector<GraphEdge>& edges, std::string_view selected_id, bool is_focused, std::string_view current_id);
    
    void draw_player_stats(WINDOW* win, int str, int cons, int agi, int intl, int wis, int stat_points, std::string affinity, int gold, const std::vector<std::string>& equipped_info);
    void draw_vitals(WINDOW* win, int hp, int max_hp, int mp, int max_mp);
    void draw_inventory(WINDOW* win, const std::vector<std::string>& item_names);
    void draw_tasks(WINDOW* win, const std::vector<std::string>& tasks);

    bool is_initialized() const;

    ~MainPage();
};