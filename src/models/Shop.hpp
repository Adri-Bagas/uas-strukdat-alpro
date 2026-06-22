#pragma once
#include <string>
#include <vector>
#include <map>

struct ShopItem {
    std::string item_id;
    int base_stock;
};

struct RestockSchedule {
    int day;
    std::vector<std::string> item_ids;
};

class Shop {
private:
    std::string id;
    std::string name;
    std::vector<ShopItem> items;
    std::vector<RestockSchedule> restock_schedule;
    std::vector<std::string> on_enter;
    std::vector<std::string> on_exit;

public:
    Shop(std::string id, std::string name) : id(std::move(id)), name(std::move(name)) {}

    const std::string& get_id() const { return id; }
    const std::string& get_name() const { return name; }

    void add_item(const std::string& item_id, int base_stock) {
        items.push_back({item_id, base_stock});
    }

    void add_restock_schedule(int day, const std::vector<std::string>& item_ids) {
        restock_schedule.push_back({day, item_ids});
    }

    void add_on_enter(const std::string& action) { on_enter.push_back(action); }
    void add_on_exit(const std::string& action) { on_exit.push_back(action); }

    const std::vector<ShopItem>& get_items() const { return items; }
    const std::vector<RestockSchedule>& get_restock_schedule() const { return restock_schedule; }
    const std::vector<std::string>& get_on_enter() const { return on_enter; }
    const std::vector<std::string>& get_on_exit() const { return on_exit; }
};
