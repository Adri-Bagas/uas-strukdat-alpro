#pragma once
#include <string>
#include <algorithm>

inline std::string to_display_name(const std::string& id) {
    std::string name = id;
    // Strip common prefixes
    if (name.find("item_") == 0) name = name.substr(5);
    if (name.find("mon_") == 0) name = name.substr(4);
    // Replace underscores with spaces
    for (auto& ch : name) {
        if (ch == '_') ch = ' ';
    }
    // Capitalize first letter
    if (!name.empty()) name[0] = std::toupper(name[0]);
    return name;
}
