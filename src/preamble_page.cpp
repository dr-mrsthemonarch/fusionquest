#include "../include/GameEngine.h"
#include "../include/ui.h"
#include "../include/functions.h"
#include <algorithm>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// shown once after character creation. any key -> stats page. Up/Down/PgUp/PgDn scroll.
Component CreatePreamblePage(int* selected_page, GameEngine* engine) {
    auto scroll_pos = std::make_shared<float>(0.0f);
    auto begin_button = Button("Begin", [selected_page] { *selected_page = STATS_PAGE; });

    auto renderer = Renderer(begin_button, [=] {
        Elements lines;
        for (const auto& l : splitLines(engine->getQuestSystem()->preamble()))
            lines.push_back(l.empty() ? text(" ") : paragraph(l));
        if (lines.empty()) lines.push_back(text("..."));
        return window(text("Fusion Quest") | bold | center,
                      vbox({
                          vbox(std::move(lines)) | focusPositionRelative(0.0f, *scroll_pos) |
                              vscroll_indicator | yframe | flex,
                          separator(),
                          text("Press any key to begin   (Up/Down scroll)") | dim | center,
                          begin_button->Render() | center,
                      }));
    });

    return CatchEvent(renderer, [=](Event e) {
        auto move = [&](float d) { *scroll_pos = std::clamp(*scroll_pos + d, 0.0f, 1.0f); return true; };
        if (e == Event::ArrowUp) return move(-0.1f);
        if (e == Event::ArrowDown) return move(0.1f);
        if (e == Event::PageUp) return move(-0.5f);
        if (e == Event::PageDown) return move(0.5f);
        // ignore mouse + the 100ms Custom refresh event
        if (e.is_character() || e == Event::Return || e == Event::Escape || e == Event::Tab ||
            e == Event::Backspace || e == Event::ArrowLeft || e == Event::ArrowRight) {
            *selected_page = STATS_PAGE;
            return true;
        }
        return false;
    });
}
