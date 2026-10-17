// ui.h


#include "character.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>


enum Pages {
    MAIN_MENU,
    PAGE_ONE,
    LOAD_PAGE,
    CHARACTER_CREATION,
    STATS_PAGE
};

// Function prototypes for individual page creators
ftxui::Component CreateMainMenu(int* selected_page);
ftxui::Component CreatePageOne(int* selected_page);
ftxui::Component CreateLoadPage(int* selected_page, Character*);
ftxui::Component CreateCharacterCreationPage(int* selected_page, Character* character);
ftxui::Component CreateStatsPage(int* selected_page, Character* character,ftxui::Closure exit_closure);

// Main UI creator function
ftxui::Component CreateUI(int* selected_page, Character* character,ftxui::Closure exit_closure);