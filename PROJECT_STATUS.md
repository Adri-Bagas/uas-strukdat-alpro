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
*   **Resilient Ncurses UI**: 
    *   Localized to Indonesian.
    *   Robust `KEY_RESIZE` handling prevents terminal crashes when resizing or zooming (Ctrl +/-).

---

## 2. In Progress / Partially Implemented

These systems have their data models built and JSON loaded, but lack the gameplay loops to fully utilize them:

*   **Dungeon Maze (`DungeonState.cpp`)**: 
    *   *Done*: Procedural maze generation using Randomized Prim's Algorithm. Player can walk around and reach the exit for a gold reward.
    *   *Missing*: Enemy encounters. The maze currently has no threats or traps.
*   **Equipment & Inventory System**: 
    *   *Done*: Items are loaded (`load_items`), players can receive them (`give_item`), and `Player` model has `equip()` logic. The UI reserves space to show equipped items.
    *   *Missing*: There is no interactive Inventory Menu for the player to manually consume potions (`on_use`) or equip weapons/armor.
*   **Time Limit Mechanic (14 Days)**: 
    *   *Done*: The Calendar tracks days, UI shows "Sisa Hari" (Days Left), and Lord Inquisitor Vane is scheduled to arrive on Day 14.
    *   *Missing*: A hard "Game Over" or "Judgment" engine trigger if the player fails to solve the mystery by Day 15.

---

## 3. To-Do / Needs Implementation (Road to 1.0)

To finish the project, the following major features must be built:

1.  **Battle System (`BattlePage` / `BattleState`)**:
    *   Need a turn-based combat loop supporting multi-entity encounters (Player + AI Party vs. up to 4 Monsters).
    *   **Autonomous Party Members**: Important NPCs (like Arthur or Garrick) can join the player's party. They will *not* be manually controlled by the player. Instead, they require an AI logic system (similar to the Monster AI) to automatically choose their targets and actions (attack, heal, defend) on their turn.
    *   **Elemental Magic System**: Implement magic attacks that utilize `INT` (Intelligence) and `WIS` (Wisdom) stats instead of STR/AGI. Combat calculations must factor in the `Elemental Affinity` (Fire, Water, etc.) of both the attacker and the defender for weakness/resistance multipliers.
    *   Need to implement HP/MP damage calculations, usage of equipped weapons, and processing Monster `loot_tables` upon victory.
2.  **Advanced Quest Mechanics**:
    *   **Quest Rejection & Retention**: Allow players to decline a quest during dialogue and still have the option to pick it up later from the NPC on a different day.
    *   **Complex Completion Triggers**: Upgrade the `Condition` evaluator to support diverse quest objectives beyond just variable tracking. This includes specific item fetching, targeted monster kills, talking to a specific NPC in a sequence, or triggering completion simply by entering a specific area.
3.  **Bulletin Board System (Papan Pengumuman)**:
    *   Create a centralized hub (likely in the `alun_alun` or `balai_kota`) for procedural or generic "Unnamed NPC" quests. This prevents the player from having to hunt down random citizens for basic fetch/kill tasks.
4.  **Area Interaction Overhaul**:
    *   Redesign how players interact with their current location. The current unified list of Activities, People, and Exits works, but needs to be more immersive or context-sensitive as the number of interactions grows.
5.  **Interactive Inventory UI**:
    *   A new sub-menu (perhaps triggered by pressing 'i' in `TownState`) where players can view item descriptions, consume healing items, and equip gear.
6.  **Stat Calculation Fix**:
    *   Update the `Player` model so that `get_str()`, `get_cons()`, etc., return the *Base Stat + Equipped Item Stat Bonuses*.
7.  **Save/Load System (Optional but Recommended)**:
    *   Implement a way to serialize the `Player` state (variables, inventory, quest states, time) back into a save-game JSON file.

---

## 4. Known Bugs & Code Smells

These are technical debts and bugs currently existing in the code that should be addressed:

### Bugs
*   **Stat Bonus Ignored**: The `Item` model has `equip_stats` (e.g., +5 STR for Iron Sword), and the UI displays equipped items. However, the `Player` model currently does not add these item bonuses to the base stats. Activities that check for `str >= 10` only check base stats.
*   **Dungeon Terminal Size Crash**: `DungeonState::render()` requires a minimum terminal size of 110x24. If the terminal is resized smaller than this *while* inside the dungeon, it shows an error overlay, but rapid resizing might still cause `std::vector` out-of-bounds exceptions if the player moves while the terminal is too small.

### Code Smells (Needs Refactoring)
*   **Hardcoded Dialogue Choice Parser**: In `src/managers/DialogManagers.cpp`, the `evaluate_choice_condition` function uses a very rudimentary string parser (e.g., `condition.rfind("item_", 0) == 0`) left over from a branch merge. **Fix:** It should be updated to use the robust `parse_condition` and `Condition::evaluate` system we use everywhere else.
*   **Action Dispatcher Duplication**: Some old actions in `Actions.cpp` (like `add_trust_warga_5`) were converted to use `add_var trust_warga 5`, but the old specific string parsers might still be lingering in the `execute_actions` block inside `DialogManagers.cpp`. They should all be routed cleanly through `engine->get_actions().execute()`.