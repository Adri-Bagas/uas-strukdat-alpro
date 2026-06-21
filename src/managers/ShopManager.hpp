#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "../db/DB.hpp"

class ShopManager {
private:
    // map of shop_id -> map of item_id -> current_stock
    std::unordered_map<std::string, std::unordered_map<std::string, int>> live_stocks;

public:
    ShopManager() = default;

    // Call this when starting a new game
    void init_from_db(const DB& db);

    // Get current stock for an item in a shop
    int get_stock(const std::string& shop_id, const std::string& item_id) const;

    // Decrease stock when bought
    bool buy_item(const std::string& shop_id, const std::string& item_id, int amount = 1);

    // Sell item to shop (increases stock if it's an item the shop carries, or maybe shops just buy anything)
    // For now we just add stock if it's in the shop's catalog.
    void add_stock(const std::string& shop_id, const std::string& item_id, int amount = 1);

    // Called every new day to restock
    void process_daily_restock(const DB& db, int current_day_of_week);

    // Save/Load
    std::string serialize() const;
    void deserialize(const std::string& data);
};
