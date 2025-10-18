#include "core/engine_core.h"
#include "core/components/character_component.h"
#include "core/components/equipment_component.h"
#include "core/components/inventory_component.h"
#include "core/components/quest_component.h"
#include "core/components/stats_component.h"
#include "core/systems/progression_system.h"
#include "core/systems/quest_system.h"
#include "core/systems/save_system.h"
#include "core/systems/ui_system.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "core/systems/GameEngineBridge.h"
#include "ui/ui.h"

using namespace ftxui;
using namespace FusionQuest;

using namespace ftxui;

int main() {
    // Initialize random seed for stat rolling
    std::srand(std::time(nullptr));

    // Initialize the game engine at application startup
    FusionQuest::InitializeGameEngine();

    // Create character and page state
    Character character;
    int current_page = MAIN_MENU;

    // Run screen
    auto screen = ScreenInteractive::Fullscreen();
    auto exit_closure = screen.ExitLoopClosure();

    // Create UI
    auto ui = CreateUI(&current_page, &character, exit_closure);

    // Track time for engine updates
    auto last_update = std::chrono::steady_clock::now();

    // Start a refresh thread that also updates the engine
    std::thread refresh_thread([&screen, &last_update, &character, &current_page] {
        while (true) {
            // Calculate delta time
            auto current_time = std::chrono::steady_clock::now();
            float deltaTime = std::chrono::duration<float>(current_time - last_update).count();
            last_update = current_time;

            // Update the game engine
            if (FusionQuest::GetGameEngine()) {
                FusionQuest::GetGameEngine()->update(deltaTime);

                // If we're on the stats page and player exists, sync the character data
                if (current_page == STATS_PAGE && FusionQuest::GetGameEngine()->getPlayer()) {
                    FusionQuest::GetGameEngine()->updateCharacterFromPlayer(character);
                }
            }

            // Sleep and trigger re-render
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            screen.PostEvent(ftxui::Event::Custom);
        }
    });

    // Detach the refresh thread to avoid blocking the main thread
    refresh_thread.detach();

    // Run the FTXUI loop
    screen.Loop(ui);

    // Cleanup: shutdown the engine when the application exits
    FusionQuest::ShutdownGameEngine();

    return 0;
}