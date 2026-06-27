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

## 2. In Progress / Partially Implemented

These systems have their data models built and JSON loaded, but lack the gameplay loops to fully utilize them:


*   **Time Limit Mechanic (14 Days)**: 
    *   *Done*: The Calendar tracks days, UI shows "Sisa Hari" (Days Left), and Lord Inquisitor Vane is scheduled to arrive on Day 14.
    *   *Missing*: A hard "Game Over" or "Judgment" engine trigger if the player fails to solve the mystery by Day 15.

---

## 3. To-Do / Needs Implementation (Road to 1.0)

To finish the project, the following major features must be built:


2.  **Advanced Quest Mechanics**:
    *   **Quest Rejection & Retention**: Allow players to decline a quest during dialogue and still have the option to pick it up later from the NPC on a different day.
    *   **Complex Completion Triggers**: Upgrade the `Condition` evaluator to support diverse quest objectives beyond just variable tracking. This includes specific item fetching, targeted monster kills, talking to a specific NPC in a sequence, or triggering completion simply by entering a specific area.
3.  **Bulletin Board System (Papan Pengumuman)**:
    *   Create a centralized hub (likely in the `alun_alun` or `balai_kota`) for procedural or generic "Unnamed NPC" quests. This prevents the player from having to hunt down random citizens for basic fetch/kill tasks.
4.  **Data Structures Implementation (From Scratch)**:
    *   **Double Linked List** (DONE): Implemented a custom `DoubleLinkedList` to manage a Message Log / Activity History. Players can press `L` to view notifications chronologically.
    *   **Binary Search Tree (BST) / AVL Tree**: 
        *   **Implementation Strategy**: Construct a custom, self-balancing tree (such as an AVL Tree) to serve as the backend for the **Encyclopedia/Bestiary** system. 
        *   **Mechanics**: Every time the player encounters a new monster, discovers a new item, or uncovers lore, an entry is inserted into the BST. The nodes should be keyed by alphabetical string comparison (e.g., entity name) or a numerical ID.
        *   **Advantages**: This guarantees `O(log N)` search and insertion times. When the player opens the Encyclopedia UI, an **in-order traversal** of the tree will automatically yield a perfectly alphabetized list of all discovered entities, making UI rendering incredibly efficient without needing an external sorting algorithm like `std::sort`.
    *   **Stack (LIFO)**: 
        *   **Implementation Strategy**: Develop a custom, from-scratch `Stack` class utilizing templates.
        *   **Mechanics**: Integrate this stack into the `GameEngine` or specific UI states to manage **Menu/Screen Navigation History**. Whenever a player drills down into a sub-menu (e.g., Main Menu -> Inventory -> Select Item -> Use Item Confirmation), the previous menu's pointer or context state is pushed onto the stack. 
        *   **Advantages**: Pressing `ESC` or a "Back" button will simply pop the top element off the stack, instantly restoring the exact previous state and cursor position. This completely eliminates hardcoded "back" routing and enables infinitely nested menus safely.
    *   **Graph with BFS/DFS**: Implemented (Breadth-First Search utilized for Fast Travel pathfinding across the Town's map nodes).
7.  **Save/Load System (Optional but Recommended)**:
    *   Implement a way to serialize the `Player` state (variables, inventory, quest states, time) back into a save-game JSON file.

---

## 4. Known Bugs & Code Smells

These are technical debts and bugs currently existing in the code that should be addressed:

### Bugs

*   **Dungeon Terminal Size Crash**: `DungeonState::render()` requires a minimum terminal size of 110x24. If the terminal is resized smaller than this *while* inside the dungeon, it shows an error overlay, but rapid resizing might still cause `std::vector` out-of-bounds exceptions if the player moves while the terminal is too small.

### Known Bugs / Code Smells
- **Terminal Resize Responsiveness Issue**: Components in certain states (like Popups or inner contents in `TownState` / `ShopState`) are sometimes not correctly re-calculating their proportional width/height after a `KEY_RESIZE` event, meaning they maintain their original dimensions instead of becoming responsive like web layouts. This appears related to ncurses window destruction/re-creation loops.

### Completed Refactors
- **Blocking UI Loops**: Refactored `Popup` to use a non-blocking state machine via `GameEngine::run()`.
- **Hardcoded Dialogue Choice Parser**: Replaced custom string parsing in `DialogManagers.cpp` with the `Condition` system.
- **Action Dispatcher Duplication**: Replaced hardcoded string parsing in `execute_actions` with standard action dispatcher.