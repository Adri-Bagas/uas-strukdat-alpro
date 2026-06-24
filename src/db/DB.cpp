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

MagicType parse_magic_type(const std::string& str) {
    if (str == "healing") return MagicType::HEALING;
    if (str == "support") return MagicType::SUPPORT;
    return MagicType::ATTACKING;
}

TargetRange parse_target_range(const std::string& range_str) {
    if (range_str == "direct") return TargetRange::DIRECT;
    if (range_str == "aoe") return TargetRange::AOE;
    return TargetRange::REACH; // default
}

void parse_magics_and_special(const json& j, Entity& entity) {
    if (j.contains("magics") && j["magics"].is_array()) {
        for (const auto& m_j : j["magics"]) {
            Magic m;
            m.id = m_j.value("id", "magic_0");
            m.name = m_j.value("name", "Unknown Magic");
            m.type = parse_magic_type(m_j.value("type", "attacking"));
            m.mana_cost = m_j.value("mana_cost", 10);
            m.power = m_j.value("power", 10);
            m.elem = string_to_element(m_j.value("element", "none"));
            m.range = parse_target_range(m_j.value("range", "reach"));
            entity.add_magic(m);
        }
    }
    
    if (j.contains("special_move")) {
        auto s_j = j["special_move"];
        SpecialMove sm;
        sm.id = s_j.value("id", "sm_0");
        sm.name = s_j.value("name", "Unknown Special");
        sm.max_uses_per_day = s_j.value("max_uses_per_day", 1);
        sm.current_uses = 0;
        sm.power = s_j.value("power", 50);
        sm.elem = string_to_element(s_j.value("element", "none"));
        sm.range = parse_target_range(s_j.value("range", "aoe"));
        entity.set_special_move(sm);
    }
}

/**
 * Fungsi pembantu untuk memparsing objek "condition" dari JSON.
 * Digunakan oleh Dialog, Quest, dan Activity untuk menentukan apakah sesuatu bisa muncul/selesai.
 */
Condition parse_condition(const json& j) {
    Condition cond;
    // Jika data kosong atau tidak punya tipe, anggap tidak ada syarat (NONE)
    if (j.is_null() || !j.contains("type")) return cond;

    std::string type_str = j["type"].get<std::string>();
    
    // Memetakan string dari JSON ke enum C++
    if (type_str == "var_equal") cond.type = ConditionType::VAR_EQUAL;
    else if (type_str == "var_greater_equal") cond.type = ConditionType::VAR_GREATER_EQUAL;
    else if (type_str == "var_less_equal") cond.type = ConditionType::VAR_LESS_EQUAL;
    else if (type_str == "has_item") cond.type = ConditionType::HAS_ITEM;
    else if (type_str == "quest_state") cond.type = ConditionType::QUEST_STATE;
    else if (type_str == "killed_monster") cond.type = ConditionType::KILLED_MONSTER;
    else if (type_str == "explored_area") cond.type = ConditionType::EXPLORED_AREA;
    else if (type_str == "reach_location") cond.type = ConditionType::REACH_LOCATION;
    else if (type_str == "has_party_member") cond.type = ConditionType::HAS_PARTY_MEMBER;

    // Mengambil data tambahan seperti nama variabel (key) atau nilai target (value)
    cond.key = j.value("key", "");
    cond.value = j.value("value", 0);
    cond.string_value = j.value("string_value", "");
    return cond;
}

DB::DB() {}

/**
 * Memuat semua file dialog dari folder tertentu.
 * Mendukung format satu file berisi satu scene ({}) atau satu file berisi banyak scene ([]).
 */
