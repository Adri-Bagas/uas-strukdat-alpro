#pragma once
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>

inline std::vector<std::string> word_wrap(const std::string& text, int max_width) {
    std::vector<std::string> lines;
    if (text.empty()) {
        lines.push_back("");
        return lines;
    }

    std::stringstream ss(text);
    std::string word;
    std::string line;

    while (ss >> word) {
        while ((int)word.length() > max_width) {
            if (!line.empty()) {
                lines.push_back(line);
                line.clear();
            }
            lines.push_back(word.substr(0, max_width));
            word = word.substr(max_width);
        }

        if (word.empty()) continue;

        if (line.empty()) {
            line = word;
        } else if ((int)(line.length() + 1 + word.length()) > max_width) {
            lines.push_back(line);
            line = word;
        } else {
            line += " " + word;
        }
    }

    if (!line.empty()) lines.push_back(line);
    if (lines.empty()) lines.push_back("");

    return lines;
}

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
