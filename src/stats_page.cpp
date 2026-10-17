#include "../include/GameEngine.h"
#include "../include/GameClock.h"
#include "../include/SaveGame.h"
#include "../include/Patent.h"
#include "../include/ui.h"
#include "../include/globals.h"
#include "progress_bar.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// NOTE: layout tree (vbox/hbox/border/window structure) kept identical to original.
// only data sources changed: hardcoded/global lists -> GameEngine state, pulled fresh
// each render frame (ftxui's existing 100ms refresh-thread pattern already re-renders
// everything, so no extra polling infra needed).

int selected_equipment_index = 0;
int selected_patent_index = 0;
int selected_inventory_index = 0;

Component CreateStatsPage(int* selected_page, GameEngine* engine,
                           std::shared_ptr<CountdownComponent> countdown_timer,
                           Closure exit_closure) {
    auto experience_bar = std::make_shared<ProgressBar>(0.0f, 0.01f, true);
    auto progress_bar = std::make_shared<ProgressBar>(0.0f, 0.005f, true);
    auto encumberance_bar = std::make_shared<ProgressBar>(0.0f, 0.001f, true);
    experience_bar->Start(); encumberance_bar->Start(); progress_bar->Start();

    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; }) | size(HEIGHT, EQUAL, 3);
    auto save_button = Button("Save", [engine] { SaveGame::save(*engine, "saves/quicksave.sav"); }) | size(HEIGHT, EQUAL, 3);
    auto quit_button = Button("Quit", std::move(exit_closure)) | size(HEIGHT, EQUAL, 3);

    // persistent backing vectors for Menu widgets - contents refilled each render, pointer stable
    auto equipment_list = std::make_shared<std::vector<std::string>>();
    auto patent_list = std::make_shared<std::vector<std::string>>();
    auto inventory_list = std::make_shared<std::vector<std::string>>();

    auto equipment_items = Menu(equipment_list.get(), &selected_equipment_index);
    auto patent_items = Menu(patent_list.get(), &selected_patent_index);
    auto inventory_items = Menu(inventory_list.get(), &selected_inventory_index);

    auto container = Container::Vertical({back_button, save_button, quit_button,
                                           equipment_items, patent_items, inventory_items});

    auto renderer = Renderer(container, [=] {
        Character* c = engine->getCharacter();

        // refill backing lists from live engine state
        equipment_list->clear();
        for (const auto& [type, eq] : c->getEquippedItemsMap())
            equipment_list->push_back(eq.name + " (+" + std::to_string(eq.equipmentvalue) + ")");

        patent_list->clear();
        for (const auto& [name, patent] : Patent::characterPatents)
            patent_list->push_back(patent.getFullName() + " | Pwr " + std::to_string(patent.getCurrentPower()));

        inventory_list->clear();
        for (const auto& [name, val] : c->getInventoryMap())
            inventory_list->push_back(name + " ($" + std::to_string(val) + ")");

        experience_bar->SetProgress(static_cast<float>(c->getFundingProgress() / 100.0));
        encumberance_bar->SetProgress(static_cast<float>(c->getInventoryCount()) / std::max(1, c->getInventoryLimit()));

        Element character_stats =
            vbox({
                text("Character") | bold | center,
                separator(),
                hbox({text("Name: ") | bold | size(WIDTH, EQUAL, 16), text(c->getName())}),
                hbox({text("Position: ") | bold | size(WIDTH, EQUAL, 16), text(c->getStartup())}),
                separator(),
                hbox({text("Runway: ") | bold | size(WIDTH, EQUAL, 16),
                      text(std::to_string(c->getRunway()) + "/" + std::to_string(c->getMaxRunway()))}),
                hbox({text("Hype Energy: ") | bold | size(WIDTH, EQUAL, 16),
                      text(std::to_string(c->getHypeEnergy()) + "/" + std::to_string(c->getMaxHypeEnergy()))}),
                hbox({text("Money: ") | bold | size(WIDTH, EQUAL, 16), text("$" + std::to_string(c->getMoney()))}),
                separator(),
                hbox({text("Hustle: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getModifiedHustle()))}),
                hbox({text("Agile-ity: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getModifiedAgility()))}),
                hbox({text("NDAs: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getModifiedNDAs()))}),
                hbox({text("Luck: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getModifiedLuck()))}),
                hbox({text("Synergy: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getModifiedSynergy()))}),
                hbox({text("Technical Debt: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getTechnicalDebt()))}),
                hbox({text("Bus Factor: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getBusFactor()))}),
            }) | border;

        Element experience_gauge = window(
            text("Funding") | bold | center,
            vbox({
                gauge(experience_bar->GetProgress()) | color(Color::Yellow),
                text("Series " + std::to_string(c->getSeries()) +
                     " (" + std::to_string(int(experience_bar->GetProgress() * 100)) + "%)") | center,
            }));

        Element patents =
            vbox({text("Patents") | bold | center, separator(),
                  patent_items->Render() | vscroll_indicator | frame}) |
            border | size(HEIGHT, LESS_THAN, 14);

        Element inventory =
            vbox({text("Bees Knees Central") | bold | center, separator(),
                  inventory_items->Render() | vscroll_indicator | frame,
                  vbox({gauge(encumberance_bar->GetProgress()) | color(Color::Red),
                        text("Storage (" + std::to_string(c->getInventoryCount()) + "/"
                             + std::to_string(c->getInventoryLimit()) + ")") | center}) |
                      border | size(HEIGHT, EQUAL, 11)}) |
            border | size(HEIGHT, EQUAL, 18);

        // battle log replaces static "Quests"/"Project Management" placeholder panels
        Element battle_log_panel = vbox({
            text("Battle Log") | bold | center,
            separator(),
            [&] {
                std::vector<Element> lines;
                const auto& log = engine->getLog().lines();
                size_t start = log.size() > 12 ? log.size() - 12 : 0;
                for (size_t i = start; i < log.size(); ++i) lines.push_back(text(log[i]));
                return vbox(std::move(lines));
            }(),
        }) | border | size(HEIGHT, EQUAL, 20);

        Element equipment =
            vbox({text("Equipment") | bold | center, separator(),
                  equipment_items->Render() | vscroll_indicator | frame}) |
            border | size(HEIGHT, EQUAL, 12);

        Element progress_gauge =
            window(text("Progress") | bold | center,
                   vbox({text("Battles won: " + std::to_string(engine->getCounters().totalVictories)),
                         gauge(std::min(1.0f, c->getSeries() / 20.0f)) | color(Color::Blue)})) |
            size(HEIGHT, EQUAL, 4);

        return window(countdown_timer->Render(),
                       vbox({
                           hbox({
                               vbox({character_stats, experience_gauge, equipment | flex}) | flex,
                               vbox({inventory | flex, patents | flex}) | flex,
                               vbox({battle_log_panel}) | flex,
                           }),
                           progress_gauge,
                           separator(),
                           hbox({back_button->Render(), save_button->Render(), quit_button->Render()}) | center,
                       }));
    });

    return renderer;
}
