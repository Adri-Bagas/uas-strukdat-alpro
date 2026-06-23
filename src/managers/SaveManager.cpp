#include "SaveManager.hpp"
#include "../GameEngine.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool SaveManager::save_game(GameEngine* engine, const std::string& filepath) {
    json data;
    
    // 1. Calendar
    data["calendar"]["day"] = engine->get_calendar().getDay();
    data["calendar"]["phase"] = static_cast<int>(engine->get_calendar().getDayTime());

    // 2. Player
    Player* p = engine->get_player_manager().get_player();
    if (p) {
        data["player"]["id"] = p->get_id();
        data["player"]["name"] = p->get_name();
        data["player"]["level"] = p->get_level();
        data["player"]["exp"] = p->get_exp();
        data["player"]["gold"] = p->get_gold();
        data["player"]["hp"] = p->get_hp();
        data["player"]["mp"] = p->get_mp();
        data["player"]["max_hp"] = p->get_max_hp();
        data["player"]["max_mp"] = p->get_max_mp();
        data["player"]["str"] = p->Entity::get_str();
        data["player"]["cons"] = p->Entity::get_cons();
        data["player"]["agi"] = p->Entity::get_agi();
        data["player"]["intl"] = p->Entity::get_intl();
        data["player"]["wis"] = p->Entity::get_wis();
        
        data["player"]["affinity"] = static_cast<int>(p->get_affinity());
        data["player"]["weakness"] = static_cast<int>(p->get_weakness());

        // Inventory
        for (const auto& [item_id, count] : p->get_inventory()) {
            data["player"]["inventory"][item_id] = count;
        }

        // Equipment
        for (const auto& [slot, item] : p->get_all_equipped()) {
            if (item) data["player"]["equipment"][slot] = item->id;
        }

        // Vars
        for (const auto& [key, val] : p->get_all_vars()) {
            data["player"]["vars"][key] = val;
        }
    }

    // 3. Quests
    for (const auto& [id, q] : engine->get_quests().get_all_quests()) {
        data["quests"][id] = static_cast<int>(q->get_state());
    }

    // 4. NPCs (has_met)
    for (const auto& npc : engine->get_db().get_all_npcs()) {
        data["npcs"][npc->get_id()]["has_met"] = npc->has_met();
    }

    // 5. Party
    auto party_slots = engine->get_player_manager().get_party_slots();
    for (int i = 0; i < 4; ++i) {
        if (party_slots[i] && party_slots[i] != p) {
            data["party"][std::to_string(i)] = party_slots[i]->get_id();
        }
    }

    std::ofstream file(filepath);
    if (file.is_open()) {
        file << data.dump(4);
        return true;
    }
    return false;
}

bool SaveManager::load_game(GameEngine* engine, const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    json data;
    try {
        file >> data;
    } catch (...) {
        return false;
    }

    // Calendar
    if (data.contains("calendar")) {
        engine->get_calendar().setDay(data["calendar"]["day"]);
        engine->get_calendar().setDayTime(static_cast<DayTime>(data["calendar"]["phase"].get<int>()));
    }

    // Quests
    if (data.contains("quests")) {
        for (auto& el : data["quests"].items()) {
            Quest* q = engine->get_quests().get_quest(el.key());
            if (q) q->set_state(static_cast<QuestState>(el.value().get<int>()));
        }
    }

    // NPCs
    if (data.contains("npcs")) {
        for (auto& el : data["npcs"].items()) {
            const NPC* npc = engine->get_db().get_npc(el.key());
            if (npc) {
                if (el.value()["has_met"].get<bool>()) {
                    const_cast<NPC*>(npc)->reveal();
                }
            }
        }
    }

    // Player
    if (data.contains("player")) {
        engine->get_player_manager().init_player(data["player"]["id"], data["player"]["name"]);
        Player* p = engine->get_player_manager().get_player();
        
        p->set_level(data["player"]["level"]);
        int current_exp = p->get_exp();
        p->add_exp(data["player"]["exp"].get<int>() - current_exp);
        p->add_gold(data["player"]["gold"].get<int>() - p->get_gold());
        
        p->set_str(data["player"]["str"]);
        p->set_cons(data["player"]["cons"]);
        p->set_agi(data["player"]["agi"]);
        p->set_intl(data["player"]["intl"]);
        p->set_wis(data["player"]["wis"]);
        
        p->set_max_hp(data["player"]["max_hp"]);
        p->set_max_mp(data["player"]["max_mp"]);
        
        p->take_damage(p->get_hp()); // Reset to 0
        p->heal_hp(data["player"]["hp"]);
        
        p->consume_mp(p->get_mp()); // Reset to 0
        p->restore_mp(data["player"]["mp"]);

        p->set_affinity(static_cast<Element>(data["player"]["affinity"].get<int>()));
        p->set_weakness(static_cast<Element>(data["player"]["weakness"].get<int>()));

        if (data["player"].contains("vars")) {
            for (auto& el : data["player"]["vars"].items()) {
                p->set_var(el.key(), el.value().get<int>());
            }
        }

        std::vector<std::string> to_remove;
        for (const auto& [item_id, count] : p->get_inventory()) to_remove.push_back(item_id);
        for (const auto& item_id : to_remove) p->remove_item(item_id, p->get_item_count(item_id));

        if (data["player"].contains("inventory")) {
            for (auto& el : data["player"]["inventory"].items()) {
                p->add_item(el.key(), el.value().get<int>());
            }
        }

        if (data["player"].contains("equipment")) {
            for (auto& el : data["player"]["equipment"].items()) {
                const Item* item = engine->get_db().get_item(el.value().get<std::string>());
                if (item) p->equip(item);
            }
        }
    }

    // Party
    if (data.contains("party")) {
        for (auto& el : data["party"].items()) {
            const NPC* npc = engine->get_db().get_npc(el.value().get<std::string>());
            if (npc) {
                engine->get_player_manager().add_ally(*npc);
            }
        }
    }

    return true;
}
