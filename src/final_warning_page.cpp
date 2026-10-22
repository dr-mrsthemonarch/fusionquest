#include "../include/GameEngine.h"
#include "../include/ui.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// final quest popup. engine tick is paused until Continue. Save button is FAKE (does nothing).
Component CreateFinalWarningPage(int* selected_page, GameEngine* engine) {
    auto msg = std::make_shared<std::string>();
    auto save_button = Button("Save Game", [msg] { *msg = "Game saved."; });  // fake on purpose
    auto continue_button = Button("Continue", [selected_page, engine, msg] {
        msg->clear();
        engine->acknowledgeFinal();
        *selected_page = STATS_PAGE;
    });
    auto container = Container::Horizontal({save_button, continue_button});

    return Renderer(container, [=] {
        const Quest& q = engine->getQuestSystem()->state().main;
        return vbox({
                   text("!! FINAL QUEST !!") | bold | center | color(Color::Red),
                   separator(),
                   text(q.title) | bold | center,
                   paragraph(q.story),
                   separator(),
                   paragraph("Warning: there is no going back. Save your game before you continue."),
                   text(*msg) | center | color(Color::Green),
                   hbox({save_button->Render(), text("  "), continue_button->Render()}) | center,
               }) |
               border | size(WIDTH, LESS_THAN, 70) | center;
    });
}
