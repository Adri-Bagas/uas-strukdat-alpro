#include "ShopManager.hpp"
#include "../utils/Logger.hpp"
#include <sstream>

void ShopManager::init_from_db(const DB& db) {
    live_stocks.clear();
    for (const Shop* shop : db.get_all_shops()) {
        const std::string& shop_id = shop->get_id();
        for (const ShopItem& item : shop->get_items()) {
            live_stocks[shop_id][item.item_id] = item.base_stock;
        }
    }
    Utils::Logger::log("ShopManager: Initialized live stocks from DB");
}

int ShopManager::get_stock(const std::string& shop_id, const std::string& item_id) const {
    auto shop_it = live_stocks.find(shop_id);
    if (shop_it != live_stocks.end()) {
        auto item_it = shop_it->second.find(item_id);
        if (item_it != shop_it->second.end()) {
            return item_it->second;
        }
    }
    return 0;
}

bool ShopManager::buy_item(const std::string& shop_id, const std::string& item_id, int amount) {
    auto shop_it = live_stocks.find(shop_id);
    if (shop_it != live_stocks.end()) {
        auto item_it = shop_it->second.find(item_id);
        if (item_it != shop_it->second.end()) {
            if (item_it->second >= amount) {
                item_it->second -= amount;
                return true;
            }
        }
    }
    return false; // Not enough stock or item not found
}

void ShopManager::add_stock(const std::string& shop_id, const std::string& item_id, int amount) {
    live_stocks[shop_id][item_id] += amount;
}

void ShopManager::process_daily_restock(const DB& db, int current_day_of_week) {
    for (const Shop* shop : db.get_all_shops()) {
        const std::string& shop_id = shop->get_id();
        for (const RestockSchedule& rs : shop->get_restock_schedule()) {
            if (rs.day == current_day_of_week) {
                // Restock these items up to base_stock
                for (const std::string& item_id : rs.item_ids) {
                    // Find base stock
                    int base = 0;
                    for (const ShopItem& si : shop->get_items()) {
                        if (si.item_id == item_id) {
                            base = si.base_stock;
                            break;
                        }
                    }
                    if (base > 0) {
                        int current = live_stocks[shop_id][item_id];
                        if (current < base) {
                            live_stocks[shop_id][item_id] = base;
                            Utils::Logger::log("ShopManager: Restocked " + item_id + " in shop " + shop_id + " to " + std::to_string(base));
                        }
                    }
                }
            }
        }
    }
}

std::string ShopManager::serialize() const {
    std::stringstream ss;
    ss << live_stocks.size() << "\n";
    for (const auto& shop_pair : live_stocks) {
        ss << shop_pair.first << " " << shop_pair.second.size() << "\n";
        for (const auto& item_pair : shop_pair.second) {
            ss << item_pair.first << " " << item_pair.second << "\n";
        }
    }
    return ss.str();
}

void ShopManager::deserialize(const std::string& data) {
    std::stringstream ss(data);
    live_stocks.clear();
    size_t shop_count = 0;
    if (!(ss >> shop_count)) return;
    
    for (size_t i = 0; i < shop_count; ++i) {
        std::string shop_id;
        size_t item_count = 0;
        if (!(ss >> shop_id >> item_count)) break;
        
        for (size_t j = 0; j < item_count; ++j) {
            std::string item_id;
            int stock = 0;
            if (ss >> item_id >> stock) {
                live_stocks[shop_id][item_id] = stock;
            }
        }
    }
}
