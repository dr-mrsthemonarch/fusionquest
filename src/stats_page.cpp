#include "../include/GameEngine.h"
#include "../include/GameClock.h"
#include "../include/SaveGame.h"
#include "../include/Patent.h"
#include "../include/progress_bar.h"
#include "../include/ui.h"
#include "../include/globals.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// layout tree identical to original fusionquest stats page. only data sources changed:
// hardcoded text -> GameEngine state. Quests + Project Management panels are still the
// original static placeholder text (no engine concept for them yet).

int selected_patent_index = 0;
int selected_inventory_index = 0;

namespace {
std::string formatMoney(long long v) {
    std::string s = std::to_string(v < 0 ? -v : v);
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3) s.insert(i, ",");
    return (v < 0 ? "-" : "") + s + " $";
}
std::string pct1(float f) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%.1f", f * 100.0f);
    return buf;
}
// bars animate up toward target, but snap down (level-up reset, battle end, selloff)
void pushTarget(const std::shared_ptr<ProgressBar>& bar, float v) {
    if (v < bar->GetProgress()) bar->SetImmediate(v);
    else bar->SetTarget(v);
}
}  // namespace

Component CreateStatsPage(int* selected_page, GameEngine* engine,
                           std::shared_ptr<CountdownComponent> countdown_timer,
                           Closure exit_closure) {
    // funding = series progress, activity = current quest/battle progress, encumbrance = selloff
    auto experience_bar = std::make_shared<ProgressBar>(0.05f, 0.02f);
    auto activity_bar = std::make_shared<ProgressBar>(0.05f, 0.05f);
    auto encumberance_bar = std::make_shared<ProgressBar>(0.05f, 0.02f);
    experience_bar->Start();
    activity_bar->Start();
    encumberance_bar->Start();

    auto save_msg = std::make_shared<std::string>();
    auto save_time = std::make_shared<std::chrono::steady_clock::time_point>();

    auto back_button =
        Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; }) | size(HEIGHT, EQUAL, 3);
    auto save_button =
        Button("Save", [engine, save_msg, save_time] {
            std::string path;
            bool ok = engine->isInitialized() && SaveGame::saveSlot(*engine, &path);
            *save_msg = ok ? "Game saved: " + path : "Save failed (no character yet?)";
            *save_time = std::chrono::steady_clock::now();
        }) | size(HEIGHT, EQUAL, 3);
    auto quit_button = Button("Quit", std::move(exit_closure)) | size(HEIGHT, EQUAL, 3);

    // Menu backing vectors: stable pointers, contents refilled each frame
    auto patent_list = std::make_shared<std::vector<std::string>>();
    auto inventory_list = std::make_shared<std::vector<std::string>>();
    auto patent_items = Menu(patent_list.get(), &selected_patent_index);
    auto inventory_items = Menu(inventory_list.get(), &selected_inventory_index);

    auto container = Container::Vertical({back_button, save_button, quit_button, patent_items, inventory_items});

    auto renderer = Renderer(container, [=] {
        // no game started yet (e.g. opened via main menu "Stats Page"): draw the SAME layout
        // with default/empty values instead of a different screen.
        static Character placeholder("", "", "", "");
        Character* c = engine->getCharacter();
        if (!c) c = &placeholder;

        patent_list->clear();
        for (const auto& [name, patent] : Patent::characterPatents)
            patent_list->push_back(patent.getFullName());
        if (patent_list->empty()) patent_list->push_back("(none)");

        inventory_list->clear();
        for (const auto& [name, val] : c->getInventoryMap())
            inventory_list->push_back(name + " ($" + std::to_string(val) + ")");
        if (inventory_list->empty()) inventory_list->push_back("(empty)");

        float funding_frac = engine->getFundingFraction();
        float activity_frac = engine->getActivityProgress();
        float encumb_frac = std::clamp(
            static_cast<float>(c->getInventoryCount()) / std::max(1, c->getInventoryLimit()), 0.0f, 1.0f);
        pushTarget(experience_bar, funding_frac);
        pushTarget(activity_bar, activity_frac);
        pushTarget(encumberance_bar, encumb_frac);

        // ---- Column 1: character ----
        Element character_stats =
            vbox({
                text("Character") | bold | center,
                separator(),
                hbox({text("Name: ") | bold | size(WIDTH, EQUAL, 16), text(c->getName().empty() ? "Unknown" : c->getName())}),
                hbox({text("Employee nr: ") | bold | size(WIDTH, EQUAL, 16), text(engine->getEmployeeNr())}),
                hbox({text("Position: ") | bold | size(WIDTH, EQUAL, 16), text(engine->getPosition())}),
                hbox({text("Startup Name: ") | bold | size(WIDTH, EQUAL, 16), text(c->getStartup())}),
                separator(),
                hbox({text("Runway: ") | bold | size(WIDTH, EQUAL, 16),
                      text(std::to_string(c->getRunway()) + "/" + std::to_string(c->getMaxRunway()))}),
                hbox({text("Hype Energy: ") | bold | size(WIDTH, EQUAL, 16),
                      text(std::to_string(c->getHypeEnergy()) + "/" + std::to_string(c->getMaxHypeEnergy()))}),
                hbox({text("Income: ") | bold | size(WIDTH, EQUAL, 16), text(formatMoney(c->getMoney()))}),
                separator(),
                hbox({text("Hustle: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getModifiedHustle()))}),
                hbox({text("Agile-ity: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getModifiedAgility()))}),
                hbox({text("NDAs: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getModifiedNDAs()))}),
                hbox({text("Luck: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getModifiedLuck()))}),
                hbox({text("Synergy: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getModifiedSynergy()))}),
                hbox({text("Technical Debt: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getTechnicalDebt()))}),
                hbox({text("Bus Factor: ") | bold | size(WIDTH, EQUAL, 16), text(std::to_string(c->getBusFactor()))}),
            }) |
            border;

        // funding bar = progress to next series (funding round)
        Element experience_gauge = window(
            text("Funding") | bold | center,
            vbox({
                gauge(experience_bar->GetProgress()) | color(Color::Yellow),
                text("Series " + std::to_string(c->getSeries()) + " (" + pct1(funding_frac) + "%)") | center,
            }));

        // fills whatever is left of column 2 below inventory
        Element patents =
            vbox({text("Patents") | bold | center, separator(),
                  patent_items->Render() | vscroll_indicator | frame | flex}) |
            border | flex;

        // ---- Column 2: quests (static placeholder, original text) ----
        Element quests =
            vbox({text("Quests") | bold | center, separator(),
                  text("Main Quest: Druming up the Beats") | color(Color::Yellow),
                  gauge(0.7f) | color(Color::Yellow), separator(),
                  text("Side Hustle: Comsol Consultancy") | color(Color::GreenLight),
                  gauge(0.3f) | color(Color::GreenLight), separator(),
                  text("Dev: Quantum Dots") | color(Color::DeepPink4),
                  gauge(0.5f) | color(Color::DeepPink4), separator(),
                  text("Active Quests: 3/5") | bold | size(HEIGHT, EQUAL, 1), separator(),
                  text("Completed Projects: 12") | bold | size(HEIGHT, EQUAL, 1)}) |
            border | size(HEIGHT, EQUAL, 16);

        // height 20 = same as character panel / Project Management, so the patents panel
        // top lines up with Funding (col 1) and Quests (col 3). item list absorbs spare
        // height, selloff box stays at its minimum height pinned to the bottom.
        Element inventory =
            vbox({text("Bees Knees Central") | bold | center, separator(),
                  inventory_items->Render() | vscroll_indicator | frame | flex,
                  vbox({gauge(encumberance_bar->GetProgress()) | color(Color::Red),
                        text("Selloff (" + std::to_string(int(encumb_frac * 100)) + "%)") | center}) |
                      border}) |
            border | size(HEIGHT, EQUAL, 20);

        // ---- Column 3: project management (static placeholder, original text) ----
        Element plot_dev =
            vbox({
                text("Project Management") | bold | center,
                separator(),
                text("Sprint 1: The Beginning") | color(Color::Green),
                text("✓ Got first Dollar") | color(Color::Green),
                text("✓ Sold Grandmothers Soul to NFT") | color(Color::Green),
                separator(),
                text("Sprint 2: Dark Tidings") | color(Color::Yellow),
                text("✓ Changed Politics") | color(Color::Green),
                text("➤ Confront the cultists") | color(Color::Yellow),
                text("□ Find abandoned warehouse") | color(Color::GrayDark),
                separator(),
                text("Project Progress: Sprint 2 of 5") | bold,
            }) |
            border | size(HEIGHT, EQUAL, 20);

        // equipment: same row format as original (slot | bonus | name), live from engine.
        // original had Pants/Shoes rows - no matching engine slot, so 8 real slots only.
        const auto& eqmap = c->getEquippedItemsMap();
        auto equip_row = [&](const char* label, EquipmentType t) {
            auto it = eqmap.find(t);
            std::string bonus = "-", name = "Empty";
            if (it != eqmap.end()) {
                const Equipment& e = it->second;
                int total = e.runway + e.hypeEnergy + e.hustle + e.agility + e.ndas + e.luck + e.synergy +
                            (t == EquipmentType::BACKPACK ? e.inventoryLimit : 0);
                bonus = "+" + std::to_string(total);
                name = e.name;
            }
            return hbox({text(std::string(label) + ": ") | bold | size(WIDTH, EQUAL, 16),
                         text(bonus + " ") | size(WIDTH, EQUAL, 8), text(name)});
        };
        Element equipment =
            vbox({
                text("Equipment") | bold | center,
                separator(),
                equip_row("Weapon", EquipmentType::LAPTOP),
                equip_row("Phone", EquipmentType::PHONE),
                equip_row("Backpack", EquipmentType::BACKPACK),
                equip_row("Biking Helmet", EquipmentType::HELMET),
                equip_row("Headphones", EquipmentType::HEADPHONES),
                equip_row("Bicycle", EquipmentType::BIKE),
                equip_row("Jacket", EquipmentType::JACKET),
                equip_row("Gloves", EquipmentType::GLOVES),
            }) |
            border | size(HEIGHT, EQUAL, 12);

        // bottom bar: progress of current activity. text above it = latest battle log line
        // (or save confirmation for 4s after pressing Save).
        std::string activity = "Idle";
        const auto& lines = engine->getLog().lines();
        if (!lines.empty()) activity = lines.back();
        if (!save_msg->empty() &&
            std::chrono::steady_clock::now() - *save_time < std::chrono::seconds(4))
            activity = *save_msg;

        Element progress_gauge =
            window(text("Progress") | bold | center,
                   vbox({text(activity + " " + std::to_string(int(activity_bar->GetProgress() * 100)) + "%"),
                         gauge(activity_bar->GetProgress()) | color(Color::Blue)})) |
            size(HEIGHT, EQUAL, 4);

        return window(countdown_timer->Render(),
                      vbox({
                          hbox({
                              vbox({character_stats, experience_gauge, equipment | flex}) | flex,
                              vbox({inventory, patents | flex}) | flex,
                              vbox({plot_dev, quests}) | flex,
                          }),
                          progress_gauge,
                          separator(),
                          hbox({back_button->Render(), save_button->Render(), quit_button->Render()}) | center,
                      }));
    });

    return renderer;
}