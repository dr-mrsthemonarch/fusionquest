// ui.h


#include "Character.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <memory>
class GameEngine;
class CountdownComponent;

enum Pages {
    MAIN_MENU,
    LOAD_PAGE,
    CHARACTER_CREATION,
    STATS_PAGE,
    BATTLE_PAGE,
    EQUIPMENT_PAGE,
    SPECIAL_DROPS_PAGE
};

// Function prototypes for individual page creators
ftxui::Component CreateMainMenu(int* selected_page);
ftxui::Component CreateLoadPage(int* selected_page, Character*);
ftxui::Component CreateStatsPage(int* selected_page, Character* character,ftxui::Closure exit_closure);

// Main UI creator function
ftxui::Component CreateUI(int* selected_page, Character* character,ftxui::Closure exit_closure);

ftxui::Component CreateUI(int* selected_page, GameEngine* engine,
                           std::shared_ptr<CountdownComponent> countdown_timer,
                           ftxui::Closure exit_closure);

ftxui::Component CreateCharacterCreationPage(int* selected_page, GameEngine* engine); // was Character*

ftxui::Component CreateStatsPage(int* selected_page, GameEngine* engine,
                                  std::shared_ptr<CountdownComponent> countdown_timer,
                                  ftxui::Closure exit_closure); // was Character*, owned its own timer

ftxui::Component CreateLoadPage(int* selected_page, GameEngine* engine,
                                 std::shared_ptr<CountdownComponent> countdown_timer); // was Character*

// new
ftxui::Component CreateBattlePage(int* selected_page, GameEngine* engine);
ftxui::Component CreateEquipmentPage(int* selected_page, GameEngine* engine);
ftxui::Component CreateSpecialDropsPage(int* selected_page, GameEngine* engine);
