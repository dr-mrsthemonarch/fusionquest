#include "../include/ui.h"
#include "../include/GameEngine.h"
#include "../include/GameClock.h"
#include <ftxui/component/screen_interactive.hpp>
#include <ctime>
#include <cstdlib>
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

    GameEngine engine(debugInstakill);
    if (!engine.loadConfig("config/")) {
        // non-fatal: continue, individual generators fall back to defaults where they can
    }

    int current_page = MAIN_MENU;
    auto countdown_timer = ftxui::Make<CountdownComponent>();

    auto screen = ftxui::ScreenInteractive::Fullscreen();
    auto exit_closure = screen.ExitLoopClosure();

    auto ui = CreateUI(&current_page, &engine, countdown_timer, exit_closure);

    // master tick loop: countdown_timer is the single clock source. every 100ms,
    // feed elapsed seconds into engine (only meaningful once character exists),
    // then post a redraw event.
    std::thread refresh_thread([&screen, &engine, countdown_timer] {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (engine.isInitialized()) {
                engine.tick(countdown_timer->GetElapsedSeconds());
            }
            screen.PostEvent(ftxui::Event::Custom);
        }
    });
    refresh_thread.detach();

    screen.Loop(ui);
    return 0;
}
