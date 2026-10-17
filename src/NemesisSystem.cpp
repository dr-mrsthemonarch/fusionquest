#include "../include/NemesisSystem.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>
#include <map>

#include "../include/Patent.h"
#include "../include/PatentStrategy.h"

// Game constants
constexpr double TOTAL_GAME_TIME = 20.0 * 365 * 24 * 60 * 60; // 20 years in seconds
constexpr double SECONDS_PER_YEAR = 365.25 * 24 * 60 * 60; // One year in seconds
constexpr double NEMESIS_INTERVAL = SECONDS_PER_YEAR; // Appears once per year

NemesisSystem::NemesisSystem() : nemesisPowerLevel_(0) {
    encounterHistory_.reserve(20); // Max 20 encounters (one per year)
}

NemesisSystem::NemesisStats NemesisSystem::calculateNemesisStats(int playerSeries, double elapsedSeconds, NemesisType type, int powerLevel) {
    NemesisStats stats;

    // Normalize time progression (0.0 to 1.0 over 20 years)
    double timeProgression = std::clamp(elapsedSeconds / TOTAL_GAME_TIME, 0.0, 1.0);

    constexpr double K = 12.0; // Sigmoid steepness
    constexpr double C = 0.5; // Midpoint

    double timePressure = 1.0 / (1.0 + std::exp(-K * (timeProgression - C)));
    double scalingFactor = 1.0 + (timePressure * 8.0); // 1.0x to 9.0x scaling

    // Nemesis type multipliers
    double typeMultiplier = 1.0;
    switch (type) {
        case NemesisType::ALPHA: typeMultiplier = 1.0;
            break; // Years 1-5
        case NemesisType::BETA: typeMultiplier = 1.5;
            break; // Years 6-10
        case NemesisType::OMEGA: typeMultiplier = 2.2;
            break; // Years 11-15
        case NemesisType::PRIME: typeMultiplier = 3.5;
            break; // Years 16-20
    }

    // Power level multiplier: 1.1x per defeat (stacks multiplicatively)
    double powerMultiplier = std::pow(1.1, powerLevel);

    // Base stats that scale with player series
    double baseHealth = 100.0 + (playerSeries * 30.0);
    double baseAttack = 15.0 + (playerSeries * 4.0);
    double baseDefense = 10.0 + (playerSeries * 3.5);
    double baseSpeed = 12.0 + (playerSeries * 2.5);
    double baseMoney = 200.0 + (playerSeries * 50.0);

    // Apply overwhelming nemesis multipliers
    // Nemesis should be 3-5x stronger than a normal boss
    constexpr double NEMESIS_HEALTH_MULT = 4.0;
    constexpr double NEMESIS_ATTACK_MULT = 2.0; // High but not instant-death
    constexpr double NEMESIS_DEFENSE_MULT = 2.5;
    constexpr double NEMESIS_SPEED_MULT = 1.8;
    constexpr double NEMESIS_MONEY_MULT = 10.0;

    // Calculate final stats (including power level boost!)
    stats.maxHealth = static_cast<int>(baseHealth * scalingFactor * typeMultiplier * powerMultiplier * NEMESIS_HEALTH_MULT);
    stats.health = stats.maxHealth;

    stats.attack = static_cast<int>(baseAttack * scalingFactor * typeMultiplier * powerMultiplier * NEMESIS_ATTACK_MULT);

    stats.defense = static_cast<int>(baseDefense * scalingFactor * typeMultiplier * powerMultiplier * NEMESIS_DEFENSE_MULT);

    stats.speed = static_cast<int>(baseSpeed * scalingFactor * typeMultiplier * powerMultiplier * NEMESIS_SPEED_MULT);

    stats.moneyReward = static_cast<int>(baseMoney * scalingFactor * typeMultiplier * powerMultiplier * NEMESIS_MONEY_MULT);

    return stats;
}

Monster NemesisSystem::debugGenerateNemesis(int playerSeries, double debugTimeSeconds, const std::string &playerName) {
    std::cout << "\n🔧 DEBUG MODE: Generating nemesis at time: " << debugTimeSeconds << " seconds" << std::endl;

    int debugYear = static_cast<int>(debugTimeSeconds);
    // int debugYear = static_cast<int>(debugTimeSeconds / SECONDS_PER_YEAR) + 1;
    std::cout << "Debug Year: " << debugYear << " | Player Series: " << playerSeries << std::endl;
    std::cout << "Current Nemesis Power Level: " << nemesisPowerLevel_ << std::endl;

    Monster debugNemesis = generateNemesis(playerSeries, debugTimeSeconds, playerName);

    std::cout << "\n📊 DEBUG NEMESIS STATS:" << std::endl;
    std::cout << "Name: " << debugNemesis.name << std::endl;
    std::cout << "Level: " << debugNemesis.level << std::endl;
    std::cout << "Health: " << debugNemesis.maxHealth << std::endl;
    std::cout << "Attack: " << debugNemesis.attack << std::endl;
    std::cout << "Defense: " << debugNemesis.defense << std::endl;
    std::cout << "Speed: " << debugNemesis.speed << std::endl;
    std::cout << "Reward: $" << debugNemesis.moneyReward << std::endl;

    return debugNemesis;
}

