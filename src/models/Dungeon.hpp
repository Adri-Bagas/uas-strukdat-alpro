#pragma once
#include <vector>

struct DungeonFloor {
    int floor_number;
    int height;
    int width;
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<bool>> visited;
    int player_r;
    int player_c;
    int exit_r;
    int exit_c;
    int start_r;
    int start_c;
};

struct DungeonFloorNode {
    DungeonFloor floor;
    DungeonFloorNode* next = nullptr;
    DungeonFloorNode* prev = nullptr;
};

class Dungeon {
private:
    DungeonFloorNode* head = nullptr;
    DungeonFloorNode* tail = nullptr;
    DungeonFloorNode* current_floor = nullptr;
    int total_floors = 0;

public:
    Dungeon() = default;
    ~Dungeon() {
        clear();
    }

    // Disable copying to prevent double-free issues
    Dungeon(const Dungeon&) = delete;
    Dungeon& operator=(const Dungeon&) = delete;

    // Enable moving
    Dungeon(Dungeon&& other) noexcept {
        head = other.head;
        tail = other.tail;
        current_floor = other.current_floor;
        total_floors = other.total_floors;
        
        other.head = nullptr;
        other.tail = nullptr;
        other.current_floor = nullptr;
        other.total_floors = 0;
    }

    Dungeon& operator=(Dungeon&& other) noexcept {
        if (this != &other) {
            clear();
            head = other.head;
            tail = other.tail;
            current_floor = other.current_floor;
            total_floors = other.total_floors;
            
            other.head = nullptr;
            other.tail = nullptr;
            other.current_floor = nullptr;
            other.total_floors = 0;
        }
        return *this;
    }

    void clear() {
        DungeonFloorNode* curr = head;
        while (curr != nullptr) {
            DungeonFloorNode* next_node = curr->next;
            delete curr;
            curr = next_node;
        }
        head = nullptr;
        tail = nullptr;
        current_floor = nullptr;
        total_floors = 0;
    }

    void add_floor(const DungeonFloor& floor_data) {
        DungeonFloorNode* new_node = new DungeonFloorNode{floor_data, nullptr, nullptr};
        if (head == nullptr) {
            head = new_node;
            tail = new_node;
            current_floor = new_node;
        } else {
            tail->next = new_node;
            new_node->prev = tail;
            tail = new_node;
        }
        total_floors++;
    }

    DungeonFloorNode* get_current_node() const {
        return current_floor;
    }

    void set_current_node(DungeonFloorNode* node) {
        current_floor = node;
    }

    bool go_to_next_floor() {
        if (current_floor && current_floor->next) {
            current_floor = current_floor->next;
            return true;
        }
        return false;
    }

    bool go_to_prev_floor() {
        if (current_floor && current_floor->prev) {
            current_floor = current_floor->prev;
            return true;
        }
        return false;
    }

    int get_total_floors() const {
        return total_floors;
    }
    
    DungeonFloorNode* get_head() const {
        return head;
    }
};
