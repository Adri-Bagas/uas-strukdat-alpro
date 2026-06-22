// src/utils/components/EncyclopediaPopup.cpp
#include "EncyclopediaPopup.hpp"
#include <algorithm>

namespace Utils {

EncyclopediaPopup::EncyclopediaPopup(EncyclopediaBST& bst, int w, int h)
    : Popup(""), bst(bst), display_lines(h - 5) {
    target_w = w;
    target_h = h;

    delwin(win); // Hancurkan window default ncurses berukuran kecil
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    y = (max_y - target_h) / 2;
    x = (max_x - target_w) / 2;
    win = newwin(target_h, target_w, y, x);
    keypad(win, TRUE);

    state = PopupState::WAITING;
    anim_step = target_w; // Skip animasi transisi agar responsif

    update_filtered_list();
}

void EncyclopediaPopup::update_filtered_list() {
    if (current_category == "all") {
        filtered_entries = bst.get_all_entries();
    } else {
        filtered_entries = bst.get_entries_by_category(current_category);
    }

    if (filtered_entries.empty()) {
        current_selection = 0;
    } else {
        current_selection = std::clamp(current_selection, 0, (int)filtered_entries.size() - 1);
    }
}

void EncyclopediaPopup::update() {
    // Override untuk mencegah rendering animasi text ketik
}

bool EncyclopediaPopup::handle_input(int ch) {
    if (ch == KEY_RESIZE) {
        resize();
        return true;
    }

    if (ch == 'q' || ch == 27 || ch == '\n' || ch == ' ') { // Keluar
        state = PopupState::DISMISSED;
        return true;
    }

    if (ch == KEY_UP || ch == 'w') {
        if (!filtered_entries.empty()) {
            current_selection--;
            if (current_selection < 0) current_selection = filtered_entries.size() - 1;
            
            // Atur scroll offset agar selection tetap terlihat di dalam list
            if (current_selection < scroll_offset) {
                scroll_offset = current_selection;
            } else if (current_selection >= scroll_offset + display_lines) {
                scroll_offset = current_selection - display_lines + 1;
            }
        }
    } else if (ch == KEY_DOWN || ch == 's') {
        if (!filtered_entries.empty()) {
            current_selection++;
            if (current_selection >= (int)filtered_entries.size()) current_selection = 0;

            // Atur scroll offset
            if (current_selection < scroll_offset) {
                scroll_offset = current_selection;
            } else if (current_selection >= scroll_offset + display_lines) {
                scroll_offset = current_selection - display_lines + 1;
            }
        }
    } else if (ch == KEY_LEFT || ch == 'a') {
        if (current_category == "all") current_category = "monster";
        else if (current_category == "monster") current_category = "npc";
        else current_category = "all";
        
        current_selection = 0;
        scroll_offset = 0;
        update_filtered_list();
    } else if (ch == KEY_RIGHT || ch == 'd') {
        if (current_category == "all") current_category = "npc";
        else if (current_category == "npc") current_category = "monster";
        else current_category = "all";

        current_selection = 0;
        scroll_offset = 0;
        update_filtered_list();
    }

    return true;
}

void EncyclopediaPopup::render() {
    if (state == PopupState::DISMISSED) return;

    if (!win) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        y = (max_y - target_h) / 2;
        x = (max_x - target_w) / 2;
        win = newwin(target_h, target_w, y, x);
    }

    werase(win);
    wbkgdset(win, COLOR_PAIR(2)); // Menggunakan stale color
    box(win, 0, 0);

    // Render Judul Popup
    wattron(win, A_BOLD | COLOR_PAIR(4)); // Active/Yellow Color
    mvwprintw(win, 0, (target_w - 20) / 2, " ENSIKLOPEDIA NIRVA ");
    wattroff(win, A_BOLD | COLOR_PAIR(4));

    // Render Filter Tabs
    mvwprintw(win, 1, 2, "Filter: ");
    std::string tabs[] = {"Semua", "NPC", "Monster"};
    std::string codes[] = {"all", "npc", "monster"};
    int tab_x = 10;
    for (int i = 0; i < 3; ++i) {
        if (current_category == codes[i]) {
            wattron(win, A_REVERSE | COLOR_PAIR(4));
            mvwprintw(win, 1, tab_x, " %s ", tabs[i].c_str());
            wattroff(win, A_REVERSE | COLOR_PAIR(4));
        } else {
            mvwprintw(win, 1, tab_x, " %s ", tabs[i].c_str());
        }
        tab_x += tabs[i].length() + 4;
    }

    // Horizontal Line di bawah Filter
    mvwhline(win, 2, 1, ACS_HLINE, target_w - 2);

