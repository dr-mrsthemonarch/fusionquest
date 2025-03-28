// main_menu.cpp
#include "ui.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

Component CreateMainMenu(int* selected_page) {
    auto main_menu = Container::Vertical({
        Button("Start New Game", [selected_page] { *selected_page = CHARACTER_CREATION; }),
        Button("Continue Game", [selected_page] { *selected_page = PAGE_TWO; }),
        Button("Character Creation", [selected_page] { *selected_page = CHARACTER_CREATION; }),
        Button("Stats Page", [selected_page] { *selected_page = STATS_PAGE; }),
    });

    auto renderer = Renderer(main_menu, [main_menu] {
        return vbox({
            text("Main Menu") | bold | center,
            separator(),
            main_menu->Render(),
        });
    });

    return renderer;
}