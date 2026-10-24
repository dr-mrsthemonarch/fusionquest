// ui.cpp
#include "../include/ui.h"
#include "../include/GameEngine.h"
#include "../include/GameClock.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

using namespace ftxui;

Component CreateUI(int* selected_page, GameEngine* engine,
                    std::shared_ptr<CountdownComponent> countdown_timer,
                    Closure exit_closure) {
    auto main_menu = CreateMainMenu(selected_page);
    auto load_page = CreateLoadPage(selected_page, engine, countdown_timer);
    auto character_creation = CreateCharacterCreationPage(selected_page, engine);
    auto stats_page = CreateStatsPage(selected_page, engine, countdown_timer, exit_closure);
    auto battle_page = CreateBattlePage(selected_page, engine, exit_closure);
    auto equipment_page = CreateEquipmentPage(selected_page, engine);
    auto special_drops_page = CreateSpecialDropsPage(selected_page, engine, exit_closure);
    auto preamble_page = CreatePreamblePage(selected_page, engine);
    auto final_warning_page = CreateFinalWarningPage(selected_page, engine);

    auto pages = Container::Tab(
        {
            main_menu,
            load_page,
            character_creation,
            stats_page,
            battle_page,
            equipment_page,
            special_drops_page,
            preamble_page,
            final_warning_page,
        },
        selected_page
    );

    return pages;
}
