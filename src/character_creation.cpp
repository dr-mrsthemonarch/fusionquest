// character_creation.cpp
#include "ui.h"
#include "character.h"
#include "globals.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "functions.h"
#include <random>

using namespace ftxui;



Component CreateCharacterCreationPage(int* selected_page, Character* character) {
    auto name_input = Input(&character->name, "Enter character name");

    // Use `Menu` instead of `Radiobox` for scrolling support
    auto race_selector = Radiobox(&RACES, &character->race_index);
    auto class_selector = Radiobox(&CLASSES, &character->class_index);
    auto fusion_selector = Radiobox(&FUSION_STARTUPS, &character->fusion_index);

    auto roll_button = Button("Roll!", [character] {
        character->hustle = RollStat(21);
        character->agility = RollStat(21);
        character->busfactor = RollChance();
        character->techdebt = RollStat(21)*-1;
        character->ndas = RollStat(21);
        character->luck = RollStat(21);
        character->patents = RollStat(1);
    });

    auto lets_do_it_button = Button("Let's Do It", [selected_page] {
        *selected_page = STATS_PAGE;
    });

    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; }) | size(HEIGHT,EQUAL,4);

    // Wrap race selection in a scrollable container
    auto race_container = Container::Vertical({race_selector});
    auto fusion_container = Container::Vertical({fusion_selector});

    // Ensure race selection is scrollable
    auto race_renderer = Renderer(race_container, [race_selector] {
        return vbox({
            text("Race") | bold | center,
            separator(),
            race_selector->Render() | vscroll_indicator | frame  // Enable scrolling
        }) | border;
    });

    // Ensure fusion selection is scrollable
    auto fusion_renderer = Renderer(fusion_container, [fusion_selector] {
        return vbox({
            text("Fusion Company") | bold | center,
            separator(),
            fusion_selector->Render() |flex| vscroll_indicator | frame  // Enable scrolling
        }) | border;
    });

    auto container = Container::Vertical({
        name_input,
        race_container,  // Make sure race selection is inside main container
        fusion_container,
        roll_button,
        lets_do_it_button,
        back_button,
    });

    auto renderer = Renderer(container, [container, name_input, race_renderer, fusion_renderer, roll_button, lets_do_it_button, back_button, character] {
        Element race_column = race_renderer->Render();  // Use scrollable race selector
        Element fusion_column = fusion_renderer->Render();  // Use scrollable race selector


        Element stats_column = vbox({
            text("Stats") | bold | center,
            separator(),
            text("NDAs: " + std::to_string(character->ndas)) | bold,
            text("Luck: " + std::to_string(character->luck)) | bold,
            text("Hustle: " + std::to_string(character->hustle)) | bold | selectionColor(Color::Green),
            text("Patents: " + std::to_string(character->luck)) | bold,
            text("Agile-ity: " + std::to_string(character->agility)) | bold,
            text("Bus Factor: " + std::to_string(character->busfactor)) | bold,
            text("Technical Debt: " + std::to_string(character->techdebt)) | bold,

            separator(),
            text("Total: " + std::to_string(character->total_stats())) | bold | center,
            separator(),
            roll_button->Render(),
            lets_do_it_button->Render(),
        }) | border;

        return window(text("Fusion Quest") | bold | center, vbox({
            hbox({ text("Name: ") | bold, name_input->Render() }),
            separator(),
            hbox({ race_column | flex, fusion_column | flex, stats_column | flex }),  // Use updated race column
            separator(),
            back_button->Render(),
        }));
    });

    return renderer;
}
