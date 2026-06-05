#pragma once
#include <string>
#include <unordered_map>
#include "../models/Dialog.hpp"
#include "../models/Place.hpp"

class DB {
    std::unordered_map<std::string, DialogScene> dialog_scenes;
    std::unordered_map<std::string, Place> places_db;

public:
    DB();
    void load_dialogs(const std::string& directory_path);
    const DialogScene* get_dialog_scene(const std::string& id) const;

    void load_places(const std::string& directory_path);
    const Place* get_place(const std::string& id) const;
    std::vector<const Place*> get_all_places() const;
};