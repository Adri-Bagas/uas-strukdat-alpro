#pragma once
#include <string>

class Item {
private:
    std::string id;
    std::string name;

public:
    Item(std::string id, std::string name) : id(std::move(id)), name(std::move(name)) {}
    
    const std::string& get_id() const { return id; }
    const std::string& get_name() const { return name; }
};