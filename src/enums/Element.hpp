#pragma once
#include <string>

enum class Element {
    FIRE,
    WATER,
    EARTH,
    WIND,
    LIGHT,
    NONE
};

inline Element string_to_element(const std::string& str) {
    if (str == "fire") return Element::FIRE;
    if (str == "water") return Element::WATER;
    if (str == "earth") return Element::EARTH;
    if (str == "wind") return Element::WIND;
    if (str == "light") return Element::LIGHT;
    return Element::NONE;
}

inline std::string element_to_string(Element e) {
    switch (e) {
        case Element::FIRE: return "Fire";
        case Element::WATER: return "Water";
        case Element::EARTH: return "Earth";
        case Element::WIND: return "Wind";
        case Element::LIGHT: return "Light";
        default: return "None";
    }
}
