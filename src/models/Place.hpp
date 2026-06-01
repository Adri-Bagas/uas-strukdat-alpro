#pragma once
#include <string>
#include <vector>

class NPC;

class Place {
    private:
        std::string id;
        std::string name;
        std::vector<NPC*> present_npcs;
        
        std::vector<std::string> walkable_place_ids;
        std::vector<Place*> walkable_places;

    public:
        Place(std::string id, std::string name) : id(std::move(id)), name(std::move(name)) {}

        const std::string& get_id() const { return id; }
        const std::string& get_name() const { return name; }

        void add_npc(NPC* npc) { present_npcs.push_back(npc); }
        void clear_npcs() { present_npcs.clear(); }
        const std::vector<NPC*>& get_npcs() const { return present_npcs; }

        void add_walkable_id(const std::string& wid) { walkable_place_ids.push_back(wid); }
        const std::vector<std::string>& get_walkable_ids() const { return walkable_place_ids; }
        
        void add_walkable_place(Place* p) { walkable_places.push_back(p); }
        const std::vector<Place*>& get_walkable_places() const { return walkable_places; }

        ~Place() {}
};