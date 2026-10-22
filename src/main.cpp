#include "../include/ui.h"
#include "../include/GameEngine.h"
#include "../include/GameClock.h"
#include <ftxui/component/screen_interactive.hpp>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <string>

int main(int argc, char* argv[]) {
    std::srand(std::time(nullptr));

    bool debugInstakill = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--debug-instakill") debugInstakill = true;
        // strip this branch entirely before shipping a release build
    }

    {
        GameEngine engine(debugInstakill);
        engine.loadConfig("config/");

        int current_page = MAIN_MENU;
        auto countdown_timer = ftxui::Make<CountdownComponent>();

        auto screen = ftxui::ScreenInteractive::Fullscreen();
        auto exit_closure = screen.ExitLoopClosure();
        auto ui = CreateUI(&current_page, &engine, countdown_timer, exit_closure);

        // master tick: countdown_timer is the single clock. tick runs ON the ui thread via
        // screen.Post so engine state is never mutated while a render reads it.
        // new game / load bumps engine init generation -> clock restarts at 0.
        auto seen_generation = std::make_shared<int>(0);
        std::thread refresh_thread([&screen, &engine, &current_page, countdown_timer, seen_generation] {
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                screen.Post([&engine, &current_page, countdown_timer, seen_generation] {
                    if (!engine.isInitialized()) return;
                    if (engine.getInitGeneration() != *seen_generation) {
                        *seen_generation = engine.getInitGeneration();
                        countdown_timer->ResetClock();
                    }
                    engine.tick(countdown_timer->GetElapsedSeconds());
                    if (engine.consumeFinalWarning()) current_page = FINAL_WARNING_PAGE;
                });
                screen.PostEvent(ftxui::Event::Custom);
            }
        });
        refresh_thread.detach();

        screen.Loop(ui);
        std::exit(0);  // detached thread still alive; skip destructors racing with it
    }

    return 0;
}