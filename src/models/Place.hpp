#pragma once
#include "Activity.hpp"
#include <string>
#include <vector>
#include <map>

class NPC;

class Place {
    private:
        std::string id;
        std::string name;
        std::string on_first_enter_dialog;
        std::string on_enter_dialog;
        bool has_entered_before = false;

        std::vector<NPC*> present_npcs;
        std::vector<Activity> activities;
        
        std::map<std::string, std::string> walkable_place_ids;
        std::map<std::string, Place*> walkable_places;

    public:
        Place(std::string id, std::string name) : id(std::move(id)), name(std::move(name)) {}

        const std::string& get_id() const { return id; }
        const std::string& get_name() const { return name; }

        void set_on_first_enter(std::string d) { on_first_enter_dialog = std::move(d); }
        const std::string& get_on_first_enter() const { return on_first_enter_dialog; }

        void set_on_enter(std::string d) { on_enter_dialog = std::move(d); }
        const std::string& get_on_enter() const { return on_enter_dialog; }

        bool get_has_entered() const { return has_entered_before; }
        void set_has_entered(bool b) { has_entered_before = b; }

        void add_activity(Activity a) { activities.push_back(std::move(a)); }
        const std::vector<Activity>& get_activities() const { return activities; }
        std::vector<Activity>& get_mutable_activities() { return activities; }

        void add_npc(NPC* npc) { present_npcs.push_back(npc); }
        void clear_npcs() { present_npcs.clear(); }
        const std::vector<NPC*>& get_npcs() const { return present_npcs; }

        void add_walkable_id(const std::string& dir, const std::string& wid) { walkable_place_ids[dir] = wid; }
        const std::map<std::string, std::string>& get_walkable_ids() const { return walkable_place_ids; }
        
        void add_walkable_place(const std::string& dir, Place* p) { walkable_places[dir] = p; }
        const std::map<std::string, Place*>& get_walkable_places() const { return walkable_places; }

        ~Place() {}
};