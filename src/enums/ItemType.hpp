#pragma once
#include <string>

enum class ItemType {
    CONSUMABLE,
    MATERIAL,
    KEY_ITEM,
    EQUIPMENT,
    MISC
};

// Fungsi pembantu untuk konversi dari JSON string ke Enum
inline ItemType string_to_item_type(const std::string& type_str) {
    if (type_str == "consumable") return ItemType::CONSUMABLE;
    if (type_str == "material") return ItemType::MATERIAL;
    if (type_str == "key_item") return ItemType::KEY_ITEM;
    if (type_str == "equipment") return ItemType::EQUIPMENT;
    return ItemType::MISC; // Default
}

// Fungsi pembantu untuk UI atau Log
inline std::string item_type_to_string(ItemType type) {
    switch (type) {
        case ItemType::CONSUMABLE: return "Consumable";
        case ItemType::MATERIAL: return "Material";
        case ItemType::KEY_ITEM: return "Key Item";
        case ItemType::EQUIPMENT: return "Equipment";
        default: return "Misc";
    }
}
