#pragma once
#include "NPC.hpp" // For Entity base class
#include "Item.hpp"
#include <vector>
#include <algorithm>

class Player : public Entity {
private:
    int mp, max_mp;
    int stamina, max_stamina;
    
    int gold;
    std::vector<Item*> inventory;

public:
    Player(std::string id, std::string name);
    ~Player() override = default;

    // Mana & Stamina
    int get_mp() const { return mp; }
    int get_max_mp() const { return max_mp; }
    int get_stamina() const { return stamina; }
    int get_max_stamina() const { return max_stamina; }
    
    void consume_mp(int amount);
    void restore_mp(int amount);
    void consume_stamina(int amount);
    void restore_stamina(int amount);

    // Progression
    int get_gold() const { return gold; }
    void add_gold(int amount) { gold += amount; }
    bool spend_gold(int amount);

    // Inventory
    void add_item(Item* item);
    void remove_item(Item* item);
    const std::vector<Item*>& get_inventory() const { return inventory; }
};
