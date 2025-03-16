#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/loop.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>

using namespace ftxui;

// File to store the timer value
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

int main() {
    // Create a screen interactive for rendering
    auto screen = ScreenInteractive::Fullscreen();

    // Load the timer value from the file
    int seconds = load_timer();
    bool running = true;

    // Define components for each window
    auto stats_window = Renderer([] {
        return window(text("Character Stats"),
            vbox({
                text("Level: 10"),
                text("Runway: 100/100"),
                text("HE: 50/50"),
                text("Technical Debt: 15"),
                text("Patience: 12"),
                text("Intelligence: 18"),
                text("Series Survival: 12"),
                text("Charisma: 999")
            })
        );
    });

    auto inventory_window = Renderer([] {
        return window(text("Inventory"),
            vbox({
                text("Sword of Fire"),
                text("Shield of Light"),
                text("Potion x5"),
                text("Funding: 1000")
            })
        );
    });

    auto quest_window = Renderer([] {
        return window(text("Quests"),
            vbox({
                text("1. Defeat the Goblin King"),
                text("2. Retrieve the Lost Artifact"),
                text("3. Rescue the Princess")
            })
        );
    });

    auto log_window = Renderer([] {
        return window(text("Game Log"),
            vbox({
                text("You defeated a Goblin!"),
                text("You found a Potion."),
                text("You leveled up!"),
                text("You entered the Dark Forest.")
            })
        );
    });

    auto map_window = Renderer([] {
        return window(text("World Map"),
            vbox({
                text("Forest"),
                text("Mountains"),
                text("Village"),
                text("Dungeon")
            })
        );
    });

    auto progress_window = Renderer([] {
        return window(text("Progress"),
            vbox({
                text("Quest Progress: 50%"),
                text("Exploration: 30%"),
                text("Achievements: 5/10")
            })
        );
    });

    // Progress bar state
    float progress = 0.5f; // 50% progress

    // Progress bar component in its own window
    auto progress_bar_window = Renderer([&] {
        return window(text("Progress Bar"),
            vbox({
                gauge(progress)
            })
        );
    });

    // Combine all windows into a single layout
    auto main_layout = Container::Vertical({
        Container::Horizontal({
            stats_window,
            inventory_window,
            quest_window,
            log_window,
            map_window,
            progress_window
        }),
        progress_bar_window
    });

    // // Function to format the remaining time as HH:MM:SS, counting down from 20 years
    // auto format_time = [](int seconds) {
    //     const int twenty_years_in_seconds = 20 * 365 * 24 * 3600; // Approximation (ignoring leap years)
    //     int remaining_seconds = twenty_years_in_seconds - seconds;
    //     if (remaining_seconds < 0) {
    //         remaining_seconds = 0;
    //     }
    //     int hours = remaining_seconds / 3600;
    //     int minutes = (remaining_seconds % 3600) / 60;
    //     int secs = remaining_seconds % 60;
    //     std::ostringstream oss;
    //     oss << std::setw(2) << std::setfill('0') << hours << ":"
    //         << std::setw(2) << std::setfill('0') << minutes << ":"
    //         << std::setw(2) << std::setfill('0') << secs;
    //     return oss.str();
    // };

    // Wrap all windows inside a larger window titled "Fusion Quest" with a timer
    auto fusion_quest_window = Renderer(main_layout, [&] {
        std::string title = "Fusion Quest - Time Remaining: " + format_time(seconds);
        return window(text(title),
            vbox({
                hbox({
                    stats_window->Render(),
                    inventory_window->Render(),
                    quest_window->Render(),
                    log_window->Render(),
                    map_window->Render(),
                    progress_window->Render()
                }),
                progress_bar_window->Render()
            })
        );
    });

    // Render the layout
    auto renderer = Renderer(fusion_quest_window, [&] {
        return fusion_quest_window->Render();
    });

    // Timer thread to update the seconds counter
    std::thread timer_thread([&] {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            seconds++;
            screen.PostEvent(Event::Custom); // Trigger a re-render
        }
    });

    // Run the loop
    Loop loop(&screen, renderer);
    while (!loop.HasQuitted()) {
        loop.RunOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Stop the timer thread
    running = false;
    if (timer_thread.joinable()) {
        timer_thread.join();
    }

    // Save the timer value to the file before exiting
    save_timer(seconds);

    return 0;
}