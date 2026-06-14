#include "Player.hpp"

Player::Player(std::string id, std::string name) 
    : Entity(std::move(id), std::move(name), 10, 10, 10, 10, 10, Element::NONE), 
      gold(0) {
    
    // Initialize derived stats
    max_hp = get_cons() * 10;
    hp = max_hp;
    
    max_mp = get_intl() * 5;
    mp = max_mp;
}

void Player::consume_mp(int amount) {
    mp -= amount;
    if (mp < 0) mp = 0;
}

void Player::restore_mp(int amount) {
    mp += amount;
    if (mp > max_mp) mp = max_mp;
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

// Helper untuk menambah/mengurangi stat dari equip_stats
void modify_stats(Player* p, const Item* item, int multiplier) {
    for (const auto& [stat, val] : item->equip_stats) {
        int mod = val * multiplier;
        if (stat == "str") p->set_str(p->get_str() + mod);
        else if (stat == "cons") p->set_cons(p->get_cons() + mod);
        else if (stat == "agi") p->set_agi(p->get_agi() + mod);
        else if (stat == "intl") p->set_intl(p->get_intl() + mod);
        else if (stat == "wis") p->set_wis(p->get_wis() + mod);
        // Bisa tambah HP / Max HP logic jika perlu
    }
}

bool Player::equip(const Item* item) {
    if (!item || item->type != ItemType::EQUIPMENT) return false;
    
    std::string slot = item->equip_slot;
    if (slot != "weapon" && slot != "armor" && slot != "boots" && slot != "ring") {
        return false; // Invalid slot
    }

    if (get_item_count(item->id) <= 0) return false; // Tidak punya itemnya

    // Lepas item lama jika slot sudah terisi
    unequip(slot);

    // Hapus dari inventory & tambah stat
    remove_item(item->id, 1);
    modify_stats(this, item, 1);
    
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
        
        // Kurangi stat & masukkan kembali ke inventory
        modify_stats(this, item, -1);
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
