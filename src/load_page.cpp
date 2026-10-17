// load_page.cpp
#include "../include/ui.h"
#include "../include/GameEngine.h"
#include "../include/GameClock.h"
#include "../include/SaveGame.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <filesystem>
#include <vector>

using namespace ftxui;

Component CreateLoadPage(int* selected_page, GameEngine* engine,
                          std::shared_ptr<CountdownComponent> countdown_timer) {
    auto save_files = std::make_shared<std::vector<std::string>>();
    auto selected_index = std::make_shared<int>(0);

    try {
        if (!std::filesystem::exists("saves")) std::filesystem::create_directory("saves");
        for (const auto& entry : std::filesystem::directory_iterator("saves"))
            if (entry.path().extension() == ".sav") save_files->push_back(entry.path().filename().string());
    } catch (const std::filesystem::filesystem_error&) {
        // best effort, ignore
    }

    auto save_selector = Menu(save_files.get(), selected_index.get());
    auto save_container = Container::Vertical({save_selector});

    auto load_button = Button("Load Character", [=] {
        if (save_files->empty()) return;
        std::string filename = "saves/" + (*save_files)[*selected_index];

        // fresh baseline first (starter gear, fresh review calendar), THEN overwrite w/ save.
        // matches spec: clock always restarts at 0, rest of progress restored from file.
        engine->initializeCharacter("", "1", "", ""); // name/position overwritten by load below
        if (SaveGame::load(*engine, filename)) {
            countdown_timer->ResetClock();
            *selected_page = STATS_PAGE;
        }
    });

    auto back_button = Button("Back", [selected_page] { *selected_page = MAIN_MENU; }) | size(HEIGHT, EQUAL, 4);

    auto container = Container::Vertical({save_container, load_button, back_button});

    auto renderer = Renderer(container, [=] {
        Element list_element = save_files->empty()
            ? (Element)(text("No save files found") | center)
            : (Element)(save_selector->Render() | vscroll_indicator | frame);

        return window(text("Load Character") | bold | center, vbox({
            vbox({text("Saved Characters") | bold | center, separator(), list_element}) | flex | border,
            separator(),
            load_button->Render(),
            separator(),
            back_button->Render(),
        }));
    });

    return renderer;
}
