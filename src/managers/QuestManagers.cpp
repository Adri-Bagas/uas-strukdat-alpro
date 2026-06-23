#include "QuestManagers.hpp"
#include "../models/Player.hpp"

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
        if (q) {
            bool matches = false;
            if (q->get_state() == QuestState::AVAILABLE && q->get_source_npc_id() == npc_id) {
                matches = true;
            } else if ((q->get_state() == QuestState::IN_PROGRESS || q->get_state() == QuestState::READY_TO_TURN_IN) && q->get_target_npc_id() == npc_id) {
                matches = true;
            }
            if (matches) found.push_back(q);
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

std::string QuestManager::evaluate_endings(Player* player) {
    if (!player) return "ending_bad_inquisitor";
    int silas = player->get_var("silas_intel_secured");
    int q4_complete = player->get_var("quest_gereja_4_complete");
    int q4_completed = player->get_var("quest_gereja_4_completed");
    
    Quest* q4 = get_quest("quest_gereja_4");
    bool is_q4_done = (q4_complete == 1) || (q4_completed == 1) || (q4 && q4->get_state() == QuestState::COMPLETED);

    if (silas == 1 && is_q4_done) {
        return "ending_good_exonerated";
    }
    return "ending_bad_inquisitor";
}