Monster NemesisSystem::generateNemesis(int playerSeries, double elapsedSeconds, const std::string &playerName) {
    Monster nemesis;

    nemesis.type = MonsterType::NEMESIS;
    nemesis.difficulty = MonsterDifficulty::LEGENDARY;

    int year = static_cast<int>(elapsedSeconds / SECONDS_PER_YEAR) + 1;
    NemesisType nemesisType = getNemesisType(elapsedSeconds);

    std::string baseName = generateNemesisName(nemesisType, nemesisPowerLevel_, playerName);
    nemesis.name = baseName + " (Year " + std::to_string(year) + ")";

    NemesisStats stats = calculateNemesisStats(playerSeries, elapsedSeconds, nemesisType, nemesisPowerLevel_);

    nemesis.level = playerSeries + 5;
    nemesis.maxHealth = stats.maxHealth;
    nemesis.health = stats.health;
    nemesis.attack = stats.attack;
    nemesis.defense = stats.defense;
    nemesis.speed = stats.speed;
    nemesis.moneyReward = stats.moneyReward;

    // Nemesis gets MASSIVE hype energy
    nemesis.maxHypeEnergy = 120 + (playerSeries * 20) + (nemesisPowerLevel_ * 30);
    nemesis.hypeEnergy = nemesis.maxHypeEnergy;

    // Nemesis uses TACTICAL strategy (smart and deadly)
    nemesis.setStrategy(std::make_unique<TacticalStrategy>());

    // Nemesis gets 4-7 patents depending on type and power level
    int patentCount = 4 + (static_cast<int>(nemesisType)) + (nemesisPowerLevel_ / 2);
    patentCount = std::min(patentCount, 8); // Cap at 8
    generateNemesisPatents(nemesis, patentCount, playerSeries, nemesisType);

    nemesis.itemReward = "NEMESIS_EQUIPMENT";
    return nemesis;
}

std::string NemesisSystem::generateNemesisName(NemesisType type, int powerLevel, const std::string &playerName) {
    std::string prefix;

    if (powerLevel == 0) {
        // First encounter - use original nemesis names
        switch (type) {
            case NemesisType::ALPHA: prefix = "The Disruptor";
                break;
            case NemesisType::BETA: prefix = "The Unicorn Slayer";
                break;
            case NemesisType::OMEGA: prefix = "The Market Dominator";
                break;
            case NemesisType::PRIME: prefix = "Elongated Muskard";
                break;
        }
    } else {
        // Nemesis has been defeated - takes on scrambled player name
        std::string scrambledName = scramblePlayerName(playerName);

        // Add power level indicator
        std::string title;
        if (powerLevel == 1) title = "Clone";
        else if (powerLevel == 2) title = "Shadow";
        else if (powerLevel == 3) title = "Phantom";
        else if (powerLevel == 4) title = "Doppelganger";
        else if (powerLevel >= 5) title = "Nightmare";

        prefix = title + " " + scrambledName;
    }

    // Add power level indicator for super-powered nemeses
    if (powerLevel > 0) {
        prefix += " [+" + std::to_string(powerLevel) + "]";
    }

    return prefix;
}

double NemesisSystem::getTimeUntilNextNemesis(double elapsedSeconds) const {
    // Calculate which year we're in
    int currentYear = static_cast<int>(elapsedSeconds / SECONDS_PER_YEAR) + 1;

    // Time when next nemesis appears
    double nextNemesisTime = currentYear * SECONDS_PER_YEAR;

    return std::max(0.0, nextNemesisTime - elapsedSeconds);
}

NemesisType NemesisSystem::getNemesisType(double elapsedSeconds) const {
    int year = static_cast<int>(elapsedSeconds / SECONDS_PER_YEAR) + 1;

    if (year <= 5) return NemesisType::ALPHA;
    if (year <= 10) return NemesisType::BETA;
    if (year <= 15) return NemesisType::OMEGA;
    return NemesisType::PRIME;
}

void NemesisSystem::recordNemesisDefeat(int yearNumber, double gameTimeSeconds, NemesisType type, const std::string &nemesisName) {
    nemesisPowerLevel_++;

    std::cout << "\n🔥 NEMESIS EVOLUTION! 🔥" << std::endl;
    std::cout << "The defeated nemesis grows stronger..." << std::endl;
    std::cout << "Nemesis Power Level: " << nemesisPowerLevel_ << " (+" << (nemesisPowerLevel_ * 10) << "% stronger)" << std::endl;
    std::cout << "Next nemesis will be EVOLVED!" << std::endl;
}

void NemesisSystem::recordEncounter(const NemesisEncounter &encounter) {
    encounterHistory_.push_back(encounter);
}

