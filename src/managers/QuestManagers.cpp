#include "QuestManagers.hpp"

void QuestManager::add_quest(Quest* q) {
    if (q) {
        quests[q->get_id()] = q;
    }
}

Quest* QuestManager::get_quest(const std::string& id) {
    auto it = quests.find(id);
    if (it != quests.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<Quest*> QuestManager::get_available_quests_for_npc(const std::string& npc_id) {
    std::vector<Quest*> found;
    for (auto& pair : quests) {
        Quest* q = pair.second;
        if (q && q->get_target_npc_id() == npc_id && 
           (q->get_state() == QuestState::AVAILABLE || q->get_state() == QuestState::IN_PROGRESS)) {
            found.push_back(q);
        }
    }
    return found;
}

std::unordered_map<std::string, Quest*>& QuestManager::get_all_quests() {
    return quests;
}

void QuestManager::check_npc_quests(const NPC* npc, const Player* player) {
    if (!npc || !player) return;
    for (const auto& q_id : npc->get_quests()) {
        Quest* q = get_quest(q_id);
        if (q) {
            q->try_unlock(player);
        }
    }
}
