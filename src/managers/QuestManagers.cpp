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

std::unordered_map<std::string, Quest*>& QuestManager::get_all_quests() {
    return quests;
}

void QuestManager::check_npc_quests(const NPC* npc) {
    if (!npc) return;
    for (const auto& q_id : npc->get_quests()) {
        Quest* q = get_quest(q_id);
        if (q) {
            q->try_unlock();
            // If AVAILABLE, show in UI dialog options.
            // If IN_PROGRESS, show current objective dialog.
        }
    }
}
