#include "ui.h"
#include "character.h"
#include <ftxui/component/screen_interactive.hpp>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>

int main() {
    // Initialize random seed for stat rolling
    std::srand(std::time(nullptr));

    // Create character and page state
    Character character;
    int current_page = MAIN_MENU;

    // Run screen
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    auto exit_closure = screen.ExitLoopClosure();

    // Create UI
    auto ui = CreateUI(&current_page, &character,exit_closure);
    // Start a refresh thread to post events periodically
    std::thread refresh_thread([&screen] {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Refresh every 10ms
            screen.PostEvent(ftxui::Event::Custom); // Trigger a re-render
        }
    });

    // Detach the refresh thread to avoid blocking the main thread
    refresh_thread.detach();

    // Run the FTXUI loop
    screen.Loop(ui);

    return 0;
}