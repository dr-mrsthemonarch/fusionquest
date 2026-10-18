// character_creation.cpp
#include "../include/ui.h"
#include "../include/GameEngine.h"
#include "../include/SaveGame.h"
#include "../include/globals.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <random>

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
        engine->initializeCharacter(name, employeeNr, RACES[race_index], FUSION_STARTUPS[fusion_index]);

        // apply rolled stats on top of base profile
        Character* c = engine->getCharacter();
        CharacterProfile p = c->getBaseProfile();
        p.hustle += rolled.hustle;
        p.agility += rolled.agility;
        p.ndas += rolled.ndas;
        p.luck += rolled.luck;
        p.synergy += rolled.synergy;
        p.technicalDebt += rolled.techdebt;
        if (rolled.busfactor > 0.0f) p.busFactor = rolled.busfactor;
        c->setBaseProfileDirect(p);

        SaveGame::saveSlot(*engine);  // original chargen also saved on create
        *selected_page = STATS_PAGE;
    });

    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; }) | size(HEIGHT, EQUAL, 4);

    auto race_container = Container::Vertical({race_selector});
    auto fusion_container = Container::Vertical({fusion_selector});

    auto race_renderer = Renderer(race_container, [race_selector] {
        return vbox({text("Race") | bold | center, separator(),
                     race_selector->Render() | vscroll_indicator | frame}) |
               border | size(HEIGHT, LESS_THAN, 30);
    });

    auto fusion_renderer = Renderer(fusion_container, [fusion_selector] {
        return vbox({text("Fusion Company") | bold | center, separator(),
                     fusion_selector->Render() | flex | vscroll_indicator | frame}) |
               border | size(HEIGHT, LESS_THAN, 30);
    });

    auto container = Container::Vertical({
        name_input,
        race_container,
        fusion_container,
        roll_button,
        lets_do_it_button,
        back_button,
    });

    auto renderer = Renderer(container, [=] {
        Element race_column = race_renderer->Render();
        Element fusion_column = fusion_renderer->Render();

        float total = rolled.ndas + rolled.luck + rolled.hustle + rolled.synergy + rolled.agility +
                      rolled.busfactor + rolled.techdebt;

        Element stats_column = vbox({
            text("Stats") | bold | center,
            separator(),
            text("NDAs: " + std::to_string(rolled.ndas)) | bold,
            text("Luck: " + std::to_string(rolled.luck)) | bold,
            text("Hustle: " + std::to_string(rolled.hustle)) | bold,
            text("Patents: " + std::to_string(rolled.synergy)) | bold,
            text("Agile-ity: " + std::to_string(rolled.agility)) | bold,
            text("Bus Factor: " + std::to_string(rolled.busfactor)) | bold,
            text("Technical Debt: " + std::to_string(rolled.techdebt)) | bold,
            separator(),
            text("Total: " + std::to_string(static_cast<int>(total))) | bold | center,
            separator(),
            roll_button->Render(),
            lets_do_it_button->Render(),
        }) | border;

        return window(text("Fusion Quest") | bold | center, vbox({
            hbox({text("Name: ") | bold, name_input->Render()}),
            separator(),
            hbox({race_column | flex, fusion_column | flex, stats_column | flex}),
            separator(),
            back_button->Render(),
        }));
    });

    return renderer;
}