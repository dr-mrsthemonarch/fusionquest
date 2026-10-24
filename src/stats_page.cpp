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
#include <deque>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

using namespace ftxui;

// layout tree identical to original fusionquest stats page. only data sources changed:
// hardcoded text -> GameEngine state. Quests + Project Management panels are still the
// original static placeholder text (no engine concept for them yet).

// activity text pacing: log lines arrive faster than anyone can read them. each line is shown
// at least TEXT_MIN_DWELL; if a backlog builds up, lines flip faster so it never lags far behind.
constexpr std::chrono::milliseconds TEXT_MIN_DWELL{1800};
constexpr std::chrono::milliseconds TEXT_FAST_DWELL{600};
constexpr size_t TEXT_BACKLOG_FAST = 3;  // pending lines before switching to fast dwell
constexpr size_t TEXT_BACKLOG_MAX = 6;   // older pending lines are dropped past this

struct TextPacer {
    size_t seen = 0;
    std::deque<std::string> pending;
    std::string shown = "Idle";
    std::chrono::steady_clock::time_point since{};
};

int selected_patent_index = 0;
int selected_inventory_index = 0;

namespace {
// utf-8 safe shorten to w columns (code points), "..." suffix
std::string fit(const std::string& s, int w) {
    if (w < 4) return s;
    int cnt = 0;
    size_t i = 0, cut = std::string::npos;
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t len = c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
        if (cnt == w - 3) cut = i;
        ++cnt;
        i += len;
    }
    if (cnt <= w) return s;
    return s.substr(0, cut) + "...";
}
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

    auto pacer = std::make_shared<TextPacer>();
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
    auto log_button =
        Button("Battle Log", [selected_page] { *selected_page = BATTLE_PAGE; }) | size(HEIGHT, EQUAL, 3);
    auto special_button =
        Button("Special Items", [selected_page] { *selected_page = SPECIAL_DROPS_PAGE; }) | size(HEIGHT, EQUAL, 3);
    auto quit_button = Button("Quit", std::move(exit_closure)) | size(HEIGHT, EQUAL, 3);

    // Menu backing vectors: stable pointers, contents refilled each frame
    auto patent_list = std::make_shared<std::vector<std::string>>();
    auto inventory_list = std::make_shared<std::vector<std::string>>();
    auto patent_items = Menu(patent_list.get(), &selected_patent_index);
    auto inventory_items = Menu(inventory_list.get(), &selected_inventory_index);

    auto container = Container::Vertical({back_button, save_button, log_button, special_button, quit_button, patent_items, inventory_items});

    auto renderer = Renderer(container, [=] {
        // no game started yet (e.g. opened via main menu "Stats Page"): draw the SAME layout
        // with default/empty values instead of a different screen.
        static Character placeholder("", "", "", "");
        Character* c = engine->getCharacter();
        if (!c) c = &placeholder;

        // three equal fixed-width columns: long names can no longer stretch the layout
        const int colw = std::max(36, (Terminal::Size().dimx - 2) / 3);
        const int tw = colw - 4; // border + scroll indicator

        patent_list->clear();
        for (const auto& [name, patent] : Patent::characterPatents)
            patent_list->push_back(fit(patent.getFullName(), tw));
        if (patent_list->empty()) patent_list->push_back("(none)");

        inventory_list->clear();
        for (const auto& [name, val] : c->getInventoryMap()) {
            int n = c->getInventoryStack(name);
            std::string suffix = (n > 1 ? " x" + std::to_string(n) : "") + " ($" + std::to_string(val) + ")";
            inventory_list->push_back(fit(name, std::max(8, tw - static_cast<int>(suffix.size()))) + suffix);
        }
        if (inventory_list->empty()) inventory_list->push_back("(empty)");

        float funding_frac = engine->getFundingFraction();
        float activity_frac = engine->getActivityProgress();
        float encumb_frac = std::clamp(
            static_cast<float>(c->getInventoryCount()) / std::max(1, c->getInventoryLimit()), 0.0f, 1.0f);
        pushTarget(experience_bar, funding_frac);
        pushTarget(activity_bar, activity_frac);
        encumberance_bar->SetTarget(encumb_frac); // animate down too, so selloff visibly retracts

        // ---- Column 1: character ----
        Element character_stats =
            vbox({
                text("Character") | bold | center,
                separator(),
                hbox({text("Name: ") | bold | size(WIDTH, EQUAL, 16), text(fit(c->getName().empty() ? "Unknown" : c->getName(), tw - 16))}),
                hbox({text("Employee nr: ") | bold | size(WIDTH, EQUAL, 16), text(engine->getEmployeeNr())}),
                hbox({text("Position: ") | bold | size(WIDTH, EQUAL, 16), text(engine->getPosition())}),
                hbox({text("Startup Name: ") | bold | size(WIDTH, EQUAL, 16), text(fit(c->getStartup(), tw - 16))}),
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
        QuestSystem* qs = engine->getQuestSystem();
        auto qgauge = [tw](const std::string& label, const QuestSystem::Gauge& g, Color col) {
            return vbox({text(fit(label + g.title, tw)) | color(col), gauge(g.frac) | color(col)});
        };
        Element quests =
            vbox({text("Quests") | bold | center, separator(),
                  qgauge("Main: ", qs->mainGauge(), Color::Yellow), separator(),
                  qgauge("Hustle: ", qs->hustleGauge(), Color::GreenLight), separator(),
                  qgauge("Dev: ", qs->devGauge(), Color::DeepPink4), separator(),
                  text("Active Quests: " + std::to_string(qs->activeCount()) + "/" + std::to_string(qs->maxActive())) |
                      bold | size(HEIGHT, EQUAL, 1),
                  text("Completed Projects: " + std::to_string(qs->completedCount())) | bold | size(HEIGHT, EQUAL, 1)}) |
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
        std::vector<Element> pm;
        pm.push_back(text("Project Management") | bold | center);
        pm.push_back(separator());
        const Quest* aq = qs->activeQuest();
        const Dev* ad = qs->activeDev();
        if (!aq || !ad) {
            pm.push_back(text("No active project") | dim);
        } else {
            pm.push_back(text(fit(aq->title, tw)) | color(Color::Green));
            pm.push_back(text(fit("Dev: " + ad->name, tw)) | color(Color::Yellow));
            pm.push_back(separator());
            for (size_t i = 0; i < ad->shown.size() && i < 2; ++i) {
                const Sprint& sp = ad->shown[i];
                const bool cur = (i == 0);
                pm.push_back(text(fit(std::string(cur ? "➤ " : "□ ") + sp.name, tw)) |
                             color(cur ? Color::Yellow : Color::GrayDark));
                for (const Goal& g : sp.goals) {
                    std::string mark = g.done() ? "✓ " : (cur ? "➤ " : "□ ");
                    pm.push_back(text(fit("   " + mark + g.describe(), tw)) |
                                 color(g.done() ? Color::Green : (cur ? Color::Yellow : Color::GrayDark)));
                }
            }
            pm.push_back(separator());
            pm.push_back(text("Project Progress: Sprint " +
                              std::to_string(std::min(ad->current + 1, ad->sprintCount)) + " of " +
                              std::to_string(ad->sprintCount)) | bold);
        }
        Element plot_dev = vbox(std::move(pm)) | border | size(HEIGHT, EQUAL, 20);

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
                         text(bonus + " ") | size(WIDTH, EQUAL, 8), text(fit(name, std::max(8, tw - 24)))});
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
        // paced activity text: newest log lines are queued and shown one at a time
        const auto now = std::chrono::steady_clock::now();
        const BattleLog& blog = engine->getLog();
        if (blog.totalAdded() < pacer->seen) { pacer->seen = blog.totalAdded(); pacer->pending.clear(); }
        size_t fresh = std::min(blog.totalAdded() - pacer->seen, blog.lines().size());
        for (size_t i = blog.lines().size() - fresh; i < blog.lines().size(); ++i)
            pacer->pending.push_back(blog.lines()[i]);
        pacer->seen = blog.totalAdded();
        while (pacer->pending.size() > TEXT_BACKLOG_MAX) pacer->pending.pop_front();
        auto dwell = pacer->pending.size() > TEXT_BACKLOG_FAST ? TEXT_FAST_DWELL : TEXT_MIN_DWELL;
        if (!pacer->pending.empty() && now - pacer->since >= dwell) {
            pacer->shown = pacer->pending.front();
            pacer->pending.pop_front();
            pacer->since = now;
        }
        std::string activity = pacer->shown;
        if (!save_msg->empty() &&
            std::chrono::steady_clock::now() - *save_time < std::chrono::seconds(4))
            activity = *save_msg;

        Element progress_gauge =
            window(text("Progress") | bold | center,
                   vbox({text(fit(activity, colw * 3 - 12) + " " + std::to_string(int(activity_bar->GetProgress() * 100)) + "%"),
                         gauge(activity_bar->GetProgress()) | color(Color::Blue)})) |
            size(HEIGHT, EQUAL, 4);

        return window(countdown_timer->Render(),
                      vbox({
                          hbox({
                              vbox({character_stats, experience_gauge, equipment | flex}) | size(WIDTH, EQUAL, colw),
                              vbox({inventory, patents | flex}) | size(WIDTH, EQUAL, colw),
                              vbox({plot_dev, quests}) | size(WIDTH, EQUAL, colw),
                          }),
                          progress_gauge,
                          separator(),
                          hbox({back_button->Render(), save_button->Render(), log_button->Render(), special_button->Render(),
                                quit_button->Render()}) | center,
                      }));
    });

    return renderer;
}