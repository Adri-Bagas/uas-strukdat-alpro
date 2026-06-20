#include "PlayerManager.hpp"

void PlayerManager::init_player(const std::string& id, const std::string& name) {
    player = std::make_unique<Player>(id, name);
    player->add_item("ransum", 1);

    // Initialize slots to nullptr
    for (int i = 0; i < 4; ++i) {
        party_slots[i] = nullptr;
    }
    // Player always occupies slot 0 by default
    party_slots[0] = player.get();
}

Player* PlayerManager::get_player() const {
    return player.get();
}

bool PlayerManager::add_ally(const NPC& npc_template) {
    if (npc_allies.size() >= 3) {
        return false; // Party full (1 player + 3 NPCs max)
    }
    npc_allies.push_back(std::make_unique<NPC>(npc_template));
    assign_to_empty_slot(npc_allies.back().get());
    return true;
}

void PlayerManager::remove_ally(const std::string& id) {
    for (size_t i = 0; i < npc_allies.size(); ++i) {
        if (npc_allies[i]->get_id() == id) {
            // Remove from slot
            for (int s = 0; s < 4; ++s) {
                if (party_slots[s] == npc_allies[i].get()) {
                    party_slots[s] = nullptr;
                }
            }
            npc_allies.erase(npc_allies.begin() + i);
            break;
        }
    }
}

bool PlayerManager::swap_slots(int idx1, int idx2) {
    if (idx1 < 0 || idx1 >= 4 || idx2 < 0 || idx2 >= 4) return false;
    std::swap(party_slots[idx1], party_slots[idx2]);
    return true;
}

void PlayerManager::assign_to_empty_slot(Entity* ent) {
    for (int i = 0; i < 4; ++i) {
        if (party_slots[i] == nullptr) {
            party_slots[i] = ent;
            return;
        }
    }
}
