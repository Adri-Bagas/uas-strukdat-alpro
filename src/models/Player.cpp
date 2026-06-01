#include "Player.hpp"

Player::Player(std::string id, std::string name) 
    : Entity(std::move(id), std::move(name), 10, 10, 10, 10, 10, Element::NONE), 
      gold(0) {
    
    // Initialize derived stats
    max_hp = get_cons() * 10;
    hp = max_hp;
    
    max_mp = get_intl() * 5;
    mp = max_mp;
    
    max_stamina = get_agi() * 10;
    stamina = max_stamina;
}

void Player::consume_mp(int amount) {
    mp -= amount;
    if (mp < 0) mp = 0;
}

void Player::restore_mp(int amount) {
    mp += amount;
    if (mp > max_mp) mp = max_mp;
}

void Player::consume_stamina(int amount) {
    stamina -= amount;
    if (stamina < 0) stamina = 0;
}

void Player::restore_stamina(int amount) {
    stamina += amount;
    if (stamina > max_stamina) stamina = max_stamina;
}

bool Player::spend_gold(int amount) {
    if (gold >= amount) {
        gold -= amount;
        return true;
    }
    return false;
}

void Player::add_item(Item* item) {
    if (item) {
        inventory.push_back(item);
    }
}

void Player::remove_item(Item* item) {
    auto it = std::find(inventory.begin(), inventory.end(), item);
    if (it != inventory.end()) {
        inventory.erase(it);
    }
}
