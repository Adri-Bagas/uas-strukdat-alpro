#include "DB.hpp"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "../utils/Logger.hpp"
#include "../utils/components/ErrorPopup.hpp"
#include "../enums/Element.hpp"
#include "../models/Condition.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

Condition parse_condition(const json& j) {
    Condition cond;
    if (j.is_null() || !j.contains("type")) return cond;

    std::string type_str = j["type"].get<std::string>();
    if (type_str == "var_equal") cond.type = ConditionType::VAR_EQUAL;
    else if (type_str == "var_greater_equal") cond.type = ConditionType::VAR_GREATER_EQUAL;
    else if (type_str == "var_less_equal") cond.type = ConditionType::VAR_LESS_EQUAL;
    else if (type_str == "has_item") cond.type = ConditionType::HAS_ITEM;
    else if (type_str == "quest_state") cond.type = ConditionType::QUEST_STATE;

    cond.key = j.value("key", "");
    cond.value = j.value("value", 0);
    cond.string_value = j.value("string_value", "");
    return cond;
}

DB::DB() {}

void DB::load_dialogs(const std::string& directory_path) {
    Logger::log("DB: Starting to load dialogs from " + directory_path);

    if (!fs::exists(directory_path)) {
        Logger::log("DB FATAL: Directory " + directory_path + " not found!");
        ErrorPopup err("Missing data folder: " + directory_path);
        err.show_fatal();
        return;
    }

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            Logger::log("DB: Loading file " + entry.path().string());
            std::ifstream file(entry.path());
            if (!file.is_open()) {
                Logger::log("DB ERROR: Could not open " + entry.path().string());
                continue;
            }

            try {
                json root;
                file >> root;

                auto parse_scene = [&](const json& j) {
                    DialogScene scene;
                    scene.id = j["id"].get<std::string>();

                    if (j.contains("condition")) {
                        scene.condition = parse_condition(j["condition"]);
                    }

                    if (j.contains("on_enter") && j["on_enter"].is_array()) {
                        for (const auto& item : j["on_enter"]) {
                            scene.on_enter.push_back(item.get<std::string>());
                        }
                    }

                    if (j.contains("on_exit") && j["on_exit"].is_array()) {
                        for (const auto& item : j["on_exit"]) {
                            scene.on_exit.push_back(item.get<std::string>());
                        }
                    }

                    scene.next_scene_id = j.value("next_scene", "");

                    for (const auto& node_j : j["nodes"]) {
                        DialogNode node;
                        node.type = node_j["type"].get<int>();
                        node.npc_name = node_j.value("npc_name", "");
                        node.value = node_j["value"].get<std::string>();
                        scene.nodes.push_back(node);
                    }

                    dialog_scenes[scene.id] = scene;
                    Logger::log("DB: Registered scene '" + scene.id + "'");
                };

                if (root.is_array()) {
                    for (const auto& scene_j : root) parse_scene(scene_j);
                } else {
                    parse_scene(root);
                }
            } catch (const std::exception& e) {
                Logger::log("DB ERROR: JSON parse failure in " + entry.path().string() + ": " + e.what());
            }
        }
    }
    Logger::log("DB: Load complete.");
}

void DB::load_places(const std::string& directory_path) {
    Logger::log("DB: Starting to load places from " + directory_path);

    if (!fs::exists(directory_path)) {
        Logger::log("DB FATAL: Directory " + directory_path + " not found!");
        ErrorPopup err("Missing data folder: " + directory_path);
        err.show_fatal();
        return;
    }

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            std::ifstream file(entry.path());
            if (!file.is_open()) continue;

            try {
                json j;
                file >> j;

                Place p(j["id"].get<std::string>(), j["name"].get<std::string>());
                p.set_on_first_enter(j.value("on_first_enter", ""));
                p.set_on_enter(j.value("on_enter", ""));

                for (const auto& w_id : j["walkable"]) {
                    p.add_walkable_id(w_id.get<std::string>());
                }

                for (const auto& act_j : j["activities"]) {
                    Activity act;
                    act.id = act_j["id"].get<std::string>();
                    act.name = act_j["name"].get<std::string>();
                    
                    if (act_j.contains("visible_condition")) {
                        act.visible_condition = parse_condition(act_j["visible_condition"]);
                    }

                    act.time_cost = act_j.value("time_cost", 1);

                    const auto& sched = act_j["schedule"];
                    if (sched.contains("days")) for (int d : sched["days"]) act.days.push_back(d);
                    if (sched.contains("phases")) for (const auto& ph : sched["phases"]) act.phases.push_back(ph.get<std::string>());

                    const auto& req = act_j["req"];
                    if (req.contains("stats")) {
                        for (auto& [stat, val] : req["stats"].items()) {
                            act.req_stats[stat] = val.get<int>();
                        }
                    }
                    act.is_locked = req.value("locked", false);

                    if (act_j.contains("dialogs")) {
                        const auto& d = act_j["dialogs"];
                        act.dialog_success = d.value("success", "");
                        act.dialog_fail = d.value("fail", "");
                        act.dialog_locked = d.value("locked", "");
                    }

                    if (act_j.contains("on_execute") && act_j["on_execute"].is_array()) {
                        for (const auto& item : act_j["on_execute"]) {
                            act.on_execute.push_back(item.get<std::string>());
                        }
                    } else if (act_j.contains("effect")) {
                        // Fallback/conversion for old effect object (optional, but good for safety)
                        const auto& eff = act_j["effect"];
                        if (eff.value("gold", 0) != 0) act.on_execute.push_back("add_gold " + std::to_string(eff.value("gold", 0)));
                        if (eff.value("str", 0) != 0) act.on_execute.push_back("add_str " + std::to_string(eff.value("str", 0)));
                        if (act.time_cost > 0) act.on_execute.push_back("advance_time " + std::to_string(act.time_cost));
                    }

                    p.add_activity(std::move(act));
                }

                places_db.emplace(p.get_id(), std::move(p));
                Logger::log("DB: Registered place '" + j["id"].get<std::string>() + "'");
            } catch (const std::exception& e) {
                Logger::log("DB ERROR: JSON parse failure in place " + entry.path().string() + ": " + e.what());
            }
        }
    }
}

