# Project Guide: uas-strukdat-alpro

## Overview
This is a terminal-based Role-Playing Game (RPG) written in C++17. The game is rendered directly in the terminal using the `ncurses` library. It relies heavily on a **Data-Driven Design**, utilizing JSON files extensively to define world locations, activities, quests, NPCs, and branching dialogues. The game employs a state-based engine architecture to handle different screens and gameplay loops.

## Technology Stack
- **Language**: C++17
- **Build System**: CMake (requires 3.10+)
- **Environment**: Nix (configured via `shell.nix`)
- **Libraries**:
  - `ncurses` (and `panel`, `menu`) for terminal UI rendering.
  - `nlohmann_json` (fetched via CMake) for JSON parsing.
  - `libvlcpp` & `vlc` for potential audio/media support (fetched/linked via CMake).

## Directory Structure
- `src/` - The main C++ source code.
  - `models/` - C++ entity classes (`Player`, `Item`, `Quest`, `NPC`, `Place`, `Condition`, `Activity`, `Dialog`).
  - `managers/` - System controllers governing logic (`PlayerManager`, `PlaceManagers`, `QuestManagers`, `DialogManagers`, `TimeCalendarManagers`).
  - `states/` - Logic for game states (`TownState`, `DungeonState`) pushed/popped onto the engine's stack.
  - `views/` - `ncurses` UI layouts and static drawing logic (`MainPage`).
  - `db/` - JSON loading and data storage (`DB` class).
  - `actions/` - The string-based Action Dispatcher.
  - `utils/components/` - Floating, interactive UI components like `Popup`, `ChoicePopup`, and `ErrorPopup`.
- `data/` - JSON configuration files holding the game's data (`dialogs`, `items`, `monsters`, `npcs`, `places`, `quests`).
- `CMakeLists.txt` - Build configuration.
- `shell.nix` - Nix environment configuration defining required system dependencies.

---

## Core Architecture

### 1. `GameEngine` (Core Controller)
The `GameEngine` is the heart of the application.
- It initializes the `ncurses` environment and sets up color pairs.
- It holds instances of all **Managers**, the **Database (`DB`)**, and **Views**.
- It manages the game loop via `run()`.
- It uses a **State Stack** (push/pop) to determine the active context (e.g., `TownState` for world exploration, `DungeonState` for the maze). Only the top state receives input and update ticks.

### 2. State Management (`src/states/`)
Every major game screen implements the `GameState` interface:
- `handle_input(int ch)`: Processes keyboard input.
- `update()`: Runs background logic, animations, or queue processing (like popping the next dialogue node).
- `render()`: Draws the state to the terminal.
- `on_enter()` / `on_exit()`: Lifecycle hooks triggered when the state is pushed or popped.

### 3. Data-Driven Systems (The "Engine Magic")

To avoid hardcoding game logic in C++, the engine relies on three interconnected systems that allow JSON files to control the game flow:

#### A. The Action Dispatcher (`src/actions/Actions.cpp`)
Instead of hardcoding what a quest reward or an activity does, the engine uses string-based commands. 
The `Action` class maps strings like `"add_gold 50"`, `"give_item potion 2"`, or `"advance_day"` to C++ lambdas. 
*Usage:* JSON files define an array of `on_execute` strings. When an activity is clicked, the engine loops through the array and passes the strings to `engine->get_actions().execute()`.

#### B. Game Variables (`src/models/Player.hpp`)
To track progress (like "has the player found the clue?", "how many slimes are dead?"), the `Player` model holds a generic `std::unordered_map<std::string, int> game_vars`.
*Usage:* The Action dispatcher can manipulate these using `"add_var clue_found 1"` or `"set_var spoken_to_arthur 1"`.

#### C. The Condition Evaluator (`src/models/Condition.cpp`)
Activities, quests, and dialogue branches often require prerequisites. The `Condition` struct can evaluate itself against the `Player`'s variables, inventory, or the `QuestManager`.
*Usage:* In JSON, a condition looks like `{"type": "var_greater_equal", "key": "str", "value": 5}`. The UI checks `cond.evaluate(player)` before displaying an option.

---

## Game Systems Deep Dive

### The NPC System
- **Dynamic Schedules**: NPCs move autonomously based on the day of the month and the time phase (Morning, Afternoon, Evening, Night). This is defined in `schedules` in their JSON. The `PlaceManagers` automatically shuffles them between `Place` instances upon time changes.
- **Identity Discovery**: Named NPCs have a hidden `full_name`. Initially, they appear as `??? (Role)` to the player. A dialogue scene can trigger the `"reveal_name npc_id"` action, which marks the NPC as "known" globally.

### The Quest System
- Quests are loaded from `data/quests/`.
- They possess an `unlock_condition` (to become available) and a `completion_condition` (to be finished).
- When you talk to an NPC, the `TownState` queries the `QuestManager` for all quests associated with that NPC. Depending on the conditions, it generates a contextual "Interaction Menu" showing New, In-Progress, or Completeable quests.

### Dialogue & Branching
- Dialogues are a series of `DialogNode` objects (Dialog, Thought, or Popup).
- The `TownState` processes these nodes asynchronously in its `update()` loop, creating a typewriter effect.
- **Branching Choices**: At the end of a `DialogScene`, the JSON can define `choices`. The `DialogManager` renders these using the `ChoicePopup` component, allowing the player to select a path that triggers a `next_scene`.

### UI Resilience & Resizing
Terminal emulators can be resized dynamically (e.g., via font size changes like `Ctrl`+`+`). The game is built to survive this:
- **Blocking Loops**: Popups and typing animations use `while(true)` loops that block the main engine loop. To prevent crashes, these loops listen for `KEY_RESIZE`.
- **Redraw Logic**: When a resize occurs, the UI performs a robust reset sequence (`resizeterm(0, 0) -> endwin() -> refresh() -> clear()`) and completely re-creates all ncurses windows to match the new dimensions. If the terminal is too small (under 80x22), the game halts rendering and displays a warning until resized properly.

### Dungeon Maze Generation
`DungeonState` uses a **Randomized Prim's Algorithm** to generate a perfect maze (no isolated sections, guaranteed path to exit) dynamically every time the state is entered. 

---

## How to Build and Run
We mandate the use of `nix-shell` to ensure all external dependencies (like VLC and specific ncurses versions) are linked correctly.

```bash
# Enter the reproducible environment
nix-shell

# Build the project
cmake -B build -S .
cmake --build build

# Run the game
./build/ftdd
```

## Creating Content (JSON Guide)
When adding new content, follow the existing JSON schemas in the `data/` folder.
- **Locations**: Must be connected bidirectionally via the `walkable` array.
- **Activities**: Can be locked behind stats (`req_stats`) or game variables (`visible_condition`).
- **Dialogues**: Group related scenes in a single JSON array to keep the folder tidy. Use `"on_start"` to trigger state changes when a conversation begins.