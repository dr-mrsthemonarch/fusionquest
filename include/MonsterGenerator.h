#ifndef MONSTERGENERATOR_H
#define MONSTERGENERATOR_H

#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <iostream>

#include "ItemGenerator.h"
#include "Monster.h"

// Forward declarations for scaling stat structures
struct MobStats {
    int health;
    int maxHealth;
    int attack;
    int defense;
    int speed;
    int moneyReward;
};

struct BossStats {
    int health;
    int maxHealth;
    int attack;
    int defense;
    int speed;
    int moneyReward;
};

class MonsterGenerator {
public:
    MonsterGenerator();

    // Load configuration files
    bool loadWordLists(const std::string& configPath);

    void generateBossPatents(Monster &boss, int count, int playerSeries);

    void generateMobPatents(Monster &mob, int count, int playerSeries);

    std::vector<std::unique_ptr<Monster>> generateGroupEncounter(
        int playerSeries, int& mobCount, int& bossCount, ItemGenerator& itemGen
    );

    std::unique_ptr<Monster> generateBossMinion(
        int playerSeries, ItemGenerator& itemGen, float powerScale = 0.5f
    );

    // Generate monsters
    Monster generateMob(int playerSeries, ItemGenerator &itemGen);

    void generateMonsterPatents(Monster &monster, int count);

    Monster generateBoss(int playerSeries);
    std::vector<Monster> generateEncounter(int playerSeries, int &mobCount, int &bossCount, ItemGenerator &itemGen);

    // Check if generator is ready
    bool isLoaded() const { return loadedSuccessfully_; }

private:
    // Word lists for name generation
    std::vector<std::string> bossNames_;
    std::vector<std::string> mobNames_;
    std::vector<std::string> titles_;
    std::vector<std::string> adjectives_;
    std::vector<std::string> mythicalNouns_;

    // Internal state
    bool loadedSuccessfully_;
    std::mt19937 rng_;

    // Helper functions for loading
    bool loadWordList(const std::string& filename, std::vector<std::string>& wordList);

    // Name generation
    std::string generateMobName();
    std::string generateBossName();
    MonsterDifficulty getRandomDifficulty();

    // Mathematical scaling functions
    MobStats calculateMobStats(int playerSeries, MonsterDifficulty difficulty);
    BossStats calculateBossStats(int playerSeries, MonsterDifficulty difficulty);
};

// Mathematical scaling namespace (can be used by other systems)
namespace MonsterScaling {
    // Linear scaling: base + (level * growth)
    double linearScale(int level, double base, double growthPerLevel);

    // Polynomial scaling: base * (1 + level)^exponent
    double polynomialScale(int level, double base, double exponent);

    // Exponential scaling: base * multiplier^level
    double exponentialScale(int level, double base, double multiplier);

    // Difficulty multipliers
    double getDifficultyMultiplier(MonsterDifficulty difficulty);

    // Add random variance (±percentage)
    double applyVariance(double value, double variancePercent, std::mt19937& rng);
}

#endif // MONSTERGENERATOR_H