#include "ui.h"
#include "character.h"
#include "progress_bar.h"
#include "globals.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <chrono>

using namespace ftxui;

class CountdownComponent : public ComponentBase {
public:
    CountdownComponent() {
        start_time_ = std::chrono::system_clock::now();
        target_time_ = start_time_ + std::chrono::hours(24*365*20); // 20 years
    }

    [[nodiscard]] std::string GetCountdownString() const {
        auto now = std::chrono::system_clock::now();
        auto remaining = std::chrono::duration_cast<std::chrono::seconds>(target_time_ - now);

        if (remaining.count() <= 0) {
            return "Fusion Quest - LAUNCH TIME!";
        }

        auto seconds = remaining.count();
        auto minutes = seconds / 60; seconds %= 60;
        auto hours = minutes / 60; minutes %= 60;
        auto days = hours / 24; hours %= 24;
        auto months = days / 30; days %= 30; // Approximation
        auto years = months / 12; months %= 12;

        return "Fusion Quest - Power Plant Comissioning: " +
               std::to_string(years) + "y " +
               std::to_string(months) + "m " +
               std::to_string(days) + "d " +
               std::to_string(hours) + "h " +
               std::to_string(minutes) + "m " +
               std::to_string(seconds) + "s";
    }

    Element Render() {
        // Force an animation frame update every second
        if (animation_frame_++ % 60 == 0) {  // Assuming ~60fps, update every second
            // The screen will refresh automatically
        }
        return text(GetCountdownString()) | bold | center;
    }

private:
    std::chrono::system_clock::time_point start_time_;
    std::chrono::system_clock::time_point target_time_;
    int animation_frame_ = 0;
};




int selected_equipment_index = 0;
Component CreateStatsPage(int *selected_page, Character *character,Closure exit_closure) {
    // Create progress bars for experience and progress
    auto experience_bar = std::make_shared<ProgressBar>(0.1f, 0.01f, true);
    auto progress_bar = std::make_shared<ProgressBar>(0.02f, 0.005f, false);

    // Create countdown timer
    auto countdown_timer = Make<CountdownComponent>();

    // Start the progress bars
    experience_bar->Start();
    progress_bar->Start();

    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; })| size(HEIGHT,EQUAL,3);
    auto save_button = Button("Save", [selected_page] { *selected_page = MAIN_MENU; })| size(HEIGHT,EQUAL,3);
    auto quit_button = Button("Quit", std::move(exit_closure))| size(HEIGHT,EQUAL,3);

    // Create a container for equipment
    auto equipment_items = Menu(&equipment_list, &selected_equipment_index);

    // Create a container that includes all components
    auto container = Container::Vertical({
        back_button,
        save_button,
        quit_button,
        equipment_items
    });

    auto renderer = Renderer(container, [back_button, save_button,quit_button, character,
                             experience_bar, progress_bar, equipment_items, countdown_timer] {
        // Column 1: Character Stats

        Element character_stats = vbox({
            text("Character") | bold | center,
            separator(),
            hbox({
                text("Name: ") | bold,
                text(character->name.empty() ? "Unknown" : character->name)
            }),
            hbox({text("Employee nr: ") | bold, text("57")}),
            hbox({text("Position: ") | bold, text(RACES[character->race_index])}),
            hbox({text("Startup Name: ") | bold, text(FUSION_STARTUPS[character->fusion_index])}),
            separator(),
            hbox({
                text("Runway: ") | bold,
                text(std::to_string(int(character->health * 100)) + "/100")
            }),
            hbox({
                text("Hype Energy: ") | bold, text(std::to_string(int(character->mana * 100)) + "/100")
            }),
            separator(),
            hbox({text("Hustle: ") | bold, text(std::to_string(character->hustle))}),
            hbox({text("Agile-ity: ") | bold, text(std::to_string(character->agility))}),
            hbox({text("NDAs: ") | bold, text(std::to_string(character->ndas))}),
            hbox({text("Patents: ") | bold, text(std::to_string(character->patents))}),
            hbox({text("Luck: ") | bold, text(std::to_string(character->luck))}),
            hbox({text("Technical Debt: ") | bold, text(std::to_string(character->techdebt))}),
            hbox({text("Bus Factor: ") | bold, text(std::to_string(character->busfactor))}),
        }) | border;

        // Column 1: Experience bar
        Element experience_gauge = window(text("Funding") | bold | center, vbox({
            gauge(experience_bar->GetProgress()) | color(Color::Yellow),
            text("Series " + std::to_string(1 + int(experience_bar->GetProgress() * 10)) +
            " (" + std::to_string(int(experience_bar->GetProgress() * 100)) + "%)") | center,
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

        // Column 2: Inventory
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
            text("Stocks: 1000") | bold
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

        Element equipment = vbox({
            text("Equipment") | bold | center,
            separator(),
            equipment_items->Render() | vscroll_indicator | frame
        }) | border;

        // Bottom progress bar
        Element progress_gauge = window(
            text("Progress") | bold | center,
            vbox({
                text("Axing the Monkey") | center,
                gauge(progress_bar->GetProgress()) | color(Color::Blue),
                text(std::to_string(int(progress_bar->GetProgress() * 100)) + "%") | center,
            }));

        return window(countdown_timer->Render(),
            vbox({
                // Row 1
                hbox({
                    vbox({
                        character_stats,
                        experience_gauge,
                        spells,
                    }) | flex,
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
                progress_gauge,
                separator(),
                hbox({
                    back_button->Render(),
                    save_button->Render(),
                    quit_button->Render()
                }) | center,
            }));
    });

    return renderer;
}