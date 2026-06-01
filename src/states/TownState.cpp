#include "TownState.hpp"
#include "../GameEngine.hpp" // Full include allowed in .cpp
#include <ncurses.h>

std::string element_to_string(Element e) {
    switch (e) {
        case Element::FIRE: return "Fire";
        case Element::WATER: return "Water";
        case Element::EARTH: return "Earth";
        case Element::WIND: return "Wind";
        case Element::LIGHT: return "Light";
        default: return "None";
    }
}

TownState::TownState(GameEngine* eng) : GameState(eng) {}

void TownState::on_enter() {
    
    engine->get_places().update_npc_locations(engine->get_calendar(), engine->get_quests());
    engine->get_layout().draw();

    std::string intro_text = "The air is lovely. Lorem ipsum dolor sit amet asldhjioashdaoisdhasoidhasdoi oihdsaoiashdaso ihdsaio doash doiahsdioahsdoiahsdoihao isoi hasoiahsdioahsoihasoidhaiosdhioashdhhdoas  sado ihsdoihasd as ohdaosi hdoi asdh aoshdoiashdioashsdioasdasoi dasodaosd asdhasd haiosds haoisdhsaodaoishsdoaisd ahiosd hoisad hoiasd asd ioasd hioasdh oiasd hioasdas hodas ohida ohiusd oiausd o asudh aiosdhasoih doasd sado iasdh odhoahdo aishd aiosud aiosusd hioasd oihasdhoau sdhiou ";

    auto history = engine->get_dialogs().get_thoughts();

    engine->get_layout().type_new_text(
        engine->get_layout().win_thought,
        "Thoughts & Desc",
        engine->get_layout().w_left,
        history,
        intro_text
    );

    engine->get_dialogs().add_thought(intro_text);

    for (int i = 0; i < 5; i++) {
        napms(20);

        std::string text = "Hello ";
        text += std::to_string(i);

        history = engine->get_dialogs().get_thoughts();

        engine->get_layout().type_new_text(
            engine->get_layout().win_thought,
            "Thoughts & Desc",
            engine->get_layout().w_left,
            history,
            text
        );

        engine->get_dialogs().add_thought(text);
    }
}

void TownState::handle_input(int ch) {
    if (ch == 'q') {
        engine->quit(); 
    } 
    else if (ch == KEY_RESIZE) {
        // Handle terminal resize
        engine->get_layout().resize();
    }
}

void TownState::update() {
    
}

void TownState::render() {
    // 1. BASE RENDER (Draws borders and backgrounds)
    engine->get_layout().draw();

    // 2. PLAYER DATA FETCH
    Player* p = engine->get_player_manager().get_player();
    if (!p) return;

    // 3. THOUGHTS RENDER
    auto thoughts = engine->get_dialogs().get_thoughts();
    engine->get_layout().render_history(engine->get_layout().win_thought, thoughts);

    // 4. STAT BLOCK RENDER
    engine->get_layout().draw_player_stats(
        engine->get_layout().win_stat,
        p->get_str(), p->get_cons(), p->get_agi(), p->get_intl(), p->get_wis(),
        element_to_string(p->get_affinity())
    );

    // 5. HP/MP/STAMINA RENDER
    engine->get_layout().draw_vitals(
        engine->get_layout().win_hp,
        p->get_hp(), p->get_max_hp(),
        p->get_mp(), p->get_max_mp(),
        p->get_stamina(), p->get_max_stamina()
    );

    // 6. CALENDAR RENDER
    int current_month = engine->get_calendar().getMonth();
    int current_day = engine->get_calendar().getDay();
    std::string current_time = engine->get_calendar().getTimeString();
    int days_left = 15 - current_day; 
    if (days_left < 0) days_left = 0;

    engine->get_layout().draw_calendar(
        engine->get_layout().win_cal, 
        days_left, current_month, current_day, current_time
    );

    // 7. INVENTORY RENDER
    std::vector<std::string> item_names;
    for (auto* item : p->get_inventory()) item_names.push_back("Unknown Item");
    engine->get_layout().draw_inventory(engine->get_layout().win_menu, item_names, p->get_gold());

    // 8. TASK LIST RENDER (Quests)
    std::vector<std::string> quest_list;
    for (auto& pair : engine->get_quests().get_all_quests()) {
        Quest* q = pair.second;
        if (q && (q->get_state() == QuestState::AVAILABLE || q->get_state() == QuestState::IN_PROGRESS)) {
            quest_list.push_back(q->get_id());
        }
    }
    engine->get_layout().draw_tasks(engine->get_layout().win_task, quest_list);
}