    // Kiri: Daftar Entitas
    int list_w = 26;
    int start_y = 3;
    for (int i = 0; i < display_lines; ++i) {
        int idx = scroll_offset + i;
        if (idx >= (int)filtered_entries.size()) break;

        const auto* entry = filtered_entries[idx];
        std::string display_name = entry->is_discovered ? entry->name : "??? (Terkunci)";
        
        // Pangkas jika terlalu panjang
        if (display_name.length() > (size_t)list_w - 4) {
            display_name = display_name.substr(0, list_w - 7) + "...";
        }

        if (idx == current_selection) {
            wattron(win, A_REVERSE | COLOR_PAIR(4));
            mvwprintw(win, start_y + i, 2, " > %-22s", display_name.c_str());
            wattroff(win, A_REVERSE | COLOR_PAIR(4));
        } else {
            mvwprintw(win, start_y + i, 2, "   %-22s", display_name.c_str());
        }
    }

    // Scroll Indicator di kiri
    if (scroll_offset > 0) {
        mvwprintw(win, start_y, list_w - 1, "^");
    }
    if (scroll_offset + display_lines < (int)filtered_entries.size()) {
        mvwprintw(win, start_y + display_lines - 1, list_w - 1, "v");
    }

    // Pembatas Vertikal tengah
    for (int i = 3; i < target_h - 1; ++i) {
        mvwaddch(win, i, list_w + 1, ACS_VLINE);
    }
    mvwaddch(win, 2, list_w + 1, ACS_TTEE);
    mvwaddch(win, target_h - 1, list_w + 1, ACS_BTEE);

    // Kanan: Panel Detail
    int detail_x = list_w + 3;
    int detail_w = target_w - detail_x - 2;

    if (filtered_entries.empty()) {
        mvwprintw(win, target_h / 2, detail_x + (detail_w - 22) / 2, "Tidak ada data entitas.");
    } else {
        const auto* entry = filtered_entries[current_selection];
        if (entry->is_discovered) {
            // Judul Entitas
            wattron(win, A_BOLD | COLOR_PAIR(4));
            mvwprintw(win, 3, detail_x, "%s", entry->name.c_str());
            wattroff(win, A_BOLD | COLOR_PAIR(4));

            // Kategori
            std::string cat_label = (entry->category == "npc") ? "Karakter Kota (NPC)" : "Musuh Dungeon (Monster)";
            wattron(win, COLOR_PAIR(3));
            mvwprintw(win, 4, detail_x, "[ %s ]", cat_label.c_str());
            wattroff(win, COLOR_PAIR(3));

            // Garis pembatas kecil di panel kanan
            mvwhline(win, 5, detail_x, ACS_HLINE, detail_w);

            // Tampilkan deskripsi line by line
            std::string desc = entry->description;
            int print_y = 6;
            size_t pos = 0;
            while ((pos = desc.find('\n')) != std::string::npos && print_y < target_h - 2) {
                std::string line = desc.substr(0, pos);
                mvwprintw(win, print_y++, detail_x, "%s", line.c_str());
                desc.erase(0, pos + 1);
            }
            if (print_y < target_h - 2) {
                mvwprintw(win, print_y, detail_x, "%s", desc.c_str());
            }
        } else {
            // Tampilan Terkunci
            wattron(win, A_BOLD | COLOR_PAIR(5)); // Danger/Red Color
            mvwprintw(win, 3, detail_x, "??? (Terkunci)");
            wattroff(win, A_BOLD | COLOR_PAIR(5));

            mvwprintw(win, 4, detail_x, "[ Data Belum Diketahui ]");
            mvwhline(win, 5, detail_x, ACS_HLINE, detail_w);

            mvwprintw(win, 7, detail_x, "Informasi mengenai entitas ini belum");
            mvwprintw(win, 8, detail_x, "diketahui.");
            
            if (entry->category == "npc") {
                mvwprintw(win, 10, detail_x, "Saran: Temui NPC ini di wilayah kota");
                mvwprintw(win, 11, detail_x, "dan ajaklah mengobrol.");
            } else {
                mvwprintw(win, 10, detail_x, "Saran: Jelajahi dungeon dan kalahkan");
                mvwprintw(win, 11, detail_x, "monster ini setidaknya sekali.");
            }
        }
    }

    // Panduan Navigasi di bagian bawah
    wattron(win, COLOR_PAIR(2));
    mvwprintw(win, target_h - 1, 2, " [W/S] Gulir | [A/D] Filter Kategori | [ESC/Q/ENTER] Tutup ");
    wattroff(win, COLOR_PAIR(2));

    wrefresh(win);
}

} // namespace Utils
