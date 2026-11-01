#include "../include/GameEngine.h"
#include "../include/GameClock.h"
#include "../include/ui.h"
#include "../include/Tuning.h"
#include "../include/functions.h"
#include <algorithm>
#include <chrono>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// first-nemesis story popup. clock + engine are paused while open. OK, or auto-close after
// nemesis.intro_timeout_sec (so leaving the game running never freezes it forever).
// battle starts nemesis.intro_fight_delay_sec after dismissal.
Component CreateNemesisIntroPage(int* selected_page, GameEngine* engine,
                                 std::shared_ptr<CountdownComponent> countdown_timer) {
    auto opened = std::make_shared<std::chrono::steady_clock::time_point>();
    auto armed = std::make_shared<bool>(false);

    auto dismiss = [=] {
        *armed = false;
        engine->acknowledgeNemesisIntro();
        countdown_timer->Resume();
        *selected_page = STATS_PAGE;
    };
    auto ok_button = Button("OK", dismiss);

    return Renderer(ok_button, [=] {
        const auto now = std::chrono::steady_clock::now();
        if (!*armed) { *armed = true; *opened = now; }
        const double timeout = T("nemesis.intro_timeout_sec");
        const double left = timeout - std::chrono::duration<double>(now - *opened).count();
        if (!engine->nemesisIntroOpen() || left <= 0.0) dismiss();

        Elements lines;
        for (const auto& l : splitLines(engine->getNemesisIntroText()))
            lines.push_back(l.empty() ? text(" ") : paragraph(l));
        return vbox({
                   text("!! A WILD RIVAL APPEARS !!") | bold | center | color(Color::Red),
                   separator(),
                   vbox(std::move(lines)),
                   separator(),
                   text("Continues automatically in " + std::to_string(std::max(0, static_cast<int>(left) + 1)) + "s") | dim | center,
                   ok_button->Render() | center,
               }) |
               border | size(WIDTH, LESS_THAN, 76) | center;
    });
}
