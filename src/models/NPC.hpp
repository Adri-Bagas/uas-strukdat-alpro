#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "../enums/Element.hpp"

class Entity {
protected:
    std::string id;
    std::string name;
    
    // Base RPG Stats
    int str, cons, agi, intl, wis;
    Element affinity;

    // Vitality
    int hp, max_hp;

public:
    Entity(std::string id, std::string name, int s=10, int c=10, int a=10, int i=10, int w=10, Element elem=Element::NONE) 
        : id(std::move(id)), name(std::move(name)), 
          str(s), cons(c), agi(a), intl(i), wis(w), affinity(elem) {
        
        // Simple health formula based on Constitution
        max_hp = cons * 10;
        hp = max_hp;
    }
          
    virtual ~Entity() = default;

    const std::string& get_id() const { return id; }
    const std::string& get_name() const { return name; }

    int get_str() const { return str; }
    int get_cons() const { return cons; }
    int get_agi() const { return agi; }
    int get_intl() const { return intl; }
    int get_wis() const { return wis; }
    Element get_affinity() const { return affinity; }

    void set_str(int v) { str = v; }
    void set_cons(int v) { cons = v; }
    void set_agi(int v) { agi = v; }
    void set_intl(int v) { intl = v; }
    void set_wis(int v) { wis = v; }
    void set_affinity(Element e) { affinity = e; }

    int get_hp() const { return hp; }
    int get_max_hp() const { return max_hp; }
    void set_max_hp(int v) { max_hp = v; if(hp > max_hp) hp = max_hp; }

    virtual void take_damage(int amount) {
        hp -= amount;
        if (hp < 0) hp = 0;
    }

    virtual void heal_hp(int amount) {
        hp += amount;
        if (hp > max_hp) hp = max_hp;
    }

    bool is_dead() const { return hp <= 0; }
};

class NPC : public Entity {
private:
    std::unordered_map<std::string, std::string> schedule; 
    std::string role;
    int trust_level;

    // For Quest Triggers
    std::vector<std::string> quest_ids;

public:
    NPC(std::string id, std::string name, std::string role) 
        : Entity(std::move(id), std::move(name)), role(std::move(role)), trust_level(0) {}

    void set_location(const std::string& time_phase, const std::string& location) {
        schedule[time_phase] = location;
    }

    std::string get_location(const std::string& time_phase) {
        if (schedule.find(time_phase) != schedule.end()) {
            return schedule[time_phase];
        }
        return "Unknown";
    }

    void modify_trust(int amount) { trust_level += amount; }
    int get_trust() const { return trust_level; }

    void assign_quest_id(const std::string& q_id) {
        quest_ids.push_back(q_id);
    }

    const std::vector<std::string>& get_quests() const {
        return quest_ids;
    }
};

struct Loot {
    std::string item_id;
    int drop_chance; 
};

class Monster : public Entity {
private:
    int base_damage;
    std::vector<Loot> loot_table;

public:
    Monster(std::string id, std::string name, int max_hp, int damage, int agility)
        : Entity(std::move(id), std::move(name), 10, 10, agility, 10, 10), 
          base_damage(damage) {
        this->max_hp = max_hp;
        this->hp = max_hp;
    }

    void add_loot(std::string item_id, int chance) {
        loot_table.push_back({std::move(item_id), chance});
    }

    int get_damage() const { return base_damage; }
};
