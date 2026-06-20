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

inline float get_elemental_multiplier(Element attack_elem, Element target_affinity, Element target_weakness) {
    if (attack_elem == Element::NONE) return 1.0f;
    
    // Explicit weakness takes absolute priority
    if (attack_elem == target_weakness) return 2.0f;
    
    // If hitting same element, it's resisted
    if (attack_elem == target_affinity && target_affinity != Element::NONE) return 0.5f;
    
    // Built-in cycle if no explicit weakness/affinity given
    if (attack_elem == Element::FIRE && target_affinity == Element::WIND) return 1.5f;
    if (attack_elem == Element::WIND && target_affinity == Element::EARTH) return 1.5f;
    if (attack_elem == Element::EARTH && target_affinity == Element::WATER) return 1.5f;
    if (attack_elem == Element::WATER && target_affinity == Element::FIRE) return 1.5f;
    
    // Light doesn't have a built-in cycle weakness, but does 1.2x to all non-light
    if (attack_elem == Element::LIGHT && target_affinity != Element::LIGHT) return 1.2f;
    
    return 1.0f;
}
