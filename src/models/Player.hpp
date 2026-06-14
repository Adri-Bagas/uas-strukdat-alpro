#pragma once
#include "NPC.hpp" // For Entity base class
#include "Item.hpp"
#include <unordered_map>
#include <string>

class Player : public Entity {
private:
    int mp, max_mp;
    
    int gold;
    
    // Menyimpan inventaris pemain menggunakan ID item dan jumlah (kuantitas)
    std::unordered_map<std::string, int> inventory;

    // Menyimpan equipment yang sedang dipakai. Key: "weapon", "armor", "boots", "ring"
    std::unordered_map<std::string, const Item*> equipped_items;

public:
    Player(std::string id, std::string name);
    ~Player() override = default;

    // Mana
    int get_mp() const { return mp; }
    int get_max_mp() const { return max_mp; }
    
    void consume_mp(int amount);
    void restore_mp(int amount);
    
    // Progression
    int get_gold() const { return gold; }
    void add_gold(int amount) { gold += amount; }
    bool spend_gold(int amount);

    // Inventory
    void add_item(const std::string& item_id, int amount = 1);
    void remove_item(const std::string& item_id, int amount = 1);
    int get_item_count(const std::string& item_id) const;
    const std::unordered_map<std::string, int>& get_inventory() const { return inventory; }

    // Equipment
    bool equip(const Item* item);
    void unequip(const std::string& slot);
    const Item* get_equipped(const std::string& slot) const;
    const std::unordered_map<std::string, const Item*>& get_all_equipped() const { return equipped_items; }

    // Game Variables (Quest/Progress Tracking)
    void set_var(const std::string& key, int val) { game_vars[key] = val; }
    void add_var(const std::string& key, int amount) { game_vars[key] += amount; }
    int get_var(const std::string& key) const {
        auto it = game_vars.find(key);
        return (it != game_vars.end()) ? it->second : 0;
    }
    const std::unordered_map<std::string, int>& get_all_vars() const { return game_vars; }

private:
    std::unordered_map<std::string, int> game_vars;
};
