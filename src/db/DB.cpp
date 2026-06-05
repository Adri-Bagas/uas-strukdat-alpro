#include "DB.hpp"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "../utils/Logger.hpp"
#include "../utils/components/ErrorPopup.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

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
                json j;
                file >> j;

                DialogScene scene;
                scene.id = j["id"].get<std::string>();

                scene.on_start = j.value("on_start", "");
                scene.on_exit = j.value("on_exit", "");
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
                    act.time_cost = act_j.value("time_cost", 1);
                    act.stamina_cost = act_j.value("stamina_cost", 0);

                    const auto& sched = act_j["schedule"];
                    for (int d : sched["days"]) act.days.push_back(d);
                    for (const auto& ph : sched["phases"]) act.phases.push_back(ph.get<std::string>());

                    const auto& req = act_j["req"];
                    act.req_quest = req.value("quest", "");
                    act.min_str = req.value("min_str", 0);
                    act.is_locked = req.value("locked", false);

                    const auto& eff = act_j["effect"];
                    act.reward_gold = eff.value("gold", 0);
                    act.reward_str = eff.value("str", 0);
                    act.finish_quest_id = eff.value("finish_quest", "");

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