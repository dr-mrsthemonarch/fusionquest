#ifndef SECOND_PAGE_H
#define SECOND_PAGE_H

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

using namespace ftxui;

// Function to create the second page (poem page)
Component SecondPage(std::function<void()> on_back, std::function<void()> open_modal) {
    auto back_button = Button("Back", [on_back, open_modal] {
        on_back(); // Switch back to the main page
        open_modal(); // Reopen the modal (set depth = 1)
    });

    return Renderer(back_button, [back_button] {
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