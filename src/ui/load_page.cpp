// Add this to your appropriate file (perhaps main_menu.cpp)
#include "ui.h"
#include "../classes/functions.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <filesystem>
#include <vector>



using namespace ftxui;

Component CreateLoadPage(int* selected_page, Character* character) {
    // Create a persistent container for save files
    auto save_files = std::make_shared<std::vector<std::string>>();
    auto selected_index = std::make_shared<int>(0);

    // List all save files in the saves directory
    try {
        // Create saves directory if it doesn't exist
        if (!std::filesystem::exists("saves")) {
            std::filesystem::create_directory("saves");
        }

        for (const auto& entry : std::filesystem::directory_iterator("saves")) {
            if (entry.path().extension() == ".sav") {
                save_files->push_back(entry.path().filename().string());
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Handle error gracefully
        system("mkdir -p saves");
    }

    auto save_selector = Menu(save_files.get(), selected_index.get());

    // Create a scrollable container for the save file list
    auto save_container = Container::Vertical({save_selector});

    auto load_button = Button("Load Character", [save_files, selected_index, selected_page, character] {
        if (!save_files->empty()) {
            std::string filename = "saves/" + (*save_files)[*selected_index];
            *character = LoadCharacterFromFile(filename);
            *selected_page = STATS_PAGE;  // Go to stats page after loading
        }
    });

    auto back_button = Button("Back", [selected_page] {
        *selected_page = MAIN_MENU;
    }) | size(HEIGHT, EQUAL, 4);

    auto container = Container::Vertical({
        save_container,
        load_button,
        back_button,
    });

    auto renderer = Renderer(container, [container, save_selector, load_button, back_button, save_files] {
        Element list_element;

        if (save_files->empty()) {
            list_element = text("No save files found") | center;
        } else {
            list_element = save_selector->Render() | vscroll_indicator | frame;
        }

        return window(text("Load Character") | bold | center, vbox({
            vbox({
                text("Saved Characters") | bold | center,
                separator(),
                list_element,
            }) | flex | border,
            separator(),
            load_button->Render(),
            separator(),
            back_button->Render(),
        }));
    });

    return renderer;
}