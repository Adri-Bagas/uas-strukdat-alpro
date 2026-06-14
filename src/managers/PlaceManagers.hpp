#pragma once
#include "../models/Place.hpp"
#include "../models/NPC.hpp"
#include "../utils/components/Popup.hpp"
#include "TimeCalendarManagers.hpp"
#include "QuestManagers.hpp"
#include <vector>
#include <string>

class PlaceManagers {
private:
    std::vector<Place*> all_places;
    std::vector<NPC*> npc_list;
    Place* current_place = nullptr;

public:
    PlaceManagers() = default;
    ~PlaceManagers() = default;

    void add_place(Place* p) {
        all_places.push_back(p);
    }

    void register_npc(NPC* npc) {
        npc_list.push_back(npc);
    }

    Place* get_place(const std::string& id) {
        for (auto* p : all_places) {
            if (p->get_id() == id) return p;
        }
        return nullptr;
    }

    void set_current_place(const std::string& id) {
        Place* target = get_place(id);
        if (target && target != current_place) {
            current_place = target;
            Popup p {"Tiba di: " + current_place->get_name()};
            p.animate();
            p.type_text();
        }
    }

    Place* get_current_place() const {
        return current_place;
    }

    void resolve_connections() {
        for (auto* p : all_places) {
            for (const std::string& target_id : p->get_walkable_ids()) {
                Place* target = get_place(target_id);
                if (target) {
                    p->add_walkable_place(target);
                }
            }
        }
    }

    bool travel(const std::string& destination_id) {
        if (!current_place) return false;

        for (auto* p : current_place->get_walkable_places()) {
            if (p->get_id() == destination_id) {
                current_place = p;
                Popup pop {"Bepergian ke: " + current_place->get_name()};
                pop.animate();
                pop.type_text();
                return true;
            }
        }
        return false;
    }

    void update_npc_locations(TimeCalendarManagers& calendar, QuestManager& quest_manager) {
        // 1. Clear all places
        for (auto* p : all_places) {
            p->clear_npcs();
        }

        std::string current_phase = calendar.getTimeString();
        int current_day = calendar.getDay();

        // 2. Distribute NPCs
        for (auto* npc : npc_list) {
            // A. Scheduled Location
            std::string scheduled_loc_id = npc->get_location(current_day, current_phase);
            Place* sched_place = get_place(scheduled_loc_id);
            if (sched_place) {
                sched_place->add_npc(npc);
            }

            // B. Quest Location (Dual-Presence)
            for (auto& pair : quest_manager.get_all_quests()) {
                Quest* q = pair.second;
                if (q && (q->get_state() == QuestState::AVAILABLE || q->get_state() == QuestState::IN_PROGRESS) &&
                    q->get_target_npc_id() == npc->get_id()) {
                    
                    Place* quest_place = get_place(q->get_target_location());
                    if (quest_place) {
                        quest_place->add_npc(npc);
                    }
                }
            }
        }
    }
};
