#include "../include/NavBar.h"
#include "../include/GameEngine.h"
#include "../include/SaveGame.h"
#include "../include/ui.h"
#include "../include/Tuning.h"
#include <chrono>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

Component CreateNavBar(int* selected_page, GameEngine* engine, Closure exit_closure, int current_page) {
    auto msg = std::make_shared<std::string>();
    auto when = std::make_shared<std::chrono::steady_clock::time_point>();

    auto mk = [](const std::string& label, std::function<void()> fn) {
        return Button(label, std::move(fn)) | size(HEIGHT, EQUAL, 3);
    };

    Components buttons;
    buttons.push_back(mk("Back to Main", [selected_page] { *selected_page = MAIN_MENU; }));
    buttons.push_back(mk("Save", [engine, msg, when] {
        std::string path;
        bool ok = engine->isInitialized() && SaveGame::saveSlot(*engine, &path);
        *msg = ok ? "Game saved: " + path : "Save failed (no character yet?)";
        *when = std::chrono::steady_clock::now();
    }));
    if (current_page != STATS_PAGE) buttons.push_back(mk("Stats", [selected_page] { *selected_page = STATS_PAGE; }));
    if (current_page != BATTLE_PAGE) buttons.push_back(mk("Battle Log", [selected_page] { *selected_page = BATTLE_PAGE; }));
    if (current_page != SPECIAL_DROPS_PAGE)
        buttons.push_back(mk("Special Items", [selected_page] { *selected_page = SPECIAL_DROPS_PAGE; }));
    if (Tuning::I().enabled() && current_page != TUNER_PAGE)
        buttons.push_back(mk("Tuner", [selected_page] { *selected_page = TUNER_PAGE; }));
    buttons.push_back(Button("Quit", std::move(exit_closure)) | size(HEIGHT, EQUAL, 3));

    auto container = Container::Horizontal(buttons);
    return Renderer(container, [=] {
        Elements row;
        for (const auto& b : buttons) row.push_back(b->Render());
        Elements col;
        col.push_back(hbox(row) | center);
        if (!msg->empty() && std::chrono::steady_clock::now() - *when < std::chrono::seconds(4))
            col.push_back(text(*msg) | center | color(Color::Green));
        return vbox(col);
    });
}
