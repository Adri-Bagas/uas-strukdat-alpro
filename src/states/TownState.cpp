#include "TownState.hpp"
#include "DungeonState.hpp"
#include "../utils/components/Popup.hpp"
#include "../GameEngine.hpp" // Full include allowed in .cpp
#include "../utils/Logger.hpp"
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

void execute_dialog_action(GameEngine* engine, const std::string& action) {
    if (action.empty()) return;
    engine->get_dialogs().execute_actions({action}, engine);
}

void TownState::on_enter() {
    Logger::log("TownState: Entering state.");
    selection_index = 0;
    Place* cur = engine->get_places().get_current_place();
    if (!cur) return;

    engine->get_places().update_npc_locations(engine->get_calendar(), engine->get_quests());
    this->render();

    std::string scene_id = cur->get_on_enter();
    if (!cur->get_has_entered()) {
        if (!cur->get_on_first_enter().empty()) {
            scene_id = cur->get_on_first_enter();
        }
        cur->set_has_entered(true);
    }

    if (!scene_id.empty()) {
        const DialogScene* scene = engine->get_db().get_dialog_scene(scene_id);
        if (scene) {
            engine->get_dialogs().start_scene(*scene, engine);
        } else {
            Logger::log("TownState ERROR: Entry scene '" + scene_id + "' not found!");
        }
    }
}

void TownState::handle_input(int ch) {
    if (ch == 'q') {
        engine->quit(); 
        return;
    } 
    else if (ch == KEY_RESIZE) {
        engine->get_layout().resize();
        return;
    }

    if (engine->get_dialogs().has_active_choices()) {
        int idx = engine->get_dialogs().get_selected_choice_index();
        int count = engine->get_dialogs().get_active_choices().size();
        if (ch == KEY_UP || ch == 'w') {
            if (idx > 0) engine->get_dialogs().set_selected_choice_index(idx - 1);
        }
        else if (ch == KEY_DOWN || ch == 's') {
            if (idx < count - 1) engine->get_dialogs().set_selected_choice_index(idx + 1);
        }
        else if (ch == '\n' || ch == ' ') {
            engine->get_dialogs().select_choice(idx, engine);
        }
        return;
    }

    if (ch == KEY_UP || ch == 'w') {
        if (selection_index > 0) selection_index--;
    }
    else if (ch == KEY_DOWN || ch == 's') {
        if (selection_index < (int)current_activities.size() - 1) selection_index++;
    }
    else if (ch == '\n' || ch == ' ') {
        if (selection_index >= 0 && selection_index < (int)current_activities.size()) {
            const auto& act = current_activities[selection_index];
            Logger::log("TownState: Executing activity " + act.id);
            
            if (act.id == "masuk_dungeon") {
                engine->push_state(new DungeonState(engine));
                return;
            }

            if (act.id.rfind("travel_", 0) == 0) {
                std::string dest_id = act.id.substr(7);
                engine->get_places().set_current_place(dest_id);
                on_enter();
                return;
            }

            if (act.id == "ke_permukiman_kumuh") {
                const DialogScene* scene = engine->get_db().get_dialog_scene("hub_permukiman_kumuh");
                if (scene) {
                    engine->get_dialogs().start_scene(*scene, engine);
                } else {
                    Logger::log("TownState ERROR: hub_permukiman_kumuh scene not found!");
                }
                return;
            }
            
            // Apply effects
            Player* p = engine->get_player_manager().get_player();
            if (p) {
                p->add_gold(act.reward_gold);
                // p->add_str(act.reward_str); // Add stat helpers to Player if needed
            }
            
            for (int i=0; i<act.time_cost; ++i) engine->get_calendar().advanceTime(false);
            
            if (!act.finish_quest_id.empty()) {
                // engine->get_quests().finish_quest(act.finish_quest_id);
            }
        }
    }
}