void DB::load_dialogs(const std::string& directory_path) {
    Utils::Logger::log("DB: Mulai memuat dialog dari " + directory_path);

    if (!fs::exists(directory_path)) {
        Utils::Logger::log("DB FATAL: Direktori " + directory_path + " tidak ditemukan!");
        Utils::ErrorPopup err("Folder data hilang: " + directory_path);
        err.show_fatal();
        return;
    }

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            Utils::Logger::log("DB: Membaca file " + entry.path().string());
            std::ifstream file(entry.path());
            if (!file.is_open()) {
                Utils::Logger::log("DB ERROR: Tidak bisa membuka " + entry.path().string());
                continue;
            }

            try {
                json root;
                file >> root;

                // Lambda function untuk memproses satu objek DialogScene
                auto parse_scene = [&](const json& scene_j) {
                    DialogScene scene;
                    scene.id = scene_j["id"].get<std::string>();

                    // Parsing syarat munculnya dialog (opsional)
                    if (scene_j.contains("condition")) {
                        scene.condition = parse_condition(scene_j["condition"]);
                    }

                    // Parsing aksi saat dialog dimulai (on_start / on_enter)
                    std::vector<std::string> on_start_list;
                    std::string key_start = scene_j.contains("on_enter") ? "on_enter" : "on_start";
                    if (scene_j.contains(key_start)) {
                        const auto& start_val = scene_j[key_start];
                        if (start_val.is_array()) {
                            for (const auto& val : start_val) {
                                if (val.is_string()) on_start_list.push_back(val.get<std::string>());
                            }
                        } else if (start_val.is_string() && !start_val.get<std::string>().empty()) {
                            on_start_list.push_back(start_val.get<std::string>());
                        }
                    }
                    scene.on_start = on_start_list;

                    // Parsing aksi saat dialog selesai
                    std::vector<std::string> on_exit_list;
                    if (scene_j.contains("on_exit")) {
                        const auto& exit_val = scene_j["on_exit"];
                        if (exit_val.is_array()) {
                            for (const auto& val : exit_val) {
                                if (val.is_string()) on_exit_list.push_back(val.get<std::string>());
                            }
                        } else if (exit_val.is_string() && !exit_val.get<std::string>().empty()) {
                            on_exit_list.push_back(exit_val.get<std::string>());
                        }
                    }
                    scene.on_exit = on_exit_list;

                    // ID scene berikutnya jika ada (rantaian otomatis)
                    scene.next_scene_id = scene_j.value("next_scene", "");

                    // Parsing isi percakapan (nodes)
                    if (scene_j.contains("nodes")) {
                        for (const auto& node_j : scene_j["nodes"]) {
                            DialogNode node;
                            node.type = node_j.value("type", 1); // Default to dialog
                            node.npc_name = node_j.value("npc_name", "");
                            node.value = node_j.value("value", "");
                            if (node.value.empty() && node_j.contains("text")) {
                                node.value = node_j["text"].get<std::string>();
                            }
                            scene.nodes.push_back(node);
                        }
                    } else if (scene_j.contains("text")) {
                        DialogNode node;
                        node.type = 1; // Dialog
                        node.npc_name = scene_j.value("speaker", "");
                        node.value = scene_j["text"].get<std::string>();
                        scene.nodes.push_back(node);
                    }


                    // Parsing pilihan jawaban di akhir dialog (branching)
                    if (scene_j.contains("choices")) {
                        for (const auto& choice_j : scene_j["choices"]) {
                            DialogChoice choice;
                            choice.text = choice_j["text"].get<std::string>();
                            if (choice_j.contains("condition")) {
                                choice.condition = parse_condition(choice_j["condition"]);
                            }
                            choice.next_scene = choice_j.value("next_scene", ""); // Scene tujuan jika dipilih
                            
                            if (choice_j.contains("on_select")) {
                                for (const auto& act : choice_j["on_select"]) {
                                    choice.on_select.push_back(act.get<std::string>());
                                }
                            }
                            // Backwards compatibility if user used on_exit in choices:
                            if (choice_j.contains("on_exit")) {
                                for (const auto& act : choice_j["on_exit"]) {
                                    choice.on_select.push_back(act.get<std::string>());
                                }
                            }

                            scene.choices.push_back(choice);
                        }
                    }

                    return scene;
                };

                // Jika file berisi array [], proses tiap elemennya
                if (root.is_array()) {
                    for (const auto& scene_j : root) {
                        DialogScene scene = parse_scene(scene_j);
                        dialog_scenes[scene.id] = scene;
                        Utils::Logger::log("DB: Terdaftar scene '" + scene.id + "' dari array");
                    }
                } else {
                    // Jika hanya satu objek {}, proses langsung
                    DialogScene scene = parse_scene(root);
                    dialog_scenes[scene.id] = scene;
                    Utils::Logger::log("DB: Terdaftar scene '" + scene.id + "'");
                }
            } catch (const std::exception& e) {
                Utils::Logger::log("DB ERROR: Gagal parse JSON di " + entry.path().string() + ": " + e.what());
            }
        }
    }
    Utils::Logger::log("DB: Memuat dialog selesai.");
}