std::string NemesisSystem::scramblePlayerName(const std::string &playerName) {
    if (playerName.length() <= 2) {
        return playerName; // Can't scramble names too short
    }

    std::string scrambled = playerName;

    // Keep first and last letters, scramble everything in between
    if (scrambled.length() > 2) {
        std::random_device rd;
        std::mt19937 g(rd());

        // Scramble only the middle characters (index 1 to length-2)
        std::string middle = scrambled.substr(1, scrambled.length() - 2);
        std::shuffle(middle.begin(), middle.end(), g);

        // Reconstruct: first + scrambled middle + last
        scrambled = scrambled[0] + middle + scrambled[scrambled.length() - 1];
    }

    return scrambled;
}

bool NemesisSystem::shouldNemesisAppear(double elapsedSeconds, double lastNemesisTime) {
    // Check if at least one year has passed since last nemesis
    double timeSinceLastNemesis = elapsedSeconds - lastNemesisTime;
    return timeSinceLastNemesis >= NEMESIS_INTERVAL;
}

void NemesisSystem::generateNemesisPatents(Monster& nemesis, int count, int playerSeries, NemesisType type) {
    std::vector<std::pair<std::string, PatentType>> nemesisPatents;

    switch (type) {
        case NemesisType::ALPHA:
            nemesisPatents = {
                {"Disruption Wave", PatentType::ATTACK},
                {"Market Pressure", PatentType::DEBUFF},
                {"Innovation Theft", PatentType::DEBUFF},
                {"Agile Defense", PatentType::DEFENSE},
                {"Startup Shield", PatentType::DEFENSE},
                {"Quick Recovery", PatentType::HEAL}
            };
            break;

        case NemesisType::BETA:
            nemesisPatents = {
                {"Unicorn Strike", PatentType::ATTACK},
                {"VC Manipulation", PatentType::DEBUFF},
                {"Funding Cut", PatentType::DEBUFF},
                {"Capital Defense", PatentType::DEFENSE},
                {"Momentum Surge", PatentType::BUFF},
                {"Board Blessing", PatentType::BUFF},
                {"IPO Rush", PatentType::ATTACK}
            };
            break;

        case NemesisType::OMEGA:
            nemesisPatents = {
                {"Market Domination", PatentType::ATTACK},
                {"Monopoly Crush", PatentType::ATTACK},
                {"Hostile Takeover", PatentType::DEBUFF},
                {"Legal Warfare", PatentType::DEBUFF},
                {"Talent Poaching", PatentType::DEBUFF},
                {"Empire Shield", PatentType::DEFENSE},
                {"Antitrust Evasion", PatentType::BUFF},
                {"Resource Drain", PatentType::HEAL}
            };
            break;

        case NemesisType::PRIME:
            nemesisPatents = {
                {"Meme Stock Surge", PatentType::ATTACK},
                {"Twitter Storm", PatentType::DEBUFF},
                {"Media Manipulation", PatentType::DEBUFF},
                {"Regulator Confusion", PatentType::DEBUFF},
                {"Crypto Pump", PatentType::BUFF},
                {"Fanboy Army", PatentType::BUFF},
                {"Reality Distortion", PatentType::BUFF},
                {"SEC Evasion", PatentType::DEFENSE},
                {"Cult of Personality", PatentType::HEAL}
            };
            break;
    }

    // Ensure we have diverse patents
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(nemesisPatents.begin(), nemesisPatents.end(),g);

    // Select patents ensuring variety
    std::map<PatentType, int> typeCounts;
    std::vector<std::pair<std::string, PatentType>> selected;

    for (const auto& [name, type] : nemesisPatents) {
        if (selected.size() >= count) break;

        // Ensure no more than 40% of patents are the same type
        if (typeCounts[type] < (count * 0.4) + 1) {
            selected.push_back({name, type});
            typeCounts[type]++;
        }
    }

    // Fill remaining slots if needed
    if (selected.size() < count) {
        for (const auto& [name, type] : nemesisPatents) {
            if (selected.size() >= count) break;

            bool alreadySelected = false;
            for (const auto& [selName, _] : selected) {
                if (selName == name) {
                    alreadySelected = true;
                    break;
                }
            }

            if (!alreadySelected) {
                selected.push_back({name, type});
            }
        }
    }

    // Create the patents with power scaling
    for (const auto& [name, type] : selected) {
        int basePower = 35 + (playerSeries * 5) + (nemesisPowerLevel_ * 15);
        int cost = 15 + (playerSeries * 2);

        // Debuffs and attacks are extra powerful for Nemesis
        if (type == PatentType::DEBUFF) {
            basePower = static_cast<int>(basePower * 1.5); // 50% stronger
        } else if (type == PatentType::ATTACK) {
            basePower = static_cast<int>(basePower * 1.3); // 30% stronger
        }

        PatentTarget target = (type == PatentType::ATTACK || type == PatentType::DEBUFF)
            ? PatentTarget::ENEMY
            : PatentTarget::SELF;

        auto patent = std::make_shared<Patent>(name, 0, type, basePower, cost, target);
        nemesis.addPatent(patent);
    }
}

