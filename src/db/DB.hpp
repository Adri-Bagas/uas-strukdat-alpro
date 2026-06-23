#pragma once
#include <string>
#include <unordered_map>
#include "../models/Dialog.hpp"
#include "../models/Place.hpp"
#include "../models/Item.hpp"
#include "../models/NPC.hpp"
#include "../models/Quest.hpp"
#include "../models/Shop.hpp"

class DB {
    std::unordered_map<std::string, DialogScene> dialog_scenes;
    std::unordered_map<std::string, Place> places_db;
    std::unordered_map<std::string, Item> items_db;
    std::unordered_map<std::string, NPC> npcs_db;
    std::unordered_map<std::string, Monster> monsters_db;
    std::unordered_map<std::string, Quest> quests_db;
    std::unordered_map<std::string, Shop> shops_db;
    std::unordered_map<std::string, std::vector<std::string>> endings_db;

public:
    DB();
    
    void load_dialogs(const std::string& directory_path);
    const DialogScene* get_dialog_scene(const std::string& id) const;

    void load_places(const std::string& directory_path);
    const Place* get_place(const std::string& id) const;
    std::vector<const Place*> get_all_places() const;

    void load_items(const std::string& directory_path);
    const Item* get_item(const std::string& id) const;

    void load_npcs(const std::string& directory_path);
    const NPC* get_npc(const std::string& id) const;
    std::vector<const NPC*> get_all_npcs() const;

    void load_monsters(const std::string& directory_path);
    const Monster* get_monster(const std::string& id) const;
    std::vector<const Monster*> get_all_monsters() const;

    void load_quests(const std::string& directory_path);
    const Quest* get_quest(const std::string& id) const;
    std::vector<const Quest*> get_all_quests() const;

    void load_shops(const std::string& directory_path);
    const Shop* get_shop(const std::string& id) const;
    std::vector<const Shop*> get_all_shops() const;

    void load_endings(const std::string& file_path);
    std::vector<std::string> get_ending(const std::string& id) const;
};