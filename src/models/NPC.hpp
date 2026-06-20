#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "../enums/Element.hpp"
#include "CombatActions.hpp"

enum class NPCType {
    NAMED,
    UNNAMED
};

class Entity {
protected:
    std::string id;
    std::string name;
    
    // Base RPG Stats
    int str, cons, agi, intl, wis;
    Element affinity;

    // Vitality
    int hp, max_hp;
    int mp = 0, max_mp = 0; // Move MP up to Entity so all combatants have it

    // Combat Data
    std::vector<Magic> magics;
    SpecialMove special_move;
    bool has_special_move = false;
    std::vector<CombatModifier> active_modifiers;

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

    int get_mp() const { return mp; }
    int get_max_mp() const { return max_mp; }
    void set_max_mp(int v) { max_mp = v; if (mp > max_mp) mp = max_mp; }
    
    virtual void consume_mp(int amount) {
        mp -= amount;
        if (mp < 0) mp = 0;
    }
    
    virtual void restore_mp(int amount) {
        mp += amount;
        if (mp > max_mp) mp = max_mp;
    }

    const std::vector<Magic>& get_magics() const { return magics; }
    void add_magic(const Magic& m) { magics.push_back(m); }

    bool has_special() const { return has_special_move; }
    SpecialMove& get_special_move() { return special_move; }
    void set_special_move(const SpecialMove& sm) { special_move = sm; has_special_move = true; }

    bool is_dead() const { return hp <= 0; }
};

struct ScheduleEntry {
    std::vector<int> days; // Empty means everyday
    std::string phase;
    std::string location_id;
};

class NPC : public Entity {
private:
    NPCType type;
    std::string role;
    std::string faction;
    std::string full_name; // Hidden full name with titles
    int trust_level;
    
    std::vector<ScheduleEntry> schedules; 
    std::string default_dialog_id;
    bool is_known = false;

    // For Quest Triggers
    std::vector<std::string> quest_ids;

public:
    NPC(std::string id, std::string name, NPCType type, std::string role, std::string faction = "Neutral") 
        : Entity(std::move(id), std::move(name)), type(type), role(std::move(role)), faction(std::move(faction)), trust_level(0) {}

    NPCType get_type() const { return type; }
    const std::string& get_role() const { return role; }
    const std::string& get_faction() const { return faction; }
    const std::string& get_full_name() const { return full_name; }
    void set_full_name(std::string name) { full_name = std::move(name); }

    bool known() const { return type == NPCType::UNNAMED || is_known; }
    void reveal() { is_known = true; }

    void add_schedule_entry(ScheduleEntry entry) {
        schedules.push_back(std::move(entry));
    }

    std::string get_location(int day, const std::string& time_phase) const {
        std::string fallback = "Unknown";
        for (const auto& entry : schedules) {
            if (entry.phase == time_phase) {
                // Check if days match
                if (entry.days.empty()) {
                    fallback = entry.location_id; // Store as fallback if no specific day match found
                } else {
                    for (int d : entry.days) {
                        if (d == day) return entry.location_id; // Exact day match found!
                    }
                }
            }
        }
        return fallback;
    }

    void set_default_dialog(std::string dialog_id) { default_dialog_id = std::move(dialog_id); }
    const std::string& get_default_dialog() const { return default_dialog_id; }

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
    std::string description;
    int base_damage;
    std::vector<Loot> loot_table;

public:
    Monster(std::string id, std::string name, std::string desc, int max_hp, int damage, int agility)
        : Entity(std::move(id), std::move(name), 10, 10, agility, 10, 10), 
          description(std::move(desc)), base_damage(damage) {
        this->max_hp = max_hp;
        this->hp = max_hp;
    }

    const std::string& get_description() const { return description; }
    int get_damage() const { return base_damage; }

    void add_loot(std::string item_id, int chance) {
        loot_table.push_back({std::move(item_id), chance});
    }
    
    const std::vector<Loot>& get_loot_table() const { return loot_table; }
};