// main_menu.cpp
#include "ui.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

Component CreateMainMenu(int* selected_page) {
    auto main_menu = Container::Vertical({
        Button("Start New Game", [selected_page] { *selected_page = CHARACTER_CREATION; }) |size(WIDTH,EQUAL,20),
        Button("Continue Game", [selected_page] { *selected_page = LOAD_PAGE; })|size(WIDTH,EQUAL,20),
        Button("Character Creation", [selected_page] { *selected_page = CHARACTER_CREATION; })|size(WIDTH,EQUAL,20),
        Button("Stats Page", [selected_page] { *selected_page = STATS_PAGE; })|size(WIDTH,EQUAL,20),
    });

    auto renderer = Renderer(main_menu, [main_menu] {
        return vbox({
            text("Main Menu") | bold | center,
            separator(),
            main_menu->Render() |center,
        });
    });

    return renderer;
}