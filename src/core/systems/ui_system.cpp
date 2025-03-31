#include "ui_system.h"
#include "../components/stats_component.h"
#include "../components/inventory_component.h"
#include "../components/equipment_component.h"
#include "../components/quest_component.h"
#include "../components/character_component.h"
namespace FusionQuest {

    UISystem::UISystem(Engine* engine) : engine(engine) {}

    void UISystem::initialize() {
        // Subscribe to events
    }

    void UISystem::updateUI() {
        // Update FTXUI components
    }

    GameObject* UISystem::createCharacterFromUI(const Character& uiCharacter) {
        GameObject* character = engine->createGameObject(uiCharacter.name);

        character->addComponent<StatsComponent>();
        character->addComponent<InventoryComponent>();
        character->addComponent<EquipmentComponent>();
        character->addComponent<QuestComponent>();
        auto* charComp = character->addComponent<CharacterComponent>();

        charComp->syncWithCharacter(uiCharacter);

        return character;
    }

    void UISystem::updateUICharacter(GameObject* gameObject, Character& uiCharacter) {
        auto* charComp = gameObject->getComponent<CharacterComponent>();
        if (charComp) {
            uiCharacter = charComp->toCharacter();
        }
    }

} // namespace FusionQuest