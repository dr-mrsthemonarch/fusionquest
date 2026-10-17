// character_creation.cpp
#include "../include/ui.h"
#include "../include/GameEngine.h"
#include "../include/globals.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// NOTE: SPG Character's starting stats are fixed in its ctor (baseProfile_ constants),
// not player-rollable like fusionquest's placeholder Character was. "Roll!" button kept
// for layout parity but now just re-renders the fixed preview (no randomization target
// exists on the real Character model). name/race/fusion selection still meaningful -
// feed into GameEngine::initializeCharacter on submit.

Component CreateCharacterCreationPage(int* selected_page, GameEngine* engine) {
    static std::string name;
    static int race_index = 0;
    static int class_index = 0;
    static int fusion_index = 0;

    auto name_input = Input(&name, "Enter character name");
    auto race_selector = Radiobox(&RACES, &race_index);
    auto fusion_selector = Radiobox(&FUSION_STARTUPS, &fusion_index);

    auto roll_button = Button("Roll!", [] { /* no-op: SPG Character stats fixed at ctor, see note above */ });

    auto lets_do_it_button = Button("Let's Do It", [selected_page, engine] {
        if (name.empty()) name = "Unnamed Founder";
        engine->initializeCharacter(name, "1", CLASSES[class_index], FUSION_STARTUPS[fusion_index]);
        *selected_page = STATS_PAGE;
    });

    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; }) | size(HEIGHT, EQUAL, 4);

    auto race_container = Container::Vertical({race_selector});
    auto fusion_container = Container::Vertical({fusion_selector});

    auto race_renderer = Renderer(race_container, [race_selector] {
        return vbox({
            text("Race") | bold | center,
            separator(),
            race_selector->Render() | vscroll_indicator | frame
        }) | border | size(HEIGHT, LESS_THAN, 30);
    });

    auto fusion_renderer = Renderer(fusion_container, [fusion_selector] {
        return vbox({
            text("Fusion Company") | bold | center,
            separator(),
            fusion_selector->Render() | flex | vscroll_indicator | frame
        }) | border | size(HEIGHT, LESS_THAN, 30);
    });

    auto container = Container::Vertical({
        name_input,
        race_container,
        fusion_container,
        roll_button,
        lets_do_it_button,
        back_button,
    });

    auto renderer = Renderer(container, [=] {
        Element race_column = race_renderer->Render();
        Element fusion_column = fusion_renderer->Render();

        // preview of fixed starting stats (Character ctor constants) - not yet-created character
        Element stats_column = vbox({
            text("Starting Stats") | bold | center,
            separator(),
            text("Runway: 80") | bold,
            text("Hype Energy: 10") | bold,
            text("Hustle: 20") | bold,
            text("Agile-ity: 10") | bold,
            text("NDAs: 9") | bold,
            text("Luck: 5") | bold,
            text("Synergy: 5") | bold,
            text("Technical Debt: -10") | bold,
            separator(),
            roll_button->Render(),
            lets_do_it_button->Render(),
        }) | border;

        return window(text("Fusion Quest") | bold | center, vbox({
            hbox({text("Name: ") | bold, name_input->Render()}),
            separator(),
            hbox({race_column | flex, fusion_column | flex, stats_column | flex}),
            separator(),
            back_button->Render(),
        }));
    });

    return renderer;
}
