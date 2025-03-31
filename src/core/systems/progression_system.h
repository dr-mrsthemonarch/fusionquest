#ifndef PROGRESSION_SYSTEM_H
#define PROGRESSION_SYSTEM_H

#include "../engine_core.h"
#include <random>
#include "progression_system.h"

namespace FusionQuest {

    class ProgressionSystem : public System {
    public:
        ProgressionSystem(Engine* engine);

        void initialize() override;
        void update(float deltaTime) override;

        int rollStat(int max);
        float rollChance();
        void generateRandomStats(GameObject* character);
        bool tryLevelUp(GameObject* character);

    private:
        Engine* engine;
        std::mt19937 rng;
    };

} // namespace FusionQuest

#endif // PROGRESSION_SYSTEM_H