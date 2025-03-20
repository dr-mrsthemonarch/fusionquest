// character_creation.cpp
#include "ui.h"
#include "character.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// Static lists for character creation
const std::vector<std::string> RACES = {"Human", "Elf", "Dwarf", "Orc"};
const std::vector<std::string> CLASSES = {"Warrior", "Mage", "Rogue", "Priest"};

// Implementation of the stat roller function
int RollStat() {
    return (std::rand() % 21) + 1;
}

Component CreateCharacterCreationPage(int* selected_page, Character* character) {
    auto name_input = Input(&character->name, "Enter character name");
    auto race_selector = Radiobox(&RACES, &character->race_index);
    auto class_selector = Radiobox(&CLASSES, &character->class_index);

    auto roll_button = Button("Roll!", [character] {
        character->strength = RollStat();
        character->dexterity = RollStat();
        character->intelligence = RollStat();
        character->constitution = RollStat();
    });

    auto lets_do_it_button = Button("Let's Do It", [] {
        // Placeholder for character confirmation logic
    });

    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; });

    auto container = Container::Vertical({
        name_input,
        race_selector,
        class_selector,
        roll_button,
        lets_do_it_button,
        back_button,
    });

    auto renderer = Renderer(container, [container, name_input, race_selector, class_selector, roll_button, lets_do_it_button, back_button, character] {
        Element race_column = vbox({
            text("Race") | bold | center,
            separator(),
            race_selector->Render(),
        }) | border;

        Element class_column = vbox({
            text("Class") | bold | center,
            separator(),
            class_selector->Render(),
        }) | border;

        Element stats_column = vbox({
            text("Stats") | bold | center,
            separator(),
            text("Strength: " + std::to_string(character->strength)) | center,
            text("Dexterity: " + std::to_string(character->dexterity)) | center,
            text("Intelligence: " + std::to_string(character->intelligence)) | center,
            text("Constitution: " + std::to_string(character->constitution)) | center,
            separator(),
            text("Total: " + std::to_string(character->total_stats())) | bold | center,
            separator(),
            roll_button->Render(),
            lets_do_it_button->Render(),
        }) | border;

        return window(text("Fusion Quest") | bold | center, vbox({
            hbox({ text("Name: ") | bold, name_input->Render() }),
            separator(),
            hbox({ race_column | flex, class_column | flex, stats_column | flex }),
            separator(),
            back_button->Render(),
        }));
    });

    return renderer;
}