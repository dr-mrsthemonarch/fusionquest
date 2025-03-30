#include "ui.h"
#include "character.h"
#include <ftxui/component/screen_interactive.hpp>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "core/engine_core.h"
#include "core/engine_components.h"
#include "core/engine_systems.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace FusionQuest;

int main() {
    // Create engine
    Engine engine;

    // Add systems
    auto* progressionSystem = engine.addSystem<ProgressionSystem>(&engine);
    auto* questSystem = engine.addSystem<QuestSystem>(&engine);
    auto* saveSystem = engine.addSystem<SaveSystem>(&engine);
    auto* uiSystem = engine.addSystem<UISystem>(&engine);

    // Initialize engine
    engine.initialize();

    // Create a player character
    GameObject* player = engine.createGameObject("Player");
    player->addComponent<StatsComponent>();
    player->addComponent<InventoryComponent>();
    player->addComponent<EquipmentComponent>();
    player->addComponent<QuestComponent>();
    player->addComponent<CharacterComponent>();

    // Set up player stats
    auto* statsComp = player->getComponent<StatsComponent>();
    statsComp->addStat("hustle", 10);
    statsComp->addStat("agility", 8);
    statsComp->addStat("busfactor", 0.7f);
    statsComp->addStat("techdebt", -5);
    statsComp->addStat("ndas", 3);
    statsComp->addStat("luck", 12);
    statsComp->addStat("synergy", 1);
    statsComp->addStat("runway", 0.8f);
    statsComp->addStat("hype_energy", 0.7f);
    statsComp->addStat("experience", 0.0f);
    statsComp->addStat("progress", 0.0f);
    statsComp->addStat("series", 1);

    // Set up player character info
    auto* charComp = player->getComponent<CharacterComponent>();
    charComp->setRaceIndex(2);  // Assuming 2 is a valid race index
    charComp->setClassIndex(1); // Assuming 1 is a valid class index
    charComp->setFusionIndex(3); // Assuming 3 is a valid fusion index

    // Add some items to inventory
    auto* inventoryComp = player->getComponent<InventoryComponent>();
    inventoryComp->addItem(InventoryComponent::Item("item1", "Fusion Core", "A high-energy power source"));
    inventoryComp->addItem(InventoryComponent::Item("item2", "NDA Contract", "Legal document to protect secrets"));

    // Add a quest
    auto* questComp = player->getComponent<QuestComponent>();
    auto mainQuest = questSystem->createQuest("quest1", "The First Fusion", "Complete your first fusion reactor");
    questSystem->addObjective(mainQuest, "obj1", "Acquire fusion materials");
    questSystem->addObjective(mainQuest, "obj2", "Build containment chamber");
    questSystem->addObjective(mainQuest, "obj3", "Test the reactor");
    questComp->addQuest(mainQuest);
    questComp->activateQuest("quest1");

    // Game loop (simplified)
    const float deltaTime = 0.016f; // ~60 FPS
    for (int i = 0; i < 100; ++i) { // Run for 100 frames as an example
        // Update engine
        engine.update(deltaTime);

        // For demonstration, complete an objective after some time
        if (i == 50) {
            questComp->completeObjective("quest1", "obj1");
            std::cout << "Completed objective: Acquire fusion materials\n";
        }

        // Give some experience
        if (i % 10 == 0) {
            statsComp->modifyStat("experience", 0.1f);
            std::cout << "Experience: " << statsComp->getStat("experience") << "\n";

            // Check for level up
            if (progressionSystem->tryLevelUp(player)) {
                std::cout << "Level up! New series: " << statsComp->getStat("series") << "\n";
            }
        }

        // In a real game, you would sync with the UI here

        // Sleep to simulate frame timing
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Save character (demonstration only)
    saveSystem->saveCharacter(player, "saves/example_player.sav");

    // Shut down engine
    engine.shutdown();

    return 0;
}