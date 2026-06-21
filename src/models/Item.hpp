#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "../enums/ItemType.hpp"
#include "../enums/Element.hpp"

struct Item {
    std::string id;
    std::string name;
    std::string description;
    
    // Tipe item menggunakan Enum
    ItemType type = ItemType::MISC; 
    
    // Harga dasar item jika dijual atau dibeli
    int value = 0; 
    // ---- EQUIPMENT DATA ----
    // Slot equipment: "weapon", "armor", "boots", "ring"
    std::string equip_slot = ""; 
    
    // Tipe senjata: "sword", "bow", "dagger", "staff", "unarmed"
    std::string weapon_type = "unarmed";
    
    // Bonus stat: "str" -> 5, "agi" -> -1, dll.
    std::unordered_map<std::string, int> equip_stats;

    // Elemental Affinity change
    bool has_affinity_change = false;
    Element equip_affinity = Element::NONE;

    // Aksi yang dieksekusi oleh Action dispatcher saat item digunakan (contoh: "add_hp 50")
    std::vector<std::string> on_use;
    
public:
    Item() = default;
    Item(std::string id, std::string name, std::string description) : id(std::move(id)), name(std::move(name)), description(std::move(description)) {}
    
    const std::string& get_id() const { return id; }
    const std::string& get_name() const { return name; }
};