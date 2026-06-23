#include "InventoryState.hpp"
#include "../GameEngine.hpp"
#include "../models/Player.hpp"
#include "../db/DB.hpp"
#include "../enums/Element.hpp"
#include <ncurses.h>

InventoryState::InventoryState(GameEngine* eng) : GameState(eng) {}

void InventoryState::on_enter() {
    refresh_inventory();
    message = "Tekan W/S untuk memilih. Tekan ENTER untuk interaksi.";
    this->render();
}

void InventoryState::refresh_inventory() {
    current_items.clear();
    Player* p = engine->get_player_manager().get_player();
    if (!p) return;

    for (const auto& [id, count] : p->get_inventory()) {
        if (count > 0) {
            const Item* it = engine->get_db().get_item(id);
            if (it) {
                current_items.push_back({it, count});
            }
        }
    }
    
    if (selection_index >= (int)current_items.size()) {
        selection_index = current_items.empty() ? 0 : current_items.size() - 1;
    }
}

void InventoryState::handle_input(int ch) {
    if (ch == KEY_RESIZE) { 
        engine->get_layout().resize();
        return;
    }

    if (ch == 'q' || ch == 'Q' || ch == 'i' || ch == 'I' || ch == 27) { // 27 = ESC
        engine->pop_state();
        return;
    }
    
    if (current_items.empty()) return;

    if (ch == KEY_UP || ch == 'w' || ch == 'W' || ch == 'a' || ch == 'A' || ch == KEY_LEFT) {
        selection_index--;
        if (selection_index < 0) selection_index = current_items.size() - 1;
    } else if (ch == KEY_DOWN || ch == 's' || ch == 'S' || ch == 'd' || ch == 'D' || ch == KEY_RIGHT) {
        selection_index++;
        if (selection_index >= (int)current_items.size()) selection_index = 0;
    } else if (ch == '\n' || ch == ' ') {
        auto [item, count] = current_items[selection_index];
        Player* p = engine->get_player_manager().get_player();
        
        if (item->type == ItemType::CONSUMABLE) {
            for (const auto& action : item->on_use) {
                engine->get_actions().execute(action);
            }
            p->remove_item(item->id, 1);
            message = "Kamu menggunakan " + item->name + "!";
            refresh_inventory();
        } else if (item->type == ItemType::EQUIPMENT) {
            // Check if already equipped
            const Item* currently_equipped = p->get_equipped(item->equip_slot);
            if (currently_equipped && currently_equipped->id == item->id) {
                p->unequip(item->equip_slot);
                message = "Kamu melepaskan " + item->name + ".";
            } else {
                if (p->equip(item)) {
                    message = "Kamu memakai " + item->name + " di slot " + item->equip_slot + ".";
                } else {
                    message = "Gagal memakai " + item->name + "!";
                }
            }
        } else {
            message = item->name + " tidak bisa digunakan saat ini.";
        }
    }
}

void InventoryState::update() {
    // Nothing special
}

