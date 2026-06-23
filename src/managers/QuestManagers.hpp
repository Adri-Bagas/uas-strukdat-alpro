#pragma once
#include <string>
#include <unordered_map>
#include "../models/Quest.hpp"
#include "../models/NPC.hpp"

class QuestManager {
private:
    std::unordered_map<std::string, Quest*> quests;

public:
    void add_quest(Quest* q);
    Quest* get_quest(const std::string& id);
    std::vector<Quest*> get_available_quests_for_npc(const std::string& npc_id);
    std::unordered_map<std::string, Quest*>& get_all_quests();
    void check_npc_quests(const NPC* npc, const Player* player);
    std::string evaluate_endings(Player* player);
};
