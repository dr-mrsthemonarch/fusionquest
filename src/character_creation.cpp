// character_creation.cpp
#include "../include/ui.h"
#include "../include/GameEngine.h"
#include "../include/SaveGame.h"
#include "../include/globals.h"
#include "../include/Archetype.h"
#include "../include/Tuning.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <random>
#include <ftxui/screen/terminal.hpp>

using namespace ftxui;

namespace {
std::mt19937& rng() {
    static std::mt19937 g(std::random_device{}());
    return g;
}
int rollStat(int range) { return static_cast<int>(rng()() % range); }

// same odds as original RollChance(): almost always 1.0
float rollChance() {
    static std::mt19937_64 gen(std::random_device{}());
    static std::uniform_int_distribution<uint64_t> dist(0, (1ULL << 39) - 1);
    constexpr uint64_t threshold = (1ULL << 39) / 344'000'000'000ULL;
    if (dist(gen) < threshold) {
        std::uniform_real_distribution<float> small(0.00001f, 1.0f);
        return small(gen);
    }
    return 1.0f;
}

// rolled bonuses, applied on top of Character base profile when game starts
struct Rolled {
    int hustle = 0, agility = 0, ndas = 0, luck = 0, synergy = 0, techdebt = 0;
    float busfactor = 0.0f;
};
}  // namespace

Component CreateCharacterCreationPage(int* selected_page, GameEngine* engine) {
    static std::string name;
    static int race_index = 0;
    static int fusion_index = 0;
    static Rolled rolled;
    static int class_index = 0;
    static std::vector<std::string> class_names;
    class_names = Archetypes::I().names();
    auto class_selector = Radiobox(&class_names, &class_index);

    auto name_input = Input(&name, "Enter character name");
    auto race_selector = Radiobox(&RACES, &race_index);
    auto fusion_selector = Radiobox(&FUSION_STARTUPS, &fusion_index);

    auto roll_button = Button("Roll!", [] {
        rolled.hustle = rollStat(21);
        rolled.agility = rollStat(21);
        rolled.busfactor = rollChance();
        rolled.techdebt = -rollStat(21);
        rolled.ndas = rollStat(21);
        rolled.luck = rollStat(21);
        rolled.synergy = rollStat(1);
    });

    auto lets_do_it_button = Button("Let's Do It", [selected_page, engine] {
        if (name.empty()) name = "Unnamed Founder";
        std::string employeeNr = std::to_string(1 + rng()() % 9999);
        engine->initializeCharacter(name, employeeNr, RACES[race_index], FUSION_STARTUPS[fusion_index], class_index);

        // apply rolled stats on top of base profile
        Character* c = engine->getCharacter();
        CharacterProfile p = c->getBaseProfile();
        p.hustle += rolled.hustle;
        p.agility += rolled.agility;
        p.ndas += rolled.ndas;
        p.luck += rolled.luck;
        p.synergy += rolled.synergy;
        p.technicalDebt += rolled.techdebt;
        {   // class flat mods (bus factor fixed at 1)
            const ClassDef& cd = Archetypes::I().get(class_index);
            const double ms = T("class.mod_scale");
            p.runway += static_cast<int>(cd.mods.runway * ms); p.hypeEnergy += static_cast<int>(cd.mods.hype * ms);
            p.hustle += static_cast<int>(cd.mods.hustle * ms); p.agility += static_cast<int>(cd.mods.agility * ms);
            p.ndas += static_cast<int>(cd.mods.ndas * ms); p.luck += static_cast<int>(cd.mods.luck * ms);
            p.synergy += static_cast<int>(cd.mods.synergy * ms);
        }
        c->setBaseProfileDirect(p);
        c->setMaxRunwayDirect(c->getProfile().runway); c->setRunway(c->getProfile().runway);
        c->setMaxHypeEnergyDirect(c->getProfile().hypeEnergy); c->setHypeEnergy(c->getProfile().hypeEnergy);

        engine->armNemesisIntro();   // holds first nemesis until the preamble ends
        SaveGame::saveSlot(*engine);  // original chargen also saved on create
        *selected_page = PREAMBLE_PAGE;
    });

    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; }) | size(HEIGHT, EQUAL, 4);

    auto race_container = Container::Vertical({race_selector});
    auto fusion_container = Container::Vertical({fusion_selector});

    auto race_renderer = Renderer(race_container, [race_selector] {
        return vbox({text("Race") | bold | center, separator(),
                     race_selector->Render() | vscroll_indicator | frame}) |
               border | size(HEIGHT, EQUAL, 26);
    });

    auto fusion_renderer = Renderer(fusion_container, [fusion_selector] {
        return vbox({text("Fusion Company") | bold | center, separator(),
                     fusion_selector->Render() | flex | vscroll_indicator | frame}) |
               border | size(HEIGHT, EQUAL, 26);
    });

    auto class_container = Container::Vertical({class_selector});
    auto class_renderer = Renderer(class_container, [class_selector] {
        return vbox({text("Class") | bold | center, separator(), class_selector->Render() | vscroll_indicator | frame}) |
               border | size(HEIGHT, EQUAL, 26);
    });

    auto container = Container::Vertical({
        name_input,
        race_container,
        fusion_container,
        class_container,
        roll_button,
        lets_do_it_button,
        back_button,
    });

    auto renderer = Renderer(container, [=] {
        Element race_column = race_renderer->Render();
        Element fusion_column = fusion_renderer->Render();

        const int total = rolled.ndas + rolled.luck + rolled.hustle + rolled.synergy + rolled.agility + rolled.techdebt;
        const int cw = std::max(24, (Terminal::Size().dimx - 4) / 4);   // fixed column width: rolls never reflow
        auto stat = [](const std::string& label, const std::string& v) {
            return hbox({text(label) | size(WIDTH, EQUAL, 17), text(v)}) | bold;
        };

        Element stats_column = vbox({
            text("Stats") | bold | center,
            separator(),
            stat("NDAs:", std::to_string(rolled.ndas)),
            stat("Luck:", std::to_string(rolled.luck)),
            stat("Hustle:", std::to_string(rolled.hustle)),
            stat("Patents:", std::to_string(rolled.synergy)),
            stat("Agile-ity:", std::to_string(rolled.agility)),
            stat("Bus Factor:", "1/1"),
            stat("Technical Debt:", std::to_string(rolled.techdebt)),
            separator(),
            stat("Total:", std::to_string(total)),
            separator(),
            roll_button->Render(),
            lets_do_it_button->Render(),
            filler(),
        }) | border | size(HEIGHT, EQUAL, 26);

        return window(text("Fusion Quest") | bold | center, vbox({
            hbox({text("Name: ") | bold, name_input->Render()}),
            separator(),
            hbox({race_column | size(WIDTH, EQUAL, cw), fusion_column | size(WIDTH, EQUAL, cw),
                  class_renderer->Render() | size(WIDTH, EQUAL, cw), stats_column | size(WIDTH, EQUAL, cw)}) | center,
            separator(),
            back_button->Render(),
        }));
    });

    return renderer;
}