/**
 * Mengambil data scene dialog berdasarkan ID.
 */
const DialogScene* DB::get_dialog_scene(const std::string& id) const {
    auto it = dialog_scenes.find(id);
    if (it != dialog_scenes.end()) return &(it->second);
    return nullptr;
}

/**
 * Memuat semua lokasi (places) dari folder JSON.
 * Lokasi berisi koneksi antar wilayah (walkable) dan kegiatan (activities).
 */
void DB::load_places(const std::string& directory_path) {
    Utils::Logger::log("DB: Mulai memuat lokasi dari " + directory_path);

    if (!fs::exists(directory_path)) {
        Utils::Logger::log("DB FATAL: Direktori " + directory_path + " tidak ditemukan!");
        Utils::ErrorPopup err("Folder data hilang: " + directory_path);
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

                // Memuat daftar ID lokasi yang bisa dikunjungi dari sini
                if (j.contains("walkable") && j["walkable"].is_object()) {
                    for (auto& [dir, target_id] : j["walkable"].items()) {
                        p.add_walkable_id(dir, target_id.get<std::string>());
                    }
                }

                // Memuat semua kegiatan yang bisa dilakukan di lokasi ini
                if (j.contains("activities") && j["activities"].is_array()) {
                    for (const auto& act_j : j["activities"]) {
                        Activity act;
                        act.id = act_j["id"].get<std::string>();
                        act.name = act_j["name"].get<std::string>();
                        
                        // Syarat apakah menu kegiatan ini muncul atau tidak
                        if (act_j.contains("visible_condition")) {
                            act.visible_condition = parse_condition(act_j["visible_condition"]);
                        }

                        act.time_cost = act_j.value("time_cost", 1);

                        // Jadwal kegiatan (hari apa saja / fase waktu apa saja)
                        const auto& sched = act_j["schedule"];
                        if (sched.contains("days")) for (int d : sched["days"]) act.days.push_back(d);
                        if (sched.contains("phases")) for (const auto& ph : sched["phases"]) act.phases.push_back(ph.get<std::string>());

                        // Syarat statistik untuk melakukan kegiatan (misal butuh STR 5)
                        if (act_j.contains("req")) {
                            const auto& req = act_j["req"];
                            if (req.contains("stats")) {
                                for (auto& [stat, val] : req["stats"].items()) {
                                    act.req_stats[stat] = val.get<int>();
                                }
                            }
                            act.is_locked = req.value("locked", false);
                        }

                        // Pesan popup yang muncul saat berhasil/gagal/terkunci
                        if (act_j.contains("dialogs")) {
                            const auto& d = act_j["dialogs"];
                            act.dialog_success = d.value("success", "");
                            act.dialog_fail = d.value("fail", "");
                            act.dialog_locked = d.value("locked", "");
                        }

                        // Daftar aksi yang dieksekusi saat kegiatan dilakukan (reward/efek)
                        if (act_j.contains("on_execute") && act_j["on_execute"].is_array()) {
                            for (const auto& action : act_j["on_execute"]) {
                                act.on_execute.push_back(action.get<std::string>());
                            }
                        }

                        p.add_activity(std::move(act));
                    }
                }

                places_db.emplace(p.get_id(), std::move(p));
                Utils::Logger::log("DB: Terdaftar lokasi '" + j["id"].get<std::string>() + "'");
            } catch (const std::exception& e) {
                Utils::Logger::log("DB ERROR: Gagal parse JSON di lokasi " + entry.path().string() + ": " + e.what());
            }
        }
    }
}

/**
 * Mengambil pointer lokasi berdasarkan ID.
 */
const Place* DB::get_place(const std::string& id) const {
    auto it = places_db.find(id);
    if (it != places_db.end()) return &(it->second);
    return nullptr;
}

/**
 * Mengambil daftar semua lokasi yang terdaftar.
 */
std::vector<const Place*> DB::get_all_places() const {
    std::vector<const Place*> all;
    for (auto& pair : places_db) all.push_back(&(pair.second));
    return all;
}

/**
 * Memuat template item dari folder JSON.
 * Item bisa berupa peralatan (equipment) atau barang konsumsi.
 */
