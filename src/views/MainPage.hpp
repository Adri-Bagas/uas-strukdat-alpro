#pragma once
#include <ncurses.h>
#include <string>
#include <vector>

class MainPage {
    static const char* big_digits[10][5];
    void create_windows();
    void destroy_windows();

  public:
    WINDOW *win_thought, *win_dialog, *win_stat, *win_hp, *win_cal, *win_menu, *win_task;
    int w_left, w_right, w_col1, w_col2, w_col3;
    
    MainPage();
    void draw_title(WINDOW *win, const char *title, int width, int color_pair_id);
    void draw();
    void resize();
    void type_new_text(WINDOW* win, const char* title, int width, 
                       const std::vector<std::string>& history, 
                       const std::string& new_text);
    
    void render_history(WINDOW* win, const std::vector<std::string>& history);
    void draw_calendar(WINDOW* win, int days_left, int month, int day, std::string time);
    
    void draw_player_stats(WINDOW* win, int str, int cons, int agi, int intl, int wis, std::string affinity);
    void draw_vitals(WINDOW* win, int hp, int max_hp, int mp, int max_mp, int stamina, int max_stamina);
    void draw_inventory(WINDOW* win, const std::vector<std::string>& item_names, int gold);
    void draw_tasks(WINDOW* win, const std::vector<std::string>& tasks);

    ~MainPage();
};