void TownState::update() {
    if (engine->get_dialogs().has_queued_dialog()) {
        Logger::log("TownState: Processing dialog queue node.");
        DialogNode node = engine->get_dialogs().pop_dialog();
        
        if (node.type == 1) {
            engine->get_layout().type_new_text(
                engine->get_layout().win_dialog, "In World Dialog",
                engine->get_layout().w_left, engine->get_dialogs().get_dialog(), node
            );
            engine->get_dialogs().add_dialog(node);
        } else if (node.type == 2) {
            engine->get_layout().type_new_text(
                engine->get_layout().win_thought, "Thoughts",
                engine->get_layout().w_left, engine->get_dialogs().get_thoughts(), node
            );
            engine->get_dialogs().add_thought(node);
        } else if (node.type == 3) {
            Popup otp {node.value};
            otp.animate();
            otp.type_text();
            engine->get_dialogs().add_popup(node);
        }
        
        this->render();

        if (!engine->get_dialogs().has_queued_dialog()) {
            DialogNode separator {"--------------------------------", "", 0};
            engine->get_dialogs().add_dialog(separator);
            engine->get_dialogs().add_thought(separator);

            engine->get_dialogs().execute_actions(engine->get_dialogs().get_on_exit(), engine);

            if (engine->get_dialogs().has_pending_choices()) {
                engine->get_dialogs().activate_choices(engine);
            } else {
                std::string next_id = engine->get_dialogs().get_next_scene();
                if (!next_id.empty()) {
                    const DialogScene* next_scene = engine->get_db().get_dialog_scene(next_id);
                    if (next_scene) {
                        engine->get_dialogs().start_scene(*next_scene, engine);
                    }
                } else {
                    engine->get_dialogs().set_on_exit({});
                    engine->get_dialogs().set_next_scene("");
                }
            }
        }
    }
}

void TownState::render() {
    engine->get_layout().draw();

    Player* p = engine->get_player_manager().get_player();
    if (!p) return;

    engine->get_layout().draw_player_stats(
        engine->get_layout().win_stat,
        p->get_str(), p->get_cons(), p->get_agi(), p->get_intl(), p->get_wis(),
        element_to_string(p->get_affinity()),
        p->get_gold()
    );

    engine->get_layout().draw_vitals(
        engine->get_layout().win_hp,
        p->get_hp(), p->get_max_hp(),
        p->get_mp(), p->get_max_mp(),
        p->get_stamina(), p->get_max_stamina()
    );

    int current_month = engine->get_calendar().getMonth();
    int current_day = engine->get_calendar().getDay();
    std::string current_time = engine->get_calendar().getTimeString();
    int days_left = 15 - current_day; 
    if (days_left < 0) days_left = 0;

    engine->get_layout().draw_calendar(
        engine->get_layout().win_cal, 
        days_left, current_month, current_day, current_time
    );

    std::vector<std::string> item_names;
    for (auto* item : p->get_inventory()) {
        if (item) {
            item_names.push_back(item->get_name());
        }
    }
    engine->get_layout().draw_inventory(engine->get_layout().win_menu, item_names);

    // 8. TASK LIST RENDER (Quests & Activities)
    std::vector<std::string> display_list;
    current_activities.clear();

    Place* cur = engine->get_places().get_current_place();
    if (cur) {
        int day = engine->get_calendar().getDay();
        std::string phase = engine->get_calendar().getTimeString();

        for (const auto& act : cur->get_activities()) {
            bool day_ok = act.days.empty();
            for (int d : act.days) if (d == day) day_ok = true;

            bool phase_ok = act.phases.empty();
            for (const auto& ph : act.phases) if (ph == phase) phase_ok = true;

            if (day_ok && phase_ok && !act.is_locked) {
                current_activities.push_back(act);
                std::string prefix = (selection_index == (int)display_list.size()) ? "> " : "  ";
                display_list.push_back(prefix + act.name);
            }
        }

        for (const auto* p_walkable : cur->get_walkable_places()) {
            if (p_walkable) {
                Activity travel_act;
                travel_act.id = "travel_" + p_walkable->get_id();
                travel_act.name = "Travel to " + p_walkable->get_name();
                travel_act.time_cost = 0;
                travel_act.stamina_cost = 0;

                current_activities.push_back(travel_act);
                std::string prefix = (selection_index == (int)display_list.size()) ? "> " : "  ";
                display_list.push_back(prefix + travel_act.name);
            }
        }
    }

    for (auto& pair : engine->get_quests().get_all_quests()) {
        Quest* q = pair.second;
        if (q && (q->get_state() == QuestState::AVAILABLE || q->get_state() == QuestState::IN_PROGRESS)) {
            display_list.push_back("Quest: " + q->get_id());
        }
    }
    engine->get_layout().draw_tasks(engine->get_layout().win_task, display_list);

    auto thoughts_log = engine->get_dialogs().get_thoughts();
    auto dialog_log = engine->get_dialogs().get_dialog();

    engine->get_layout().render_history(engine->get_layout().win_thought, thoughts_log);
    if (engine->get_dialogs().has_active_choices()) {
        engine->get_layout().draw_choices(
            engine->get_layout().win_dialog,
            engine->get_dialogs().get_active_choices(),
            engine->get_dialogs().get_selected_choice_index()
        );
    } else {
        engine->get_layout().render_history(engine->get_layout().win_dialog, dialog_log);
    }
}