const Place* DB::get_place(const std::string& id) const {
    auto it = places_db.find(id);
    if (it != places_db.end()) return &(it->second);
    return nullptr;
}

std::vector<const Place*> DB::get_all_places() const {
    std::vector<const Place*> all;
    for (auto& pair : places_db) all.push_back(&(pair.second));
    return all;
}

const DialogScene* DB::get_dialog_scene(const std::string& id) const {
    auto it = dialog_scenes.find(id);
    if (it != dialog_scenes.end()) return &(it->second);
    return nullptr;
}

// Memuat data item dari folder JSON
void DB::load_items(const std::string& directory_path) {
    Logger::log("DB: Starting to load items from " + directory_path);

    if (!fs::exists(directory_path)) {
        Logger::log("DB WARNING: Directory " + directory_path + " not found!");
        return;
    }

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            std::ifstream file(entry.path());
            if (!file.is_open()) continue;

            try {
                json j;
                file >> j;

                Item item;
                item.id = j["id"].get<std::string>();
                item.name = j["name"].get<std::string>();
                item.description = j.value("description", "");
                item.type = string_to_item_type(j.value("type", "misc"));
                item.value = j.value("value", 0);

                if (item.type == ItemType::EQUIPMENT) {
                    item.equip_slot = j.value("equip_slot", "");
                    if (j.contains("equip_stats")) {
                        for (auto& [stat_name, stat_val] : j["equip_stats"].items()) {
                            item.equip_stats[stat_name] = stat_val.get<int>();
                        }
                    }
                    if (j.contains("equip_affinity")) {
                        item.has_affinity_change = true;
                        item.equip_affinity = string_to_element(j["equip_affinity"].get<std::string>());
                    }
                }

                if (j.contains("on_use") && j["on_use"].is_array()) {
                    for (const auto& action : j["on_use"]) {
                        item.on_use.push_back(action.get<std::string>());
                    }
                }

                items_db.emplace(item.id, std::move(item));
                Logger::log("DB: Registered item '" + j["id"].get<std::string>() + "'");
            } catch (const std::exception& e) {
                Logger::log("DB ERROR: JSON parse failure in item " + entry.path().string() + ": " + e.what());
            }
        }
    }
}

// Mendapatkan item pointer berdasarkan ID
const Item* DB::get_item(const std::string& id) const {
    auto it = items_db.find(id);
    if (it != items_db.end()) return &(it->second);
    return nullptr;
}

