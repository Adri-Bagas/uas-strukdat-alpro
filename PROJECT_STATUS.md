# Project Status Report

This document outlines the current state of the `uas-strukdat-alpro` project, detailing what has been implemented, what is currently in progress, what needs to be done to reach completion, and a list of known issues within the codebase.

---

## 1. Implemented Features (Completed)

The core foundation and engine of the game are solid and fully operational:

*   **Data-Driven Architecture**: The game successfully loads `Places`, `Activities`, `NPCs`, `Monsters`, `Items`, `Quests`, and `Dialogues` entirely from JSON files via the `DB` class.
*   **Action & Condition System**: A string-based Action Dispatcher (`Actions.cpp`) and a `Condition` evaluator (`Condition.cpp`) allow JSON files to trigger game events, modify stats, check inventory, and handle quest logic without hardcoding C++.
*   **Dynamic Town State (Exploration)**: 
    *   Players can navigate between locations using the `[Go]` menu.
    *   Locations restrict movements based on bidirectional `walkable` arrays.
*   **NPC & Scheduling System**: 
    *   NPCs move between locations dynamically based on the current day and time phase (`ScheduleEntry`).
    *   **Identity Discovery**: NPCs start as `??? (Role)` until the `reveal_name` action is triggered.
*   **Quest System**: 
    *   Quests transition between `LOCKED`, `AVAILABLE`, `IN_PROGRESS`, and `COMPLETED`.
    *   Contextual Interaction Menus automatically show which quests an NPC can offer or complete.
*   **Dialogue System**:
    *   Typewriter effect for dialogue, thought bubbles, and popups.
    *   Branching narrative choices utilizing the new object-oriented `ChoicePopup` component.
*   **Interactive City Map (Map Mode)**:
    *   Repurposed the `win_thought` window to display a node-based ASCII map of Nirva.
    *   Players can press `TAB` to shift focus to the map, preview locations, and instantly travel to adjacent districts.
    *   Wrap-around list navigation implemented for all Town menus.
*   **Tabbed Area Interaction**:
    *   Players can use `TAB` to cycle between distinct interaction categories: `Orang`, `Aktivitas`, `Jalan Keluar`, and `Peta`.
    *   Empty tabs are smartly skipped to prevent navigating to blank menus.
*   **Shop System**:
    *   Created `ShopManager` and `ShopState`.
    *   Implemented full buy/sell interface, displaying item prices dynamically based on item types.
    *   Supports gold checking and inventory limit validation.
*   **Dynamic Item Stats & Loot Tables**:
    *   Replaced base stat getters with virtual implementations that factor in `equipped_items` bonuses on the fly.
    *   Integrated random rolls for loot tables upon enemy death in the battle state, correctly registering item drops and showing them in the end-of-battle pop-up.
*   **Resilient Ncurses UI**: 
    *   Localized to Indonesian.
    *   Robust `KEY_RESIZE` handling prevents terminal crashes when resizing or zooming (Ctrl +/-).
*   **Interactive Inventory System**: 
    *   Full `InventoryState` accessible from `TownState` and `DungeonState`.
    *   Players can view items, read word-wrapped descriptions, consume potions (`on_use`), and equip weapons/armor.
    *   Keyboard navigation with W/S/↑/↓ and contextual shortcuts.
*   **Keyboard Shortcuts (Pintasan)**: Every primary state (TownState, DungeonState, InventoryState) displays contextual shortcuts in `win_task`.
*   **Quit Confirmation**: Prevents accidental exits in TownState with a Y/N confirmation popup.
*   **Word Wrap Utility**: Shared `word_wrap()` function in `StringUtils.hpp` for consistent text wrapping across all dialogue and description rendering.
*   **Data-Driven Day 2 Trigger**: The `seen_day2_cutscene` variable is set via the JSON action system (`set_var seen_day2_cutscene 1` in `day2_opening.json`) instead of being hardcoded in `TownState.cpp`.

---

## 2. Known Bugs & Code Smells

These are technical debts and bugs currently existing in the code that should be addressed:

### Bugs

### Known Bugs / Code Smells

### Completed Refactors