#include "PlayerManager.hpp"

void PlayerManager::init_player(const std::string& id, const std::string& name) {
    player = std::make_unique<Player>(id, name);
    player->add_item("ransum", 1);
}

Player* PlayerManager::get_player() const {
    return player.get();
}
