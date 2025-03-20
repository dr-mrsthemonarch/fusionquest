// ui.h
#pragma once

#include "character.h"
#include <ftxui/component/component.hpp>

enum Pages {
    MAIN_MENU,
    PAGE_ONE,
    PAGE_TWO,
    CHARACTER_CREATION,
    STATS_PAGE
};

// Function prototypes for individual page creators
ftxui::Component CreateMainMenu(int* selected_page);
ftxui::Component CreatePageOne(int* selected_page);
ftxui::Component CreatePageTwo(int* selected_page);
ftxui::Component CreateCharacterCreationPage(int* selected_page, Character* character);
ftxui::Component CreateStatsPage(int* selected_page, Character* character);

// Main UI creator function
ftxui::Component CreateUI(int* selected_page, Character* character);