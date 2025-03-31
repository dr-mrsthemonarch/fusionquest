#include "save_system.h"
#include "../components/stats_component.h"
#include "../components/inventory_component.h"
#include "../components/equipment_component.h"
#include "../components/quest_component.h"
#include "../components/character_component.h"

namespace FusionQuest {

    SaveSystem::SaveSystem(Engine* engine) : engine(engine) {}

    void SaveSystem::initialize() {
        system("mkdir -p saves");
    }

    bool SaveSystem::saveCharacter(GameObject* character, const std::string& filename) {
        auto* charComp = character->getComponent<CharacterComponent>();
        if (!charComp) return false;

        Character oldFormatChar = charComp->toCharacter();
        // Actual file saving would be implemented here
        return true;
    }

    GameObject* SaveSystem::loadCharacter(const std::string& filename) {
        GameObject* character = engine->createGameObject("LoadedCharacter");

        character->addComponent<StatsComponent>();
        character->addComponent<InventoryComponent>();
        character->addComponent<EquipmentComponent>();
        character->addComponent<QuestComponent>();
        character->addComponent<CharacterComponent>();

        // Actual loading would be implemented here

        return character;
    }

} // namespace FusionQuest