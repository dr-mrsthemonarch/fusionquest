#include "../include/GameEngine.h"
#include "../include/ui.h"
#include "../include/globals.h"
#include "../include/NavBar.h"
#include <algorithm>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// reached from the stats page "Battle Log" button. top: player + current enemies.
// bottom: FULL scrollable log = summary lines + engine details (patent effects, buffs,
// level-ups, sales...) that never fit in the stats page progress line.
// scroll: Up/Down, PgUp/PgDn, Home/End, mouse wheel. pos 1.0 = follow newest.
Component CreateBattlePage(int* selected_page, GameEngine* engine, Closure exit_closure) {
    auto scroll_pos = std::make_shared<float>(1.0f);

    auto nav = CreateNavBar(selected_page, engine, std::move(exit_closure), BATTLE_PAGE);
    auto container = Container::Vertical({nav});

    auto renderer = Renderer(container, [=] {
        std::vector<Element> top;

        if (Character* c = engine->getCharacter()) {
            top.push_back(hbox({
                text(c->getName()) | bold | size(WIDTH, EQUAL, 30),
                text("Runway " + std::to_string(c->getRunway()) + "/" + std::to_string(c->getMaxRunway())) |
                    size(WIDTH, EQUAL, 20),
                text("Hype " + std::to_string(c->getHypeEnergy()) + "/" + std::to_string(c->getMaxHypeEnergy())),
            }));
        } else {
            top.push_back(text("No character yet.") | dim);
        }
        top.push_back(separator());

        if (!engine->isInBattle() || !engine->getCurrentEncounter()) {
            top.push_back(text("Waiting for next encounter...") | dim | center);
        } else {
            top.push_back(text("Round: " + std::to_string(engine->getRoundNumber())) | bold | center);
            for (auto* enemy : engine->getCurrentEncounter()->getAllEnemies()) {
                top.push_back(hbox({
                    text(enemy->name) | size(WIDTH, EQUAL, 30),
                    text("HP " + std::to_string(enemy->health) + "/" + std::to_string(enemy->maxHealth)) |
                        size(WIDTH, EQUAL, 16),
                    text("ATK " + std::to_string(enemy->attack)) | size(WIDTH, EQUAL, 10),
                    text("DEF " + std::to_string(enemy->defense)) | size(WIDTH, EQUAL, 10),
                    text(enemy->isAlive() ? "" : " [DEFEATED]") | color(Color::Red),
                }));
            }
        }

        std::vector<Element> lines;
        for (const auto& l : engine->getDetailLog().lines()) lines.push_back(text(l));
        if (lines.empty()) lines.push_back(text("No log entries yet.") | dim);

        Element log_view = vbox(std::move(lines)) | focusPositionRelative(0.0f, *scroll_pos) |
                           vscroll_indicator | yframe | flex;

        return window(text("Battle Log") | bold | center,
                      vbox({
                          vbox(std::move(top)) | border,
                          vbox({text("Log") | bold, separator(), log_view}) | border | flex,
                          text("Up/Down  PgUp/PgDn  Home/End  mouse wheel to scroll") | dim | center,
                          nav->Render(),
                      }));
    });

    return CatchEvent(renderer, [=](Event e) {
        size_t n = std::max<size_t>(2, engine->getDetailLog().lines().size());
        float line = 1.0f / static_cast<float>(n - 1);
        auto move = [&](float d) {
            *scroll_pos = std::clamp(*scroll_pos + d, 0.0f, 1.0f);
            return true;
        };

        if (e == Event::ArrowUp) return move(-line);
        if (e == Event::ArrowDown) return move(line);
        if (e == Event::PageUp) return move(-15 * line);
        if (e == Event::PageDown) return move(15 * line);
        if (e == Event::Home) { *scroll_pos = 0.0f; return true; }
        if (e == Event::End) { *scroll_pos = 1.0f; return true; }
        if (e.is_mouse()) {
            if (e.mouse().button == Mouse::WheelUp) return move(-3 * line);
            if (e.mouse().button == Mouse::WheelDown) return move(3 * line);
        }
        return false;
    });
}
