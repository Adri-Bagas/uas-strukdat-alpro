#include "PlayerManager.hpp"

void PlayerManager::init_player(const std::string& id, const std::string& name) {
    player = std::make_unique<Player>(id, name);
}

Player* PlayerManager::get_player() const {
    return player.get();
}