void DB::load_items(const std::string& directory_path) {
    Utils::Logger::log("DB: Mulai memuat item dari " + directory_path);
    if (!fs::exists(directory_path)) return;

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            std::ifstream file(entry.path());
            if (!file.is_open()) continue;
            try {
                json j;
                file >> j;
                Item item(j["id"].get<std::string>(), j["name"].get<std::string>(), j.value("description", ""));
                item.value = j.value("value", 0);
                item.type = string_to_item_type(j.value("type", "misc"));
                item.equip_slot = j.value("equip_slot", ""); // "weapon", "armor", dll.
                item.weapon_type = j.value("weapon_type", "unarmed");
                
                // Bonus statistik saat item ini dipakai
                item.equip_stats["str"] = j.value("str", 0);
                item.equip_stats["cons"] = j.value("cons", 0);
                item.equip_stats["agi"] = j.value("agi", 0);
                item.equip_stats["intl"] = j.value("intl", 0);
                item.equip_stats["wis"] = j.value("wis", 0);

                // Aksi yang dijalankan saat item "digunakan" dari menu
                if (j.contains("on_use") && j["on_use"].is_array()) {
                    for (const auto& action : j["on_use"]) item.on_use.push_back(action.get<std::string>());
                }

                items_db.emplace(item.get_id(), std::move(item));
                Utils::Logger::log("DB: Terdaftar item '" + j["id"].get<std::string>() + "'");
            } catch (const std::exception& e) {
                Utils::Logger::log("DB ERROR: Gagal parse JSON di item " + entry.path().string() + ": " + e.what());
            }
        }
    }
}

/**
 * Mengambil template item berdasarkan ID.
 */
const Item* DB::get_item(const std::string& id) const {
    auto it = items_db.find(id);
    if (it != items_db.end()) return &(it->second);
    return nullptr;
}

/**
 * Memuat semua NPC dari folder JSON.
 * Mendukung jadwal dinamis (schedules) dan daftar misi (quests) yang dimiliki NPC.
 */
