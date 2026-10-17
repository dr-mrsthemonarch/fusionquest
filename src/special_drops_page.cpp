#include "../include/GameEngine.h"
#include "../include/ui.h"
#include "../include/globals.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

Component CreateSpecialDropsPage(int* selected_page, GameEngine* engine) {
    static int selected_index = 0;
    auto drop_names = std::make_shared<std::vector<std::string>>();
    auto menu = Menu(drop_names.get(), &selected_index);

    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; });
    auto container = Container::Horizontal({menu, back_button});

    auto renderer = Renderer(container, [=] {
        const auto& lib = engine->getSpecialDropsLibrary().getLibrary();
        drop_names->clear();
        for (const auto& drop : lib) {
            drop_names->push_back(drop.name + (drop.isEquipped ? " [EQUIPPED]" : ""));
        }
        if (drop_names->empty()) drop_names->push_back("No special drops yet.");

        Element detail = text("Select an item to view details...") | dim;
        if (selected_index >= 0 && selected_index < static_cast<int>(lib.size())) {
            const auto& d = lib[selected_index];
            const auto& eq = d.equipment;
            detail = vbox({
                text(d.name) | bold,
                separator(),
                text("From: " + d.acquiredFrom),
                text("Acquired: Year " + std::to_string(d.yearAcquired) + ", Month " + std::to_string(d.monthAcquired)),
                separator(),
                text("Runway +" + std::to_string(eq.runway)),
                text("Hype +" + std::to_string(eq.hypeEnergy)),
                text("Hustle +" + std::to_string(eq.hustle)),
                text("Agility +" + std::to_string(eq.agility)),
                text("NDAs +" + std::to_string(eq.ndas)),
                text("Luck +" + std::to_string(eq.luck)),
                text("Synergy +" + std::to_string(eq.synergy)),
                separator(),
                text(d.loreText) | dim,
                text("UNSELLABLE - kept forever") | color(Color::Red),
            });
        }

        return window(text("Special Drops Library") | bold | center, vbox({
            hbox({
                vbox({menu->Render() | vscroll_indicator | frame}) | border | flex,
                vbox({detail}) | border | flex,
            }) | flex,
            separator(),
            back_button->Render(),
        }));
    });

    return renderer;
}