void DB::load_npcs(const std::string& directory_path) {
    Logger::log("DB: Starting to load NPCs from " + directory_path);
    if (!fs::exists(directory_path)) return;

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            std::ifstream file(entry.path());
            if (!file.is_open()) continue;
            try {
                json j;
                file >> j;
                
                NPCType type = (j.value("type", "named") == "named") ? NPCType::NAMED : NPCType::UNNAMED;
                NPC npc(j["id"].get<std::string>(), j["name"].get<std::string>(), type, j.value("role", ""), j.value("faction", "Neutral"));
                npc.set_full_name(j.value("full_name", ""));
                
                if (j.contains("schedules") && j["schedules"].is_array()) {
                    for (auto& s_j : j["schedules"]) {
                        ScheduleEntry entry;
                        if (s_j.contains("days")) {
                            for (int d : s_j["days"]) entry.days.push_back(d);
                        }
                        entry.phase = s_j.value("phase", "Morning");
                        entry.location_id = s_j.value("location", "Unknown");
                        npc.add_schedule_entry(std::move(entry));
                    }
                } else if (j.contains("schedule")) {
                    // Backward compatibility for old format
                    for (auto& [phase, place_id] : j["schedule"].items()) {
                        ScheduleEntry entry;
                        entry.phase = phase;
                        entry.location_id = place_id.get<std::string>();
                        npc.add_schedule_entry(std::move(entry));
                    }
                }
                npc.set_default_dialog(j.value("default_dialog", ""));

                if (j.contains("quests") && j["quests"].is_array()) {
                    for (const auto& q_id : j["quests"]) {
                        npc.assign_quest_id(q_id.get<std::string>());
                    }
                }

                npcs_db.emplace(npc.get_id(), std::move(npc));
                Logger::log("DB: Registered NPC '" + j["id"].get<std::string>() + "'");
            } catch (const std::exception& e) {
                Logger::log("DB ERROR: JSON parse failure in NPC " + entry.path().string() + ": " + e.what());
            }
        }
    }
}

const NPC* DB::get_npc(const std::string& id) const {
    auto it = npcs_db.find(id);
    if (it != npcs_db.end()) return &(it->second);
    return nullptr;
}

std::vector<const NPC*> DB::get_all_npcs() const {
    std::vector<const NPC*> all;
    for (auto& pair : npcs_db) all.push_back(&(pair.second));
    return all;
}

void DB::load_monsters(const std::string& directory_path) {
    Logger::log("DB: Starting to load monsters from " + directory_path);
    if (!fs::exists(directory_path)) return;

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            std::ifstream file(entry.path());
            if (!file.is_open()) continue;
            try {
                json j;
                file >> j;
                
                Monster mon(j["id"].get<std::string>(), j["name"].get<std::string>(), j.value("description", ""), 
                            j.value("max_hp", 50), j.value("damage", 5), j.value("agility", 10));
                
                if (j.contains("loot") && j["loot"].is_array()) {
                    for (const auto& l_j : j["loot"]) {
                        mon.add_loot(l_j["item_id"].get<std::string>(), l_j["chance"].get<int>());
                    }
                }

                monsters_db.emplace(mon.get_id(), std::move(mon));
                Logger::log("DB: Registered monster '" + j["id"].get<std::string>() + "'");
            } catch (const std::exception& e) {
                Logger::log("DB ERROR: JSON parse failure in monster " + entry.path().string() + ": " + e.what());
            }
        }
    }
}

const Monster* DB::get_monster(const std::string& id) const {
    auto it = monsters_db.find(id);
    if (it != monsters_db.end()) return &(it->second);
    return nullptr;
}

std::vector<const Monster*> DB::get_all_monsters() const {
    std::vector<const Monster*> all;
    for (auto& pair : monsters_db) all.push_back(&(pair.second));
    return all;
}

void DB::load_quests(const std::string& directory_path) {
    Logger::log("DB: Starting to load quests from " + directory_path);
    if (!fs::exists(directory_path)) return;

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            std::ifstream file(entry.path());
            if (!file.is_open()) continue;
            try {
                json j;
                file >> j;

                Quest q(j["id"].get<std::string>(), j["name"].get<std::string>(), j.value("description", ""));
                q.set_target_npc(j.value("target_npc_id", ""));
                q.set_target_location(j.value("target_location", ""));
                q.set_start_scene(j.value("start_scene_id", ""));
                q.set_complete_scene(j.value("complete_scene_id", ""));

                if (j.contains("unlock_condition")) {
                    q.set_unlock_condition(parse_condition(j["unlock_condition"]));
                }

                if (j.contains("condition")) {
                    q.set_completion_condition(parse_condition(j["condition"]));
                } else if (j.contains("completion_condition")) {
                    q.set_completion_condition(parse_condition(j["completion_condition"]));
                }

                if (j.contains("on_complete") && j["on_complete"].is_array()) {
                    std::vector<std::string> effects;
                    for (const auto& effect : j["on_complete"]) {
                        effects.push_back(effect.get<std::string>());
                    }
                    q.set_on_complete(std::move(effects));
                }

                quests_db.emplace(q.get_id(), std::move(q));
                Logger::log("DB: Registered quest '" + j["id"].get<std::string>() + "'");
            } catch (const std::exception& e) {
                Logger::log("DB ERROR: JSON parse failure in quest " + entry.path().string() + ": " + e.what());
            }
        }
    }
}

const Quest* DB::get_quest(const std::string& id) const {
    auto it = quests_db.find(id);
    if (it != quests_db.end()) return &(it->second);
    return nullptr;
}

std::vector<const Quest*> DB::get_all_quests() const {
    std::vector<const Quest*> all;
    for (auto& pair : quests_db) all.push_back(&(pair.second));
    return all;
}