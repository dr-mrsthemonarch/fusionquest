// ui.cpp
#include "ui.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

using namespace ftxui;

Component CreateUI(int* selected_page, Character* character,Closure exit_closure) {
    // Create individual pages
    auto main_menu = CreateMainMenu(selected_page);
    auto page_one = CreatePageOne(selected_page);
    auto load_page = CreateLoadPage(selected_page, character);
    auto character_creation = CreateCharacterCreationPage(selected_page, character);
    auto stats_page = CreateStatsPage(selected_page, character,std::move(exit_closure));

    // Create UI with navigation
    auto pages = Container::Tab(
        {
            main_menu,
            page_one,
            load_page,
            character_creation,
            stats_page
        },
        selected_page
    );

    return pages;
}