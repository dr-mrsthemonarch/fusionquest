#include "progression_system.h"
#include "../components/stats_component.h"

namespace FusionQuest {

    ProgressionSystem::ProgressionSystem(Engine* engine) : engine(engine) {
        std::random_device rd;
        rng = std::mt19937(rd());
    }

    void ProgressionSystem::initialize() {
        // Subscribe to relevant events
    }

    void ProgressionSystem::update(float deltaTime) {
        // Update character progression logic here
    }

    int ProgressionSystem::rollStat(int max) {
        std::uniform_int_distribution<> dist(1, max);
        return dist(rng);
    }

    float ProgressionSystem::rollChance() {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(rng);
    }

    void ProgressionSystem::generateRandomStats(GameObject* character) {
        auto* statsComp = character->getComponent<StatsComponent>();
        if (!statsComp) return;

        statsComp->setStat("hustle", rollStat(21));
        statsComp->setStat("agility", rollStat(21));
        statsComp->setStat("busfactor", rollChance());
        statsComp->setStat("techdebt", rollStat(21) * -1);
        statsComp->setStat("ndas", rollStat(21));
        statsComp->setStat("luck", rollStat(21));
        statsComp->setStat("synergy", rollStat(1));
    }

    bool ProgressionSystem::tryLevelUp(GameObject* character) {
        auto* statsComp = character->getComponent<StatsComponent>();
        if (!statsComp) return false;

        float experience = statsComp->getStat("experience");
        int currentSeries = static_cast<int>(statsComp->getStat("series"));

        if (experience >= 1.0f) {
            statsComp->setStat("experience", experience - 1.0f);
            statsComp->setStat("series", currentSeries + 1);
            statsComp->modifyStat("hustle", 1);
            statsComp->modifyStat("agility", 1);
            return true;
        }

        return false;
    }

} // namespace FusionQuest