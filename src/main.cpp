#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <filesystem> // For file existence check
#include "banner.h"
#include "second_page.h" // Include the second page header
#include "functions.h"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/color.hpp"

using namespace ftxui;

Component Wrap(std::string name, Component component) {
    return Renderer(component, [name, component] {
        return hbox({
            text(name) | size(WIDTH, EQUAL, 15),
            separator() | color(Color::Default),
            component->Render() | xflex,
        });
    });
}

auto button_style = ButtonOption::Animated();

int main() {
    // Initialize the log file
    InitializeLogFile();

    Log("Application started.");

    // State of the application:
    auto screen = ScreenInteractive::Fullscreen();
    Log("Screen initialized.");

    // There are two layers. One at depth = 0 and the modal window at depth = 1;
    int depth = 0;
    // State to manage which page is active
    int active_page = 0; // 0 = first page, 1 = second page
    Log("Initial state: active_page = " + std::to_string(active_page) + ", depth = " + std::to_string(depth));

    // Load the banner from the banner.h file
    std::vector<std::string> lines = splitLines(fileContent);
    Log("Banner loaded.");

    // Main screen renderer
    auto mainScreen = Renderer([&] {
        Elements children = {};
        for (size_t i = std::max(0, (int) lines.size() - 55); i < lines.size(); ++i) {
            children.push_back(text(lines[i]));
        }
        return flexbox(children) | size(HEIGHT, GREATER_THAN, 55) | center | color(Color::Yellow) | borderDouble | flex;
    });

    // At depth=1, the modal window with four buttons
    auto depth_1_container = Container::Vertical({
        Button("Start a New Game", [&] {
            Log("Starting a new game!");
            depth = 0; // Close the modal after starting a new game
            active_page = 1; // Move to the second page
            Log("State updated: active_page = " + std::to_string(active_page) + ", depth = " + std::to_string(depth));
        }, button_style),
        Button("Continue a Game", [&] {
            Log("Continuing a game!");
            depth = 0; // Close the modal after continuing a game
        }, button_style),
        Button("Delete Game", [&] {
            Log("Deleting a game!");
            depth = 0; // Close the modal after deleting a game
        }, button_style),
        Button("Quit", [&] {
            Log("Quitting application...");
            screen.Exit(); // Exit the application
        }, button_style),
    });

    auto depth_1_renderer = Renderer(depth_1_container, [&] {
        return vbox({
            text("Main Menu"),
            separator(),
            depth_1_container->Render(),
        }) | border | center;
    });

    // Create the second page using the SecondPage function from second_page.h
    auto second_page = SecondPage(
        [&] {
            active_page = 0; // Switch back to the main page
            Log("Back button clicked! active_page = " + std::to_string(active_page));
        },
        [&] {
            depth = 1; // Reopen the modal
            Log("Modal reopened! depth = " + std::to_string(depth));
        }
    );

    // Main container to hold the main screen and modal
    auto main_container = Container::Tab({
        mainScreen,
        second_page,
    }, &active_page);

    // Main renderer for the application
    auto main_renderer = Renderer(main_container, [&] {
        Element document;

        // Render the active page
        if (active_page == 0) {
            document = mainScreen->Render();
        } else {
            document = second_page->Render();
        }

        // Render the modal on top if depth == 1
        if (depth == 1) {
            document = dbox({
                document,
                depth_1_renderer->Render() | clear_under | center,
            });
        }

        return document;
    });

    // Add a global event handler to capture any key press
    auto global_event_handler = CatchEvent(main_renderer, [&](Event event) {
        if (event.is_character() || event.is_mouse()) {
            depth = 1; // Set depth to 1 (open the modal) on any key press
            Log("Key pressed! depth = " + std::to_string(depth));
            return true; // Event handled
        }
        return false; // Event not handled
    });

    Log("Starting main loop...");
    screen.Loop(global_event_handler); // Use the global event handler
    Log("Main loop exited.");

    Log("Application terminated.");

    return 0;
}