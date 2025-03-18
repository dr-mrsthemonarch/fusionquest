#ifndef PAGE_TEMPLATE_H
#define PAGE_TEMPLATE_H

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <functional>
#include <string>
#include <vector>

using namespace ftxui;

// Template for creating a general page
Component CreatePage(
    const std::string& title,                      // Title of the page
    const std::vector<std::string>& content,       // Content to display
    std::function<void()> on_back                  // Callback for the "Back" button
) {
    // Back button
    auto back_button = Button("Back", on_back);

    // Container for the page
    auto page_container = Container::Vertical({
        back_button,
    });

    // Renderer for the page
    return Renderer(page_container, [title, content, back_button] {
        Elements content_elements;
        for (const auto& line : content) {
            content_elements.push_back(text(line));
        }

        return vbox({
            text(title) | hcenter | bold,
            separator(),
            vbox(content_elements),
            hbox({
                back_button->Render(),
            }) | center,
        }) | border | center;
    });
}

#endif // PAGE_TEMPLATE_H