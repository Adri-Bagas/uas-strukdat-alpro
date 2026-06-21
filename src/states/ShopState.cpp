#include "ShopState.hpp"
#include "../GameEngine.hpp"
#include <ncurses.h>

ShopState::ShopState(GameEngine* eng, const std::string& shop_id) 
    : GameState(eng), shop_id(shop_id), current_selection(0) {}

void ShopState::on_enter() {
    current_selection = 0;
    
    // Process on_enter actions of the shop
    const Shop* shop = engine->get_db().get_shop(shop_id);
    if (shop) {
        for (const auto& action : shop->get_on_enter()) {
            engine->get_actions().execute(action);
        }
        
        display_items.clear();
        for (const auto& item_stock : shop->get_items()) {
            display_items.push_back(item_stock.item_id);
        }
    }
    display_items.push_back("Exit Shop");
    render();
}

void ShopState::update() {
    // Nothing to update periodically
}

void ShopState::handle_input(int ch) {
    if (ch == -1) return;
    
    Player* p = engine->get_player_manager().get_player();
    if (!p) return;

    if (ch == KEY_UP || ch == 'w') {
        current_selection--;
        if (current_selection < 0) current_selection = display_items.size() - 1;
    } else if (ch == KEY_DOWN || ch == 's') {
        current_selection++;
        if (current_selection >= display_items.size()) current_selection = 0;
    } else if (ch == 10) { // Enter
        if (current_selection == display_items.size() - 1) { // Exit
            engine->pop_state();
            return;
        }
        
        const std::string& selected_item_id = display_items[current_selection];
        const Item* item = engine->get_db().get_item(selected_item_id);
        int stock = engine->get_shop_manager().get_stock(shop_id, selected_item_id);
        
        if (item) {
            int price = item->value;
            if (stock <= 0) {
                engine->get_dialogs().queue_popup(item->get_name() + " is out of stock.");
            } else if (p->get_gold() >= price) {
                p->add_gold(-price);
                p->add_item(selected_item_id, 1);
                engine->get_shop_manager().buy_item(shop_id, selected_item_id, 1);
                engine->get_dialogs().queue_popup("You bought a " + item->get_name() + " for " + std::to_string(price) + "G.");
            } else {
                engine->get_dialogs().queue_popup("Not enough gold to buy " + item->get_name() + ".");
            }
        }
    } else if (ch == 'q' || ch == 27 || ch == 'c' || ch == 'C') { // Escape/Quit
        engine->pop_state();
        return;
    }
}

void ShopState::render() {
    const Shop* shop = engine->get_db().get_shop(shop_id);
    if (!shop) return;
    
    Player* p = engine->get_player_manager().get_player();
    if (!p) return;

    int popup_w = 60;
    int popup_h = display_items.size() + 8;
    int popup_y = (LINES - popup_h) / 2;
    int popup_x = (COLS - popup_w) / 2;
    
    WINDOW* pop_win = newwin(popup_h, popup_w, popup_y, popup_x);
    if (pop_win) {
        wbkgdset(pop_win, COLOR_PAIR(2));
        werase(pop_win);
        box(pop_win, 0, 0);
        
        wattron(pop_win, COLOR_PAIR(4) | A_BOLD);
        mvwprintw(pop_win, 1, (popup_w - shop->get_name().length()) / 2, "%s", shop->get_name().c_str());
        wattroff(pop_win, COLOR_PAIR(4) | A_BOLD);
        
        mvwprintw(pop_win, 3, 2, "Your Gold: %dG", p->get_gold());
        mvwprintw(pop_win, 4, 2, "Items for Sale:");
        
        for (size_t i = 0; i < display_items.size(); ++i) {
            if (i == current_selection) {
                wattron(pop_win, COLOR_PAIR(1) | A_REVERSE);
            }
            
            if (i == display_items.size() - 1) {
                mvwprintw(pop_win, 6 + i, 4, "%-35s", "Exit Shop");
            } else {
                const std::string& item_id = display_items[i];
                const Item* item = engine->get_db().get_item(item_id);
                int stock = engine->get_shop_manager().get_stock(shop_id, item_id);
                if (item) {
                    std::string item_name = item->get_name();
                    std::string item_price = std::to_string(item->value) + "G";
                    std::string stock_str = " [Stock: " + std::to_string(stock) + "]";
                    if (stock <= 0) {
                        stock_str = " [SOLD OUT]";
                    }
                    std::string display_str = item_name + " - " + item_price + stock_str;
                    mvwprintw(pop_win, 6 + i, 4, "%s", display_str.c_str());
                } else {
                    mvwprintw(pop_win, 6 + i, 4, "%-35s ????G", display_items[i].c_str());
                }
            }
            
            if (i == current_selection) {
                wattroff(pop_win, COLOR_PAIR(1) | A_REVERSE);
            }
        }
        
        wnoutrefresh(pop_win);
        delwin(pop_win);
    }
}

void ShopState::on_exit() {
    const Shop* shop = engine->get_db().get_shop(shop_id);
    if (shop) {
        for (const auto& action : shop->get_on_exit()) {
            engine->get_actions().execute(action);
        }
    }
    engine->get_layout().resize(); // Redraw underlying layout
}
