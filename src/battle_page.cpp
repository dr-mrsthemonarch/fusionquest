#include "../include/GameEngine.h"
#include "../include/ui.h"
#include "../include/globals.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

Component CreateBattlePage(int* selected_page, GameEngine* engine) {
    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; });
    auto container = Container::Vertical({back_button});

    auto renderer = Renderer(container, [=] {
        Character* c = engine->getCharacter();
        std::vector<Element> rows;

        if (!engine->isInBattle() || !engine->getCurrentEncounter()) {
            rows.push_back(text("Waiting for next encounter...") | dim | center);
        } else {
            rows.push_back(text("Round: " + std::to_string(engine->getRoundNumber())) | bold | center);
            rows.push_back(separator());
            for (auto* enemy : engine->getCurrentEncounter()->getAllEnemies()) {
                std::string status = enemy->isAlive() ? "" : " [DEFEATED]";
                rows.push_back(hbox({
                    text(enemy->name) | size(WIDTH, EQUAL, 30),
                    text("HP " + std::to_string(enemy->health) + "/" + std::to_string(enemy->maxHealth)) | size(WIDTH, EQUAL, 16),
                    text("ATK " + std::to_string(enemy->attack)) | size(WIDTH, EQUAL, 10),
                    text("DEF " + std::to_string(enemy->defense)) | size(WIDTH, EQUAL, 10),
                    text(status) | color(Color::Red),
                }));
            }
        }

        Element player_row = hbox({
            text(c->getName()) | bold | size(WIDTH, EQUAL, 30),
            text("Runway " + std::to_string(c->getRunway()) + "/" + std::to_string(c->getMaxRunway())) | size(WIDTH, EQUAL, 20),
            text("Hype " + std::to_string(c->getHypeEnergy()) + "/" + std::to_string(c->getMaxHypeEnergy())),
        });

        std::vector<Element> log_lines;
        const auto& log = engine->getLog().lines();
        size_t start = log.size() > 15 ? log.size() - 15 : 0;
        for (size_t i = start; i < log.size(); ++i) log_lines.push_back(text(log[i]));

        return window(text("Battle") | bold | center, vbox({
            player_row,
            separator(),
            vbox(rows) | border,
            separator(),
            vbox({text("Log") | bold, vbox(log_lines)}) | border | size(HEIGHT, EQUAL, 17) | vscroll_indicator | frame,
            separator(),
            back_button->Render(),
        }));
    });

    return renderer;
}
