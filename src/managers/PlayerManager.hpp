#pragma once
#include <memory>
#include "../models/Player.hpp"

class PlayerManager {
private:
    std::unique_ptr<Player> player;

public:
    PlayerManager() = default;
    ~PlayerManager() = default;

    void init_player(const std::string& id, const std::string& name);
    Player* get_player() const;
};
