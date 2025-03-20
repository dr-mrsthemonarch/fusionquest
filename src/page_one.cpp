// page_one.cpp
#include "ui.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

Component CreatePageOne(int* selected_page) {
    auto page_one = Container::Vertical({
        Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; }),
    });

    auto renderer = Renderer(page_one, [page_one] {
        return vbox({
            text("Page 1") | bold | center,
            separator(),
            text("Welcome to Page 1!\nThis is the first page.") | center | color(Color::Blue),
            page_one->Render(),
        });
    });

    return renderer;
}