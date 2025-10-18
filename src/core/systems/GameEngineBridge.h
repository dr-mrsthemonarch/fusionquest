#pragma once

#include "../../classes/character.h"
#include "../engine_core.h"
#include <memory>
#include "../../core/components/character_component.h"
#include "../../core/components/equipment_component.h"
#include "../../core/components/inventory_component.h"
#include "../../core/components/quest_component.h"
#include "../../core/components/stats_component.h"
#include <ftxui/component/screen_interactive.hpp>
#include "progression_system.h"
#include "quest_system.h"
#include "save_system.h"
#include "ui_system.h"

namespace FusionQuest {

    // Bridge class to connect the new engine with your existing UI code
    class GameEngineBridge {
    public:
        GameEngineBridge();
        ~GameEngineBridge();

        // Create a game object from your existing Character class
        GameObject* createPlayerFromCharacter(const Character& character);

        // Update your existing Character class from the game object
        void updateCharacterFromPlayer(Character& character);

        // Get the player game object
        GameObject* getPlayer();

        // Update the engine (call this in your game loop)
        void update(float deltaTime);

        // Roll stats for character creation
        void rollStats();

        // Get systems for direct access
        ProgressionSystem* getProgressionSystem();
        QuestSystem* getQuestSystem();
        SaveSystem* getSaveSystem();
        UISystem* getUISystem();
        Engine* getEngine();

    private:
        std::unique_ptr<Engine> engine;
        GameObject* playerObject = nullptr;

        // System pointers
        ProgressionSystem* progressionSystem = nullptr;
        QuestSystem* questSystem = nullptr;
        SaveSystem* saveSystem = nullptr;
        UISystem* uiSystem = nullptr;
    };

    // Global functions for engine management
    // These should be implemented in game_engine_bridge.cpp

    void InitializeGameEngine();
    GameEngineBridge* GetGameEngine();
    void ShutdownGameEngine();

} // namespace FusionQuest
