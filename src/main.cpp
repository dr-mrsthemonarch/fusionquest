#include "../include/ui.h"
#include "../include/GameEngine.h"
#include "../include/GameClock.h"
#include "../include/ConfigStore.h"
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

    {
        std::string cfgError;
        if (!validateConfig(cfgError)) {
            std::cerr << "FATAL: " << cfgError << "\nReinstall or rebuild the game.\n";
            return 1;
        }
    }

    bool debugInstakill = false;
    double debugClock = 0.0; // --debug-clock <s>: whole game lasts s real seconds instead of 20 years
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--debug-instakill") debugInstakill = true;
        else if (a == "--debug-clock") {
            try { debugClock = (i + 1 < argc) ? std::stod(argv[++i]) : -1.0; } catch (...) { debugClock = -1.0; }
            if (debugClock <= 0.0) { std::cerr << "usage: --debug-clock <seconds>  (seconds > 0)\n"; return 1; }
        }
        // strip these branches entirely before shipping a release build
    }
    const double kRealTotal = 20.0 * 365 * 24 * 3600;
    const double timeScale = debugClock > 0.0 ? kRealTotal / debugClock : 1.0;

    {
        GameEngine engine(debugInstakill);
        engine.setTimeScale(timeScale);
        engine.loadConfig("config/");

        int current_page = MAIN_MENU;
        auto countdown_timer = ftxui::Make<CountdownComponent>();
        if (debugClock > 0.0) countdown_timer->SetTotalSeconds(debugClock);

        auto screen = ftxui::ScreenInteractive::Fullscreen();
        auto exit_closure = screen.ExitLoopClosure();
        auto ui = CreateUI(&current_page, &engine, countdown_timer, exit_closure);

        // master tick: countdown_timer is the single clock. tick runs ON the ui thread via
        // screen.Post so engine state is never mutated while a render reads it.
        // new game / load bumps engine init generation -> clock restarts at 0.
        auto seen_generation = std::make_shared<int>(0);
        std::thread refresh_thread([&screen, &engine, &current_page, countdown_timer, seen_generation, timeScale] {
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                screen.Post([&engine, &current_page, countdown_timer, seen_generation, timeScale] {
                    if (!engine.isInitialized()) return;
                    if (engine.getInitGeneration() != *seen_generation) {
                        *seen_generation = engine.getInitGeneration();
                        countdown_timer->ResetClock();
                    }
                    engine.tick(countdown_timer->GetElapsedSeconds() * timeScale); // game seconds
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