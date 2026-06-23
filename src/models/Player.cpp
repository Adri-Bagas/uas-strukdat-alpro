#include "Player.hpp"

Player::Player(std::string id, std::string name) 
    : Entity(std::move(id), std::move(name), 15, 15, 15, 15, 15, 1, Element::NONE), 
      gold(0) {
    
    // Initialize derived stats
    set_max_hp(get_cons() * 10);
    this->hp = get_max_hp();
    
    set_max_mp(get_intl() * 5);
    this->mp = get_max_mp();

    gold = 0;
    set_var("day", 1);
}

bool Player::spend_gold(int amount) {
    if (gold >= amount) {
        gold -= amount;
        return true;
    }
    return false;
}

// Menambahkan item ke inventory berdasarkan ID
void Player::add_item(const std::string& item_id, int amount) {
    if (amount <= 0) return;
    inventory[item_id] += amount;
}

// Menghapus item dari inventory
void Player::remove_item(const std::string& item_id, int amount) {
    if (amount <= 0) return;
    
    if (inventory.find(item_id) != inventory.end()) {
        inventory[item_id] -= amount;
        // Jika jumlahnya 0 atau kurang, hapus item dari map
        if (inventory[item_id] <= 0) {
            inventory.erase(item_id);
        }
    }
}

// Mendapatkan jumlah item tertentu
int Player::get_item_count(const std::string& item_id) const {
    auto it = inventory.find(item_id);
    if (it != inventory.end()) {
        return it->second;
    }
    return 0;
}

// ---- EQUIPMENT SYSTEM ----

bool Player::equip(const Item* item) {
    if (!item || item->type != ItemType::EQUIPMENT) return false;
    
    std::string slot = item->equip_slot;
    if (slot != "weapon" && slot != "armor" && slot != "boots" && slot != "ring") {
        return false; // Invalid slot
    }

    if (get_item_count(item->id) <= 0) return false; // Tidak punya itemnya

    // Lepas item lama jika slot sudah terisi
    unequip(slot);

    // Hapus dari inventory
    remove_item(item->id, 1);
    
    // Ubah affinity jika ada
    if (item->has_affinity_change) {
        set_affinity(item->equip_affinity);
    }
    
    equipped_items[slot] = item;
    return true;
}

void Player::unequip(const std::string& slot) {
    auto it = equipped_items.find(slot);
    if (it != equipped_items.end() && it->second != nullptr) {
        const Item* item = it->second;
        
        // Masukkan kembali ke inventory
        add_item(item->id, 1);

        // Reset affinity jika item ini yang merubahnya
        if (item->has_affinity_change) {
            set_affinity(Element::NONE);
        }
        
        equipped_items.erase(it);
    }
}

const Item* Player::get_equipped(const std::string& slot) const {
    auto it = equipped_items.find(slot);
    if (it != equipped_items.end()) {
        return it->second;
    }
    return nullptr;
}

// Dynamically calculate stats based on base stat + equipment bonuses
int Player::get_str() const {
    int total = str;
    for (const auto& [slot, item] : equipped_items) {
        if (item && item->equip_stats.count("str")) total += item->equip_stats.at("str");
    }
    return total;
}

int Player::get_cons() const {
    int total = cons;
    for (const auto& [slot, item] : equipped_items) {
        if (item && item->equip_stats.count("cons")) total += item->equip_stats.at("cons");
    }
    return total;
}

int Player::get_agi() const {
    int total = agi;
    for (const auto& [slot, item] : equipped_items) {
        if (item && item->equip_stats.count("agi")) total += item->equip_stats.at("agi");
    }
    return total;
}

int Player::get_intl() const {
    int total = intl;
    for (const auto& [slot, item] : equipped_items) {
        if (item && item->equip_stats.count("intl")) total += item->equip_stats.at("intl");
    }
    return total;
}

int Player::get_wis() const {
    int total = wis;
    for (const auto& [slot, item] : equipped_items) {
        if (item && item->equip_stats.count("wis")) total += item->equip_stats.at("wis");
    }
    return total;
}

std::string Player::get_weapon_type() const {
    const Item* wpn = get_equipped("weapon");
    if (wpn) return wpn->weapon_type;
    return "unarmed";
}

std::string Player::get_weapon_name() const {
    const Item* wpn = get_equipped("weapon");
    if (wpn) return wpn->name;
    return "tangan kosong";
}
