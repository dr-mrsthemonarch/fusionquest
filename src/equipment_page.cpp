#include "../include/GameEngine.h"
#include "../include/ui.h"
#include "../include/globals.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

Component CreateEquipmentPage(int* selected_page, GameEngine* engine) {
    static int selected_index = 0;
    auto slot_list = std::make_shared<std::vector<std::string>>();
    auto menu = Menu(slot_list.get(), &selected_index);

    auto back_button = Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; });
    auto container = Container::Vertical({menu, back_button});

    auto renderer = Renderer(container, [=] {
        Character* c = engine->getCharacter();
        slot_list->clear();
        for (const auto& [type, eq] : c->getEquippedItemsMap()) {
            slot_list->push_back(eq.name + " | R+" + std::to_string(eq.runway) +
                " H+" + std::to_string(eq.hypeEnergy) + " Hu+" + std::to_string(eq.hustle) +
                " A+" + std::to_string(eq.agility) + " N+" + std::to_string(eq.ndas) +
                " L+" + std::to_string(eq.luck) + " S+" + std::to_string(eq.synergy));
        }
        for (const auto& eq : c->getEquipmentInventoryVec()) {
            slot_list->push_back("[unequipped] " + eq.name + " ($" + std::to_string(eq.equipmentvalue) + ")");
        }
        if (slot_list->empty()) slot_list->push_back("No equipment.");

        return window(text("Equipment") | bold | center, vbox({
            menu->Render() | vscroll_indicator | frame | flex,
            separator(),
            back_button->Render(),
        }));
    });

    return renderer;
}
