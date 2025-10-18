// character_creation.cpp
#include "ui.h"
#include "../classes/functions.h"
#include "../classes/character.h"
#include "../classes/globals.h"
#include "../core/engine_core.h"
#include "../core/components/character_component.h"
#include "../core/components/equipment_component.h"
#include "../core/components/inventory_component.h"
#include "../core/components/quest_component.h"
#include "../core/components/stats_component.h"
#include "../core/systems/GameEngineBridge.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <random>

using namespace ftxui;

Component CreateCharacterCreationPage(int* selected_page, Character* character) {
    auto name_input = Input(&character->name, "Enter character name");

    auto race_selector = Radiobox(&RACES, &character->race_index);
    auto class_selector = Radiobox(&CLASSES, &character->class_index);
    auto fusion_selector = Radiobox(&FUSION_STARTUPS, &character->fusion_index);

    // Modified roll button to use the engine
    auto roll_button = Button("Roll!", [character] {
        // Initialize engine if not already done
        if (!FusionQuest::GetGameEngine()) {
            FusionQuest::InitializeGameEngine();
        }

        // Create or update player object
        auto* gameEngine = FusionQuest::GetGameEngine();
        auto* player = gameEngine->createPlayerFromCharacter(*character);

        // Roll stats using the engine
        gameEngine->rollStats();

        // Update the UI character from the game object
        gameEngine->updateCharacterFromPlayer(*character);
    });

    // Modified "Let's Do It" button to use the engine for saving
    auto lets_do_it_button = Button("Let's Do It", [selected_page, character] {
        // Make sure engine is initialized
        if (!FusionQuest::GetGameEngine()) {
            FusionQuest::InitializeGameEngine();
        }

        auto* gameEngine = FusionQuest::GetGameEngine();

        // Create or update player object
        auto* player = gameEngine->createPlayerFromCharacter(*character);

        // Create a filename based on character name (sanitized)
        std::string filename = character->name;
        for (char& c : filename) {
            if (!isalnum(c)) {
                c = '_';
            }
        }
        filename = "saves/" + filename + ".sav";

        // Save using the engine's save system
        gameEngine->getSaveSystem()->saveCharacter(player, filename);

        // Proceed to stats page
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
        }) | border | size(HEIGHT,LESS_THAN,30);
    });

    auto fusion_renderer = Renderer(fusion_container, [fusion_selector] {
        return vbox({
            text("Fusion Company") | bold | center,
            separator(),
            fusion_selector->Render() | flex | vscroll_indicator | frame
        }) | border| size(HEIGHT,LESS_THAN,30);
    });

    auto container = Container::Vertical({
        name_input,
        race_container,
        fusion_container,
        roll_button,
        lets_do_it_button,
        back_button,
    });

    auto renderer = Renderer(container, [container, name_input, race_renderer, fusion_renderer, roll_button, lets_do_it_button, back_button, character] {
        Element race_column = race_renderer->Render();
        Element fusion_column = fusion_renderer->Render();

        Element stats_column = vbox({
            text("Stats") | bold | center,
            separator(),
            text("NDAs: " + std::to_string(character->ndas)) | bold,
            text("Luck: " + std::to_string(character->luck)) | bold,
            text("Hustle: " + std::to_string(character->hustle)) | bold,
            text("Synergy: " + std::to_string(character->Synergy)) | bold,
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
            hbox({ race_column | flex, fusion_column | flex, stats_column | flex }),
            separator(),
            back_button->Render(),
        }));
    });

    return renderer;
}