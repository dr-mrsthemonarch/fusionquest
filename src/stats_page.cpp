#include "ui.h"
#include "character.h"
#include "progress_bar.h" // Include the ProgressBar class
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>

using namespace ftxui;
const std::vector<std::string> equipment_list = {
    "Vintage Macbook Pro",
    "Emergency Coffee IV Drip",
    "Noise-Cancelling Headphones",
    "Ergonomic Standing Desk",
    "Bitcoin Mining Rig",
    "Quantum Computing Textbook",
    "Raspberry Pi Cluster",
    "VR Pitch Deck Simulator",
    "Startup Hoodie (Silicon Valley Edition)",
    "Mechanical Keyboard with RGB Backlighting",
    "Whiteboard Marker of Inspiration",
    "Debugging Energy Drink Stash",
    "Backup Charger Power Bank",
    "Laser Pointer for Epic Presentations",
    "Startup Motivational Poster",
    "Blockchain Certification",
    "AI Neural Network Schematics",
    "Emergency Pizza Delivery Card",
    "Noise-Cancelling Earpods",
    "Startup Equity Options Paperweight"
};

const std::string timer_file = "timer_value.txt";

// Function to load the timer value from a file
int load_timer() {
    std::ifstream file(timer_file);
    int seconds = 0;
    if (file.is_open()) {
        file >> seconds;
        file.close();
    }
    return seconds;
}

// Function to save the timer value to a file
void save_timer(int seconds) {
    std::ofstream file(timer_file);
    if (file.is_open()) {
        file << seconds;
        file.close();
    }
}

// Function to calculate the remaining time in years, months, days, hours, minutes, and seconds
std::string format_time(int elapsed_seconds) {
    // Total duration of 20 years in seconds
    const int twenty_years_in_seconds = 20 * 365 * 24 * 3600; // Approximation (ignoring leap years)

    // Calculate remaining time
    int remaining_seconds = twenty_years_in_seconds - elapsed_seconds;

    // Ensure remaining time doesn't go negative
    if (remaining_seconds < 0) {
        remaining_seconds = 0;
    }

    // Convert remaining seconds to years, months, days, hours, minutes, and seconds
    int years = remaining_seconds / (365 * 24 * 3600);
    remaining_seconds %= 365 * 24 * 3600;
    int months = remaining_seconds / (30 * 24 * 3600); // Approximation
    remaining_seconds %= 30 * 24 * 3600;
    int days = remaining_seconds / (24 * 3600);
    remaining_seconds %= 24 * 3600;
    int hours = remaining_seconds / 3600;
    remaining_seconds %= 3600;
    int minutes = remaining_seconds / 60;
    int secs = remaining_seconds % 60;

    // Format the current date
    std::time_t now = std::time(nullptr);
    char current_date[100];
    std::strftime(current_date, sizeof(current_date), "%A, %d %B %Y", std::localtime(&now));

    // Format the remaining time
    std::ostringstream oss;
    oss << current_date << " - Time Left: "
        << years << " years, "
        << months << " months, "
        << days << " days, "
        << hours << " hours, "
        << minutes << " minutes, "
        << secs << " seconds";
    return oss.str();
}
// Load the timer value from the file
int seconds = load_timer();
bool running = true;

int selected_equipment_index = 0;

// Shared state to track if the timer value has been updated
std::atomic<bool> timer_updated(false);

Component CreateStatsPage(int *selected_page, Character *character) {
    // Create progress bars for experience and progress
    auto experience_bar = std::make_shared<ProgressBar>(0.01f, 0.01f, true); // Looping experience bar
    auto progress_bar = std::make_shared<ProgressBar>(0.02f, 0.005f, false); // Non-looping progress bar
    // auto assbar = std::make_shared<ProgressBar>(0.02,0.005,)

    // Start the progress bars
    experience_bar->Start();
    progress_bar->Start();

    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; });
    auto save_button = Button("Save", [selected_page] { *selected_page = MAIN_MENU; });

    // Create a container for equipment
    auto equipment_items = Menu(&equipment_list, &selected_equipment_index);

    // Create a container that includes the equipment menu
    auto container = Container::Vertical({
        back_button,
        save_button,
        equipment_items  // Add equipment menu directly to the container
    });

    auto renderer = Renderer(container, [back_button, save_button, character, experience_bar, progress_bar,equipment_items] {
        std::string title = "Fusion Quest - Time Remaining: " + format_time(seconds);
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
        Element experience_gauge = window(text("Experience") | bold | center, vbox({
            gauge(experience_bar->GetProgress()) | color(Color::Yellow),
            text("Level " + std::to_string(1 + int(experience_bar->GetProgress() * 10)) +
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


        // In your main renderer, use the scrollable equipment renderer
        // Element equipment = equipment_renderer->Render();

        // // Column 3: Equipment
        // Element equipment = vbox({
        //     text("Equipment") | bold | center,
        //     separator(),
        //     text("Act Dickening") | color(Color::Green),
        //     text("Spider Monkey") | color(Color::Green),
        //     text("Monkey Bolt") | color(Color::Green),
        //     text("To Catasshole") | color(Color::Yellow),
        //     text("✓ Investigate the ruins") | color(Color::Green),
        //     text("➤ Confront the cultists") | color(Color::Yellow),
        //     text("□ Find the lost artifact") | color(Color::GrayDark),
        // }) | border | focus | vscroll_indicator;


        // Bottom progress bar
        Element progress_gauge = window(
            text("Progress") | bold | center,
            vbox({
                text("Axing the Monkey") | center,
                gauge(progress_bar->GetProgress()) | color(Color::Magenta),
                text(std::to_string(int(progress_bar->GetProgress() * 100)) + "%") | center,
            }));

        return window(text(title) | bold | center,
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
                    save_button->Render()
                }) | center,
            }));
    });

// Timer thread to update the seconds counter
std::thread timer_thread([&] {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        seconds++;
        timer_updated.store(true);  // Mark the timer as updated
    }
});

// Main loop: Update the UI if the timer has been updated
if (timer_updated.load()) {
    // Re-render the UI to show the updated time
    timer_updated.store(false); // Reset the update flag
}

// Stop the timer thread when done
running = false;
if (timer_thread.joinable()) {
    timer_thread.join();
}

// Save the timer value to the file before exiting
save_timer(seconds);


    return renderer;
}