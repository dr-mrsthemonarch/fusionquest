#ifndef SECOND_PAGE_H
#define SECOND_PAGE_H

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <functional>
#include <utility>

#include "functions.h"

using namespace ftxui;

// Function to create the second page (poem) components and renderer
Component CreateSecondPage(std::function<void()> on_back) {
    // Components for the second page (poem)
    auto back_button = Button("Back",
        std::move(on_back)
    );

    // Container for the second page
    auto second_page = Container::Horizontal({
        back_button,
    });

    // Renderer for the second page (poem)
    return Renderer(second_page, [back_button] {
        return vbox({
            text("A Poem:"),
            text("Roses are red,"),
            text("Violets are blue,"),
            text("Sugar is sweet,"),
            text("And so are you!"),
            hbox({
                back_button->Render(),
            }),
        }) | border | center;
    });
}

#endif // SECOND_PAGE_H