void InventoryState::render() {
    engine->get_layout().draw();
    Player* p = engine->get_player_manager().get_player();
    if (!p) return;

    // Draw vitals & stats like town
    std::vector<std::string> equip_info;
    const auto& equips = p->get_all_equipped();
    auto format_equip = [&](const std::string& slot, const std::string& label) {
        auto it = equips.find(slot);
        equip_info.push_back(label + ": " + (it != equips.end() && it->second ? it->second->name : "(Kosong)"));
    };
    format_equip("weapon", "Sjt"); format_equip("armor", "Zir"); format_equip("boots", "Sep"); format_equip("ring", "Cin");

    engine->get_layout().draw_player_stats(engine->get_layout().win_stat, p->get_str(), p->get_cons(), p->get_agi(), p->get_intl(), p->get_wis(), p->get_stat_points(), element_to_string(p->get_affinity()), p->get_gold(), equip_info);
    engine->get_layout().draw_vitals(engine->get_layout().win_hp, p->get_hp(), p->get_max_hp(), p->get_mp(), p->get_max_mp());

    // Use win_thought to show title & instructions
    WINDOW* wt = engine->get_layout().win_thought;
    wbkgdset(wt, COLOR_PAIR(2)); werase(wt); box(wt, 0, 0);
    engine->get_layout().draw_title(wt, "INVENTARIS & EQUIPMENT", engine->get_layout().w_right, 2);
    mvwprintw(wt, 2, 2, "Gunakan W/S untuk memilih item.");
    mvwprintw(wt, 3, 2, "Tekan ENTER untuk menggunakan barang atau memakai equipment.");
    mvwprintw(wt, 4, 2, "Tekan I atau Q untuk keluar.");
    wnoutrefresh(wt);

    // Left List (win_menu)
    std::vector<std::string> menu_display;
    menu_display.push_back("--- Daftar Item ---");
    
    if (current_items.empty()) {
        menu_display.push_back(" ");
        menu_display.push_back(" (Inventaris Kosong) ");
    } else {
        for (int i = 0; i < (int)current_items.size(); ++i) {
            auto [item, count] = current_items[i];
            std::string pref = (i == selection_index) ? "> " : "  ";
            
            // Check if equipped
            bool is_equipped = false;
            if (item->type == ItemType::EQUIPMENT) {
                const Item* eq = p->get_equipped(item->equip_slot);
                if (eq && eq->id == item->id) is_equipped = true;
            }

            std::string equip_mark = is_equipped ? "[E] " : "";
            std::string count_str = (item->type == ItemType::EQUIPMENT && count == 1) ? "" : " (x" + std::to_string(count) + ")";
            menu_display.push_back(pref + equip_mark + item->name + count_str);
        }
    }
    
    // Bottom msg in win_menu
    menu_display.push_back("");
    menu_display.push_back("Info: " + message);

    engine->get_layout().draw_tasks(engine->get_layout().win_menu, menu_display);

    // Right Details (win_task)
    std::vector<std::string> details_display;
    details_display.push_back("--- Detail Item ---");
    if (!current_items.empty() && selection_index >= 0 && selection_index < (int)current_items.size()) {
        auto [item, count] = current_items[selection_index];
        details_display.push_back("Nama  : " + item->name);
        details_display.push_back("Tipe  : " + item_type_to_string(item->type));
        details_display.push_back("Harga : " + std::to_string(item->value) + " G");
        details_display.push_back("Deskripsi:");
        
        // Simple manual word wrap for description
        std::string desc = item->description;
        int max_w = engine->get_layout().w_col3 - 4; // approximate
        if (max_w < 10) max_w = 20;
        std::string line = "";
        for (char c : desc) {
            line += c;
            if (c == ' ' && line.length() > (size_t)max_w) {
                details_display.push_back("  " + line);
                line = "";
            }
        }
        if (!line.empty()) details_display.push_back("  " + line);
        
        if (item->type == ItemType::EQUIPMENT) {
            details_display.push_back("");
            details_display.push_back("[ Info Equipment ]");
            details_display.push_back("Slot  : " + item->equip_slot);
            if (item->equip_slot == "weapon") {
                details_display.push_back("Jenis : " + item->weapon_type);
            }
            if (item->has_affinity_change && item->equip_affinity != Element::NONE) {
                details_display.push_back("Afin  : " + element_to_string(item->equip_affinity));
            }
            if (!item->equip_stats.empty()) {
                details_display.push_back("Bonus Atribut:");
                for (const auto& [stat, val] : item->equip_stats) {
                    details_display.push_back("  " + stat + " " + (val >= 0 ? "+" : "") + std::to_string(val));
                }
            }
        }
    }
    
    engine->get_layout().draw_tasks(engine->get_layout().win_task, details_display);
    
    // Refresh all windows safely without touching curses directly outside LayoutManager
}
