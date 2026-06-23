#include "SaveManager.hpp"
#include "../GameEngine.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

bool SaveManager::save_game(GameEngine* engine, const std::string& filename) {
    fs::create_directories("saves");
    std::string filepath = "saves/" + filename;
    if (filepath.find(".json") == std::string::npos) {
        filepath += ".json";
    }

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

        data["player"]["stat_points"] = p->get_stat_points();
        
        auto& magics_data = data["player"]["magics"];
        for (const auto& m : p->get_magics()) {
            json mj;
            mj["id"] = m.id;
            mj["name"] = m.name;
            mj["type"] = static_cast<int>(m.type);
            mj["mana_cost"] = m.mana_cost;
            mj["power"] = m.power;
            mj["elem"] = static_cast<int>(m.elem);
            mj["range"] = static_cast<int>(m.range);
            auto& mods_data = mj["modifiers"];
            for (const auto& mod : m.modifiers) {
                json modj;
                modj["stat_name"] = mod.stat_name;
                modj["amount"] = mod.amount;
                modj["duration_turns"] = mod.duration_turns;
                mods_data.push_back(modj);
            }
            magics_data.push_back(mj);
        }

        if (p->has_special()) {
            auto& sm = p->get_special_move();
            json smj;
            smj["id"] = sm.id;
            smj["name"] = sm.name;
            smj["max_uses_per_day"] = sm.max_uses_per_day;
            smj["current_uses"] = sm.current_uses;
            smj["power"] = sm.power;
            smj["elem"] = static_cast<int>(sm.elem);
            smj["range"] = static_cast<int>(sm.range);
            auto& mods_data = smj["modifiers"];
            for (const auto& mod : sm.modifiers) {
                json modj;
                modj["stat_name"] = mod.stat_name;
                modj["amount"] = mod.amount;
                modj["duration_turns"] = mod.duration_turns;
                mods_data.push_back(modj);
            }
            data["player"]["special_move"] = smj;
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

    // 6. Places
    data["places"] = json::object();
    for (const auto& place_ptr : engine->get_db().get_all_places()) {
        data["places"][place_ptr->get_id()]["has_entered"] = place_ptr->get_has_entered();
    }

    std::ofstream file(filepath);
    if (file.is_open()) {
        file << data.dump(4);
        return true;
    }
    return false;
}

bool SaveManager::load_game(GameEngine* engine, const std::string& filename) {
    std::string filepath = "saves/" + filename;
    if (filepath.find(".json") == std::string::npos) {
        filepath += ".json";
    }

    std::ifstream file(filepath);
    if (!file.is_open()) {
        // Fallback to root for backward compatibility with older savegame.json
        if (filename == "savegame.json" && fs::exists("savegame.json")) {
            filepath = "savegame.json";
            file.open(filepath);
            if (!file.is_open()) return false;
        } else {
            return false;
        }
    }

    json data;
    try {
        file >> data;
    } catch (...) {
        return false;
    }

    try {
        // Calendar
        if (data.contains("calendar") && data["calendar"].is_object()) {
            if (data["calendar"].contains("day")) {
                engine->get_calendar().setDay(data["calendar"]["day"]);
            }
            if (data["calendar"].contains("phase")) {
                engine->get_calendar().setDayTime(static_cast<DayTime>(data["calendar"]["phase"].get<int>()));
            }
        }

        // Quests
        if (data.contains("quests") && data["quests"].is_object()) {
            for (auto& el : data["quests"].items()) {
                Quest* q = engine->get_quests().get_quest(el.key());
                if (q) q->set_state(static_cast<QuestState>(el.value().get<int>()));
            }
        }

        // NPCs
        if (data.contains("npcs") && data["npcs"].is_object()) {
            for (auto& el : data["npcs"].items()) {
                const NPC* npc = engine->get_db().get_npc(el.key());
                if (npc) {
                    if (el.value().is_object() && el.value().contains("has_met") && el.value()["has_met"].get<bool>()) {
                        const_cast<NPC*>(npc)->reveal();
                    }
                }
            }
        }

        // Player
        if (data.contains("player") && data["player"].is_object()) {
            engine->get_player_manager().init_player(data["player"].value("id", "hero"), data["player"].value("name", "Nirva Hero"));
            Player* p = engine->get_player_manager().get_player();
            
            p->set_level(data["player"].value("level", 1));
            int current_exp = p->get_exp();
            p->add_exp(data["player"].value("exp", 0) - current_exp);
            p->add_gold(data["player"].value("gold", 0) - p->get_gold());
            
            p->set_str(data["player"].value("str", 15));
            p->set_cons(data["player"].value("cons", 15));
            p->set_agi(data["player"].value("agi", 15));
            p->set_intl(data["player"].value("intl", 15));
            p->set_wis(data["player"].value("wis", 15));
            
            p->set_max_hp(data["player"].value("max_hp", 150));
            p->set_max_mp(data["player"].value("max_mp", 75));
            
            p->take_damage(p->get_hp()); // Reset to 0
            p->heal_hp(data["player"].value("hp", 150));
            
            p->consume_mp(p->get_mp()); // Reset to 0
            p->restore_mp(data["player"].value("mp", 75));

            p->set_affinity(static_cast<Element>(data["player"].value("affinity", 0)));
            p->set_weakness(static_cast<Element>(data["player"].value("weakness", 0)));

            if (data["player"].contains("vars") && data["player"]["vars"].is_object()) {
                for (auto& el : data["player"]["vars"].items()) {
                    p->set_var(el.key(), el.value().get<int>());
                }
            }

            std::vector<std::string> to_remove;
            for (const auto& [item_id, count] : p->get_inventory()) to_remove.push_back(item_id);
            for (const auto& item_id : to_remove) p->remove_item(item_id, p->get_item_count(item_id));

            if (data["player"].contains("inventory") && data["player"]["inventory"].is_object()) {
                for (auto& el : data["player"]["inventory"].items()) {
                    p->add_item(el.key(), el.value().get<int>());
                }
            }

            if (data["player"].contains("equipment") && data["player"]["equipment"].is_object()) {
                for (auto& el : data["player"]["equipment"].items()) {
                    if (el.value().is_string()) {
                        const Item* item = engine->get_db().get_item(el.value().get<std::string>());
                        if (item) p->equip(item);
                    }
                }
            }

            if (data["player"].contains("stat_points")) {
                p->stat_points = data["player"]["stat_points"].get<int>();
            }

            if (data["player"].contains("magics") && data["player"]["magics"].is_array()) {
                p->clear_magics();
                for (const auto& mj : data["player"]["magics"]) {
                    if (!mj.is_object()) continue;
                    Magic m;
                    m.id = mj.value("id", "");
                    m.name = mj.value("name", "");
                    m.type = static_cast<MagicType>(mj.value("type", 0));
                    m.mana_cost = mj.value("mana_cost", 0);
                    m.power = mj.value("power", 0);
                    m.elem = static_cast<Element>(mj.value("elem", 0));
                    m.range = static_cast<TargetRange>(mj.value("range", 0));
                    if (mj.contains("modifiers") && mj["modifiers"].is_array()) {
                        for (const auto& modj : mj["modifiers"]) {
                            if (!modj.is_object()) continue;
                            CombatModifier mod;
                            mod.stat_name = modj.value("stat_name", "");
                            mod.amount = modj.value("amount", 0);
                            mod.duration_turns = modj.value("duration_turns", 0);
                            m.modifiers.push_back(mod);
                        }
                    }
                    p->add_magic(m);
                }
            }

            if (data["player"].contains("special_move") && data["player"]["special_move"].is_object()) {
                const auto& smj = data["player"]["special_move"];
                SpecialMove sm;
                sm.id = smj.value("id", "");
                sm.name = smj.value("name", "");
                sm.max_uses_per_day = smj.value("max_uses_per_day", 1);
                sm.current_uses = smj.value("current_uses", 0);
                sm.power = smj.value("power", 0);
                sm.elem = static_cast<Element>(smj.value("elem", 0));
                sm.range = static_cast<TargetRange>(smj.value("range", 0));
                if (smj.contains("modifiers") && smj["modifiers"].is_array()) {
                    for (const auto& modj : smj["modifiers"]) {
                        if (!modj.is_object()) continue;
                        CombatModifier mod;
                        mod.stat_name = modj.value("stat_name", "");
                        mod.amount = modj.value("amount", 0);
                        mod.duration_turns = modj.value("duration_turns", 0);
                        sm.modifiers.push_back(mod);
                    }
                }
                p->set_special_move(sm);
            } else {
                p->clear_special_move();
            }
        }

        // Party
        if (data.contains("party") && data["party"].is_array()) {
            for (auto& el : data["party"].items()) {
                if (el.value().is_string()) {
                    const NPC* npc = engine->get_db().get_npc(el.value().get<std::string>());
                    if (npc) {
                        engine->get_player_manager().add_ally(*npc);
                    }
                }
            }
        }

        // Places
        if (data.contains("places") && data["places"].is_object()) {
            for (auto& el : data["places"].items()) {
                std::string place_id = el.key();
                const Place* place = engine->get_db().get_place(place_id);
                if (place && el.value().is_object() && el.value().contains("has_entered")) {
                    bool entered = el.value()["has_entered"].get<bool>();
                    const_cast<Place*>(place)->set_has_entered(entered);
                }
            }
        }
    } catch (...) {
        return false;
    }

    return true;
}

std::vector<SaveFileInfo> SaveManager::get_save_files() {
    std::vector<SaveFileInfo> files;
    
    // Add old savegame.json if it exists
    if (fs::exists("savegame.json")) {
        try {
            std::ifstream file("savegame.json");
            if (file.is_open()) {
                json data;
                file >> data;
                SaveFileInfo info;
                info.filename = "savegame.json";
                
                if (data.contains("player") && data["player"].is_object()) {
                    info.player_name = data["player"].value("name", "Unknown");
                    info.level = data["player"].value("level", 1);
                } else {
                    info.player_name = "Unknown";
                    info.level = 1;
                }
                
                if (data.contains("calendar") && data["calendar"].is_object()) {
                    info.day = data["calendar"].value("day", 1);
                    int phase = data["calendar"].value("phase", 0);
                    info.phase_name = (phase == 0) ? "Pagi" : (phase == 1) ? "Siang" : "Malam";
                } else {
                    info.day = 1;
                    info.phase_name = "Pagi";
                }
                files.push_back(info);
            }
        } catch (...) {}
    }

    try {
        if (fs::exists("saves") && fs::is_directory("saves")) {
            for (const auto& entry : fs::directory_iterator("saves")) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    try {
                        std::ifstream file(entry.path());
                        if (!file.is_open()) continue;
                        
                        json data;
                        file >> data;
                        
                        SaveFileInfo info;
                        info.filename = entry.path().filename().string();
                        
                        if (data.contains("player") && data["player"].is_object()) {
                            info.player_name = data["player"].value("name", "Unknown");
                            info.level = data["player"].value("level", 1);
                        } else {
                            info.player_name = "Unknown";
                            info.level = 1;
                        }
                        
                        if (data.contains("calendar") && data["calendar"].is_object()) {
                            info.day = data["calendar"].value("day", 1);
                            int phase = data["calendar"].value("phase", 0);
                            info.phase_name = (phase == 0) ? "Pagi" : (phase == 1) ? "Siang" : "Malam";
                        } else {
                            info.day = 1;
                            info.phase_name = "Pagi";
                        }
                        
                        files.push_back(info);
                    } catch (...) {
                        // Ignore corrupt/invalid saves in listing
                    }
                }
            }
        }
    } catch (...) {
        // Ignore directory reading errors
    }

    return files;
}
