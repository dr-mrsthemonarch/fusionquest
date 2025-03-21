// stats_page.cpp
#include "ui.h"
#include "character.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

Component CreateStatsPage(int *selected_page, Character *character) {
    // These would typically come from the character class
    float experience_value = 0.45f; // Example value
    float progress_value = 0.65f; // Example value

    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; });
    auto save_button = Button("Save", [selected_page] { *selected_page = MAIN_MENU; });
    // auto quit_button = Button("Quit", [selected_page], exit(0));

    auto container = Container::Vertical({
        back_button,
        save_button
    });

    auto renderer = Renderer(container, [back_button, save_button,character, experience_value, progress_value] {
        // Column 1: Character Stats
        Element character_stats = vbox({
                                      text("Character") | bold | center,
                                      separator(),
                                      hbox({
                                          text("Name: ") | bold,
                                          text(character->name.empty() ? "Unknown" : character->name)
                                      }),
                                      hbox({text("Employee nr: ") | bold, text("57")}),
                                      hbox({text("Position: ") | bold, text("Rocket Biologist")}),

                                      hbox({text("Startup Name: ") | bold, text("Marbel Fusion")}),
                                      // hbox({ text("Class: ") | bold, text(GetClassName(character->class_index)) }),
                                      separator(),
                                      hbox({
                                          text("Runway: ") | bold,
                                          text(std::to_string(int(character->health * 100)) + "/100")
                                      }),
                                      hbox({
                                          text("Hype Energy: ") | bold, text(std::to_string(int(character->mana * 100)) + "/100")
                                      }),
                                      separator(),
                                      hbox({text("Technical Debt: ") | bold, text(std::to_string(character->strength))}),
                                      hbox({text("Patience: ") | bold, text(std::to_string(character->dexterity))}),
                                      hbox({text("Intelligence: ") | bold, text(std::to_string(character->intelligence))}),
                                      hbox({text("Conartistry: ") | bold, text(std::to_string(character->constitution))}),
                                  }) | border;
        // Column 1: Experience bar
        Element experience_bar = window(text("Experience") | bold | center, vbox({
                                            gauge(experience_value) | color(Color::Yellow),
                                            text("Level " + std::to_string(1 + int(experience_value * 10)) +
                                                 " (" + std::to_string(int(experience_value * 100)) + "%)") | center,
                                        }));
        // Column 1: Spells
        Element spells = vbox({
                             text("Spells") | bold | center,
                             separator(),
                             text("Act Dickening") | color(Color::Green),
                             text("Spider Monkey") | color(Color::Green),
                             text("Monkey Bolt") | color(Color::Green),
                             text("To Catasshole") | color(Color::Yellow),
                             text("✓ Investigate the ruins") | color(Color::Green),
                             text("➤ Confront the cultists") | color(Color::Yellow),
                             text("□ Find the lost artifact") | color(Color::GrayDark),
                         }) | border;

        // Column 2: Quests
        Element quests = vbox({
                             text("Quests") | bold | center,
                             separator(),
                             text("Main Quest: The Dragon's Lair") | color(Color::Yellow),
                             gauge(0.7f) | color(Color::Yellow),
                             separator(),
                             text("Side Quest: Missing Merchant") | color(Color::GreenLight),
                             gauge(0.3f) | color(Color::GreenLight),
                             separator(),
                             text("Guild Quest: Thieves' Den") | color(Color::BlueLight),
                             gauge(0.5f) | color(Color::BlueLight),
                             separator(),
                             text("Active Quests: 3/5") | bold,
                         }) | border;
        //Column 2: Inventory
        Element inventory = vbox({
                                text("Inventory") | bold | center,
                                separator(),
                                text("Sword of Flames") | color(Color::Red),
                                text("Shield of Ice") | color(Color::Blue),
                                text("Potion of Healing") | color(Color::Green),
                                text("Bow of Wind") | color(Color::Cyan),
                                text("Helm of Wisdom") | color(Color::Yellow),
                                text("Gauntlets of Strength") | color(Color::RosyBrown),
                                text("Cloak of Shadows") | color(Color::GrayDark),
                                separator(),
                                text("Items: 7/20") | bold,
                            }) | border;

        // Column 3: Plot Development
        Element plot_dev = vbox({
                               text("Plot Development") | bold | center,
                               separator(),
                               text("Chapter 1: The Beginning") | color(Color::Green),
                               text("✓ Meet the village elder") | color(Color::Green),
                               text("✓ Find the ancient map") | color(Color::Green),
                               separator(),
                               text("Chapter 2: Dark Tidings") | color(Color::Yellow),
                               text("✓ Investigate the ruins") | color(Color::Green),
                               text("➤ Confront the cultists") | color(Color::Yellow),
                               text("□ Find the lost artifact") | color(Color::GrayDark),
                               separator(),
                               text("Story Progress: Act 2 of 5") | bold | flex,
                           }) | border;

        // Column 3: Equipment
        Element equipment = vbox({
                                text("Equipment") | bold | center,
                                separator(),
                                text("Act Dickening") | color(Color::Green),
                                text("Spider Monkey") | color(Color::Green),
                                text("Monkey Bolt") | color(Color::Green),
                                text("To Catasshole") | color(Color::Yellow),
                                text("✓ Investigate the ruins") | color(Color::Green),
                                text("➤ Confront the cultists") | color(Color::Yellow),
                                text("□ Find the lost artifact") | color(Color::GrayDark),
                            }) | border | focus | vscroll_indicator;

        // Bottom progress bar
        Element progress_bar = window(
            text("Progress") | bold | center,
            vbox({
                gauge(progress_value) | color(Color::Magenta),
                text(std::to_string(int(progress_value * 100)) + "%") | center,
            }));

        return window(text("Fusion Quest") | bold | center,
                      vbox({
                          // Row 1
                          hbox({
                              vbox({
                                  character_stats,
                                  experience_bar,
                                  spells,
                              }) | flex,
                              // Experience bar with same width as column 1
                              vbox({
                                  quests | flex,
                                  inventory | flex,
                              }) | flex,
                              vbox({
                                  plot_dev | yflex | xflex,
                                  equipment | xflex
                              }) | flex,
                          }),
                          // Progress bar at the bottom
                          progress_bar,
                          separator(),
                          hbox({
                              back_button->Render(),
                              save_button->Render()
                          }) | center,
                      }));
    });

    return renderer;
}

// Helper functions to get race and class names
std::string GetRaceName(int race_index) {
    const std::vector<std::string> RACES = {"Human", "Elf", "Dwarf", "Orc"};
    if (race_index >= 0 && race_index < RACES.size()) {
        return RACES[race_index];
    }
    return "Unknown";
}

std::string GetClassName(int class_index) {
    const std::vector<std::string> CLASSES = {"Warrior", "Mage", "Rogue", "Priest"};
    if (class_index >= 0 && class_index < CLASSES.size()) {
        return CLASSES[class_index];
    }
    return "Unknown";
}
