#include "GameEngineBridge.h"
#include "progression_system.h"
#include "quest_system.h"
#include "save_system.h"
#include "ui_system.h"
#include <iostream>

namespace FusionQuest {

GameEngineBridge::GameEngineBridge() {
    // Create engine
    engine = std::make_unique<Engine>();

    // Add systems
    progressionSystem = engine->addSystem<ProgressionSystem>(engine.get());
    questSystem = engine->addSystem<QuestSystem>(engine.get());
    saveSystem = engine->addSystem<SaveSystem>(engine.get());
    uiSystem = engine->addSystem<UISystem>(engine.get());

    // Initialize engine
    engine->initialize();
}

GameEngineBridge::~GameEngineBridge() {
    if (engine) {
        engine->shutdown();
    }
}

GameObject* GameEngineBridge::createPlayerFromCharacter(const Character& character) {
    // Destroy old player if it exists
    if (playerObject) {
        engine->destroyGameObject(playerObject->getID());
    }

    // Create new player
    playerObject = uiSystem->createCharacterFromUI(character);
    return playerObject;
}

void GameEngineBridge::updateCharacterFromPlayer(Character& character) {
    if (playerObject) {
        uiSystem->updateUICharacter(playerObject, character);
    }
}

GameObject* GameEngineBridge::getPlayer() {
    return playerObject;
}

void GameEngineBridge::update(float deltaTime) {
    if (engine) {
        engine->update(deltaTime);
    }
}

void GameEngineBridge::rollStats() {
    if (playerObject && progressionSystem) {
        progressionSystem->generateRandomStats(playerObject);
    }
}

ProgressionSystem* GameEngineBridge::getProgressionSystem() { return progressionSystem; }
QuestSystem* GameEngineBridge::getQuestSystem() { return questSystem; }
SaveSystem* GameEngineBridge::getSaveSystem() { return saveSystem; }
UISystem* GameEngineBridge::getUISystem() { return uiSystem; }
Engine* GameEngineBridge::getEngine() { return engine.get(); }

} // namespace FusionQuest

// ------------------------------------------------------
// Global Game Engine management
// ------------------------------------------------------
namespace FusionQuest {

static std::unique_ptr<GameEngineBridge> g_gameEngine = nullptr;

void InitializeGameEngine() {
    if (!g_gameEngine) {
        g_gameEngine = std::make_unique<GameEngineBridge>();
    }
}

GameEngineBridge* GetGameEngine() {
    return g_gameEngine.get();
}

void ShutdownGameEngine() {
    if (g_gameEngine) {
        g_gameEngine.reset();
    }
}

} // namespace FusionQuest