void DB::load_npcs(const std::string& directory_path) {
    Utils::Logger::log("DB: Mulai memuat NPC dari " + directory_path);
    if (!fs::exists(directory_path)) return;

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            std::ifstream file(entry.path());
            if (!file.is_open()) continue;
            try {
                json j;
                file >> j;
                NPCType type = (j.value("type", "named") == "named") ? NPCType::NAMED : NPCType::UNNAMED;
                NPC npc(j["id"].get<std::string>(), j["name"].get<std::string>(), type, j.value("role", ""), j.value("faction", "Neutral"),
                        j.value("str", 10), j.value("cons", 10), j.value("agi", 10), j.value("intl", 10), j.value("wis", 10), j.value("level", 1));
                npc.set_full_name(j.value("full_name", ""));
                npc.set_max_mp(j.value("max_mp", 20));
                npc.restore_mp(npc.get_max_mp());
                npc.set_affinity(string_to_element(j.value("affinity", "none")));
                npc.set_weakness(string_to_element(j.value("weakness", "none")));
                npc.set_weapon_type(j.value("weapon_type", "unarmed"));
                npc.set_weapon_name(j.value("weapon_name", "tangan kosong"));
                parse_magics_and_special(j, npc);
                
                // Memuat jadwal pergerakan NPC berdasarkan hari dan fase waktu
                if (j.contains("schedules") && j["schedules"].is_array()) {
                    for (auto& s_j : j["schedules"]) {
                        ScheduleEntry entry_s;
                        if (s_j.contains("days")) {
                            for (int d : s_j["days"]) entry_s.days.push_back(d);
                        }
                        entry_s.phase = s_j.value("phase", "Morning");
                        entry_s.location_id = s_j.value("location", "Unknown");
                        npc.add_schedule_entry(std::move(entry_s));
                    }
                } else if (j.contains("schedule")) {
                    // Fallback untuk format lama
                    for (auto& [phase, place_id] : j["schedule"].items()) {
                        ScheduleEntry entry_s;
                        entry_s.phase = phase;
                        entry_s.location_id = place_id.get<std::string>();
                        npc.add_schedule_entry(std::move(entry_s));
                    }
                }
                npc.set_default_dialog(j.value("default_dialog", ""));
                npc.set_first_dialog(j.value("first_dialog", ""));

                // Daftar misi yang bisa dipicu lewat NPC ini
                if (j.contains("quests") && j["quests"].is_array()) {
                    for (const auto& q_id : j["quests"]) {
                        npc.assign_quest_id(q_id.get<std::string>());
                    }
                }

                npcs_db.emplace(npc.get_id(), std::move(npc));
                Utils::Logger::log("DB: Terdaftar NPC '" + j["id"].get<std::string>() + "'");
            } catch (const std::exception& e) {
                Utils::Logger::log("DB ERROR: Gagal parse JSON di NPC " + entry.path().string() + ": " + e.what());
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

/**
 * Memuat template monster untuk sistem pertarungan.
 */
void DB::load_monsters(const std::string& directory_path) {
    Utils::Logger::log("DB: Mulai memuat monster dari " + directory_path);
    if (!fs::exists(directory_path)) return;

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            std::ifstream file(entry.path());
            if (!file.is_open()) continue;
            try {
                json j;
                file >> j;
                Monster mon(j["id"].get<std::string>(), j["name"].get<std::string>(), j.value("description", ""), 
                            j.value("max_hp", 50), j.value("damage", 5), j.value("agility", 10),
                            j.value("str", 10), j.value("cons", 10), j.value("intl", 10), j.value("wis", 10), j.value("level", 1));
                
                mon.set_max_mp(j.value("max_mp", 20));
                mon.restore_mp(mon.get_max_mp());
                mon.set_affinity(string_to_element(j.value("affinity", "none")));
                mon.set_weakness(string_to_element(j.value("weakness", "none")));
                mon.set_weapon_type(j.value("weapon_type", "unarmed"));
                mon.set_weapon_name(j.value("weapon_name", "tangan kosong"));
                
                std::string tactic_str = j.value("tactic", "ACT_FREELY");
                if (tactic_str == "FULL_ASSAULT") mon.set_tactic(Tactic::FULL_ASSAULT);
                else if (tactic_str == "HEAL_SUPPORT") mon.set_tactic(Tactic::HEAL_SUPPORT);
                else if (tactic_str == "CONSERVE_SP") mon.set_tactic(Tactic::CONSERVE_SP);
                else mon.set_tactic(Tactic::ACT_FREELY);
                
                mon.set_exp_drop(j.value("exp_drop", 10 * j.value("level", 1)));
                mon.set_gold_drop(j.value("gold_drop", 5 * j.value("level", 1)));
                
                parse_magics_and_special(j, mon);
                // Daftar item yang bisa dijatuhkan monster saat mati
                if (j.contains("loot") && j["loot"].is_array()) {
                    for (const auto& l_j : j["loot"]) {
                        mon.add_loot(l_j["item_id"].get<std::string>(), l_j["chance"].get<int>());
                    }
                }

                monsters_db.emplace(mon.get_id(), std::move(mon));
                Utils::Logger::log("DB: Terdaftar monster '" + j["id"].get<std::string>() + "'");
            } catch (const std::exception& e) {
                Utils::Logger::log("DB ERROR: Gagal parse JSON di monster " + entry.path().string() + ": " + e.what());
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

/**
 * Memuat semua misi (quests) dari folder JSON.
 * Misi memiliki syarat buka (unlock), syarat selesai (condition), dan rantaian dialog.
 */
void DB::load_quests(const std::string& directory_path) {
    Utils::Logger::log("DB: Mulai memuat misi dari " + directory_path);
    if (!fs::exists(directory_path)) return;

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            std::ifstream file(entry.path());
            if (!file.is_open()) continue;
            try {
                json j;
                file >> j;

                Quest q(j["id"].get<std::string>(), j["name"].get<std::string>(), j.value("description", ""));
                q.set_objective_text(j.value("objective_text", "Ikuti petunjuk quest."));
                q.set_source_npc(j.value("source_npc_id", ""));
                q.set_target_npc(j.value("target_npc_id", ""));
                q.set_target_location_id(j.value("target_location_id", j.value("target_location", "")));
                q.set_start_scene(j.value("start_scene_id", ""));
                q.set_complete_scene(j.value("complete_scene_id", ""));
                
                q.set_location_trigger_scene(j.value("location_trigger_scene", ""));
                if (j.contains("location_trigger_action") && j["location_trigger_action"].is_array()) {
                    std::vector<std::string> actions;
                    for (const auto& act : j["location_trigger_action"]) {
                        actions.push_back(act.get<std::string>());
                    }
                    q.set_location_trigger_action(std::move(actions));
                }

                // Syarat agar misi ini bisa diambil oleh pemain
                if (j.contains("unlock_conditions") && j["unlock_conditions"].is_array()) {
                    for (const auto& cond_j : j["unlock_conditions"]) {
                        q.add_unlock_condition(parse_condition(cond_j));
                    }
                } else if (j.contains("unlock_condition")) {
                    q.add_unlock_condition(parse_condition(j["unlock_condition"]));
                }

                // Syarat agar misi ini dianggap selesai (Progress tracking)
                if (j.contains("condition")) {
                    q.set_completion_condition(parse_condition(j["condition"]));
                } else if (j.contains("completion_condition")) {
                    q.set_completion_condition(parse_condition(j["completion_condition"]));
                }

                // Hadiah atau efek yang didapat saat misi ini diselesaikan
                if (j.contains("on_complete") && j["on_complete"].is_array()) {
                    std::vector<std::string> effects;
                    for (const auto& effect : j["on_complete"]) {
                        effects.push_back(effect.get<std::string>());
                    }
                    q.set_on_complete(std::move(effects));
                }

                quests_db.emplace(q.get_id(), std::move(q));
                Utils::Logger::log("DB: Terdaftar misi '" + j["id"].get<std::string>() + "'");
            } catch (const std::exception& e) {
                Utils::Logger::log("DB ERROR: Gagal parse JSON di misi " + entry.path().string() + ": " + e.what());
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

void DB::load_shops(const std::string& directory_path) {
    Utils::Logger::log("DB: Mulai memuat shop dari " + directory_path);
    if (!fs::exists(directory_path)) return;

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            std::ifstream file(entry.path());
            if (!file.is_open()) continue;
            try {
                json j;
                file >> j;
                Shop shop(j["id"].get<std::string>(), j["name"].get<std::string>());
                
                if (j.contains("items") && j["items"].is_array()) {
                    for (const auto& item_j : j["items"]) {
                        shop.add_item(item_j["item"].get<std::string>(), item_j["base_stock"].get<int>());
                    }
                }

                if (j.contains("restock_schedule") && j["restock_schedule"].is_array()) {
                    for (const auto& rs_j : j["restock_schedule"]) {
                        std::vector<std::string> items;
                        for (const auto& it_j : rs_j["items"]) {
                            items.push_back(it_j.get<std::string>());
                        }
                        shop.add_restock_schedule(rs_j["day"].get<int>(), items);
                    }
                }

                if (j.contains("on_enter") && j["on_enter"].is_array()) {
                    for (const auto& action : j["on_enter"]) shop.add_on_enter(action.get<std::string>());
                }

                if (j.contains("on_exit") && j["on_exit"].is_array()) {
                    for (const auto& action : j["on_exit"]) shop.add_on_exit(action.get<std::string>());
                }

                shops_db.emplace(shop.get_id(), std::move(shop));
                Utils::Logger::log("DB: Terdaftar shop '" + j["id"].get<std::string>() + "'");
            } catch (const std::exception& e) {
                Utils::Logger::log("DB ERROR: Gagal parse JSON di shop " + entry.path().string() + ": " + e.what());
            }
        }
    }
}

const Shop* DB::get_shop(const std::string& id) const {
    auto it = shops_db.find(id);
    if (it != shops_db.end()) return &(it->second);
    return nullptr;
}

std::vector<const Shop*> DB::get_all_shops() const {
    std::vector<const Shop*> all;
    for (auto& pair : shops_db) all.push_back(&(pair.second));
    return all;
}

void DB::load_dungeon_pools(const std::string& filepath) {
    Utils::Logger::log("DB: Memuat pool monster dungeon dari " + filepath);
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Utils::Logger::log("DB ERROR: Tidak bisa membuka file pool dungeon " + filepath);
        return;
    }
    try {
        json root;
        file >> root;
        for (auto& [floor_str, list_j] : root.items()) {
            int floor = std::stoi(floor_str);
            std::vector<std::string> pool;
            for (const auto& id_j : list_j) {
                pool.push_back(id_j.get<std::string>());
            }
            dungeon_pools[floor] = pool;
        }
    } catch (...) {
        Utils::Logger::log("DB ERROR: Gagal memparsing JSON pool dungeon " + filepath);
    }
}

const std::vector<std::string>& DB::get_dungeon_pool(int floor) const {
    auto it = dungeon_pools.find(floor);
    if (it != dungeon_pools.end()) {
        return it->second;
    }
    static std::vector<std::string> empty_pool;
    return empty_pool;
}