// stats_page.cpp
#include "ui.h"
#include "character.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

Component CreateStatsPage(int* selected_page, Character* character) {
    // These would typically come from the character class
    float experience_value = 0.45f; // Example value
    float progress_value = 0.65f;   // Example value

    // Stats for the columns - these would come from your character in a real app
    float health_value = 0.8f;
    float mana_value = 0.6f;
    float stamina_value = 0.7f;
    float attack_value = 0.5f;
    float defense_value = 0.9f;
    float magic_value = 0.4f;

    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; });

    auto container = Container::Vertical({
        back_button,
    });

    auto renderer = Renderer(container, [container, back_button, experience_value, progress_value,
                                        health_value, mana_value, stamina_value,
                                        attack_value, defense_value, magic_value] {
        // Row 1 columns
        Element health_column = vbox({
            text("Health") | bold | center,
            separator(),
            text(std::to_string(int(health_value * 100)) + "/100") | center,
            gaugeUp(health_value) | color(Color::Red),
        }) | border;

        Element mana_column = vbox({
            text("Mana") | bold | center,
            separator(),
            text(std::to_string(int(mana_value * 100)) + "/100") | center,
            gaugeUp(mana_value) | color(Color::Blue),
        }) | border;

        Element stamina_column = vbox({
            text("Stamina") | bold | center,
            separator(),
            text(std::to_string(int(stamina_value * 100)) + "/100") | center,
            gaugeUp(stamina_value) | color(Color::Green),
        }) | border;

        // Experience bar between rows
        Element experience_bar = vbox({
            text("Experience") | bold | center,
            gauge(experience_value) | color(Color::Yellow),
            text("Level " + std::to_string(1 + int(experience_value * 10))) | center,
        }) | border;

        // Row 2 columns
        Element attack_column = vbox({
            text("Attack") | bold | center,
            separator(),
            text(std::to_string(int(attack_value * 100))) | center,
            gaugeUp(attack_value) | color(Color::RedLight),
        }) | border;

        Element defense_column = vbox({
            text("Defense") | bold | center,
            separator(),
            text(std::to_string(int(defense_value * 100))) | center,
            gaugeUp(defense_value) | color(Color::BlueLight),
        }) | border;

        Element magic_column = vbox({
            text("Magic") | bold | center,
            separator(),
            text(std::to_string(int(magic_value * 100))) | center,
            gaugeUp(magic_value) | color(Color::GreenLight),
        }) | border;

        // Bottom progress bar
        Element progress_bar = vbox({
            text("Progress") | bold | center,
            gauge(progress_value) | color(Color::Magenta),
            text(std::to_string(int(progress_value * 100)) + "%") | center,
        }) | border;

        return window(text("Fusion Quest") | bold | center, vbox({
            // Row 1
            hbox({
                health_column | flex,
                mana_column | flex,
                stamina_column | flex
            }),
            // Experience between rows
            experience_bar,
            // Row 2
            hbox({
                attack_column | flex,
                defense_column | flex,
                magic_column | flex
            }),
            // Progress bar at the bottom
            progress_bar,
            separator(),
            back_button->Render(),  // Back button
        }));
    });

    return renderer;
}