#pragma once
#include <memory>
#include <array>
#include "../models/Player.hpp"
#include "../models/NPC.hpp"

class PlayerManager {
private:
    std::unique_ptr<Player> player;
    // Owned NPC objects in the party
    std::vector<std::unique_ptr<NPC>> npc_allies;
    
    // The current layout of the party on the 4 slots
    std::array<Entity*, 4> party_slots;

public:
    PlayerManager() = default;
    ~PlayerManager() = default;

    void init_player(const std::string& id, const std::string& name);
    Player* get_player() const;

    // Party mechanics
    bool add_ally(const NPC& npc_template);
    void remove_ally(const std::string& id);
    
    const std::array<Entity*, 4>& get_party_slots() const { return party_slots; }
    
    // Slot management (0-3)
    bool swap_slots(int idx1, int idx2);
    void assign_to_empty_slot(Entity* ent);
};
