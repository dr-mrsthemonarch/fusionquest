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
    InitializeLogFile();
    Log("Game Started");
    // State of the application:
    auto screen = ScreenInteractive::Fullscreen();
    // There are two layers. One at depth = 0 and the modal window at depth = 1;
    int depth = 0;

    // State to manage which page is active
    int active_page = 0; // 0 = first page, 1 = second page
    auto startaction = [&] {
        active_page = 1; // Switch to the second page
        depth = 0; // Close the modal
        Log("Start Button Pressed");
    };
    auto quitaction = [&] {
        Log("Quit Button Pressed");
        screen.ExitLoopClosure()();
    };

    // Load the banner from the banner.h file
    std::vector<std::string> lines = splitLines(fileContent);

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
        Button("Start a New Game", startaction, button_style),
        Button("Continue a Game", [&] {
            std::cout << "Continuing a game!" << std::endl;
            depth = 0; // Close the modal after continuing a game
        }, button_style),
        Button("Delete Game", [&] {
            std::cout << "Deleting a game!" << std::endl;
            depth = 0; // Close the modal after deleting a game
        }, button_style),
        Button("Quit", [&] {
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

    // Create the second page using the function from second_page.h
    auto second_page_renderer = CreateSecondPage([&] { active_page = 0; });

    // Main container to hold the main screen and modal
    auto first_container = Container::Horizontal({
        mainScreen | size(HEIGHT, EQUAL, 60) | size(WIDTH, EQUAL, 150),
        depth_1_renderer,
    }, &depth) | size(HEIGHT, EQUAL, 60) | size(WIDTH, EQUAL, 150);

    // Main container to switch between pages
    auto main_container = Container::Tab({
        first_container,
        second_page_renderer,
    }, &active_page); // Pass the active_page index

    // Main renderer for the application
    auto main_renderer = Renderer(main_container, [&] {
        if (active_page == 0) {
            // Render the first page (main screen and modal)
            Element document = mainScreen->Render();
            if (depth == 1) {
                document = dbox({
                    document,
                    depth_1_renderer->Render() | clear_under | center,
                });
            }
            return document;
        } else {
            // Render the second page
            return second_page_renderer->Render();
        }
    });

    // Run the application
    screen.Loop(main_renderer);
    return 0;
}