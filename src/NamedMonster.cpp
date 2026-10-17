//
// Created by drmrsthemonarch on 23.11.25.
//

#include "../include/NamedMonster.h"
#include "../include/PatentStrategy.h"
#include <algorithm>
#include <iostream>

NamedMonsterSystem::NamedMonsterSystem() {}

Monster NamedMonsterSystem::generateNamedMonster(const NamedMonsterDef& def, int playerSeries) {
    Monster monster;

    monster.name = def.name;
    monster.type = MonsterType::BOSS;
    monster.difficulty = MonsterDifficulty::LEGENDARY;
    monster.level = playerSeries;

    // Scale stats
    monster.maxHealth = scaleStatToSeries(def.baseHealth, playerSeries);
    monster.health = monster.maxHealth;
    monster.attack = scaleStatToSeries(def.baseAttack, playerSeries);
    monster.defense = scaleStatToSeries(def.baseDefense, playerSeries);
    monster.speed = scaleStatToSeries(def.baseSpeed, playerSeries);

    // Rewards (will be overridden by special drop system)
    monster.moneyReward = 500 + (playerSeries * 50);
    monster.itemReward = "NAMED_EQUIPMENT";

    // Hype energy for patents
    monster.maxHypeEnergy = 100 + (playerSeries * 10);
    monster.hypeEnergy = monster.maxHypeEnergy;

    // Add patents
    for (const auto& patentDef : def.patents) {
        auto patent = createNamedPatent(patentDef, playerSeries);
        monster.addPatent(patent);
    }

    // Set strategy
    monster.setStrategy(createStrategy(def.strategyType));

    return monster;
}

std::shared_ptr<Patent> NamedMonsterSystem::createNamedPatent(const PatentDefinition& patentDef, int playerSeries) {
    // Scale power with player series
    int scaledPower = patentDef.basePower + (playerSeries * 5);

    auto patent = std::make_shared<Patent>(
        patentDef.name,
        0, // No series requirement for monster patents
        patentDef.type,
        scaledPower,
        patentDef.cost,
        patentDef.target
    );

    // Check if this patent has custom stat modifiers
    bool hasCustomMods = patentDef.hustleModifier != 0 ||
                         patentDef.agilityModifier != 0 ||
                         patentDef.ndasModifier != 0 ||
                         patentDef.luckModifier != 0 ||
                         patentDef.synergyModifier != 0 ||
                         patentDef.technicalDebtModifier != 0;

    if (hasCustomMods) {
        patent->setCustomModifiers(
            patentDef.hustleModifier,
            patentDef.agilityModifier,
            patentDef.ndasModifier,
            patentDef.luckModifier,
            patentDef.synergyModifier,
            patentDef.technicalDebtModifier,
            patentDef.duration
        );
    }

    return patent;
}

int NamedMonsterSystem::scaleStatToSeries(int baseStat, int playerSeries) const {
    // Linear scaling: baseStat + (playerSeries * growthRate)
    // Growth rate is 10% of base per series
    double growthRate = baseStat * 0.1;
    return static_cast<int>(baseStat + (playerSeries * growthRate));
}

std::unique_ptr<PatentStrategy> NamedMonsterSystem::createStrategy(const std::string& strategyType) {
    if (strategyType == "AGGRESSIVE") {
        return std::make_unique<AggressiveStrategy>();
    } else if (strategyType == "BALANCED") {
        return std::make_unique<BalancedStrategy>();
    } else if (strategyType == "DEBUFF") {
        return std::make_unique<DebuffStrategy>();
    } else if (strategyType == "TACTICAL") {
        return std::make_unique<TacticalStrategy>();
    } else if (strategyType == "MIRROR") {
        return std::make_unique<MirrorStrategy>();
    } else if (strategyType == "DEFENSIVE") {
        return std::make_unique<DefensiveStrategy>();
    } else {
        // Default to balanced
        return std::make_unique<BalancedStrategy>();
    }
}

Monster NamedMonsterSystem::generateNamedMinion(
    const std::string& minionName, int playerSeries, int index
) {
    Monster minion;
    minion.name = minionName + " #" + std::to_string(index);
    minion.type = MonsterType::MOB;
    minion.difficulty = MonsterDifficulty::MEDIUM;
    minion.level = playerSeries;

    // 50% of named monster power
    minion.maxHealth = scaleStatToSeries(80, playerSeries);
    minion.health = minion.maxHealth;
    minion.attack = scaleStatToSeries(30, playerSeries);
    minion.defense = scaleStatToSeries(20, playerSeries);
    minion.speed = scaleStatToSeries(25, playerSeries);

    minion.moneyReward = 100 + (playerSeries * 15);
    minion.itemReward = "EQUIPMENT";

    minion.maxHypeEnergy = 40 + (playerSeries * 5);
    minion.hypeEnergy = minion.maxHypeEnergy;

    minion.setStrategy(std::make_unique<AggressiveStrategy>());

    return minion;
}

std::vector<Monster> NamedMonsterSystem::generateNamedEncounter(
    const NamedMonsterDef& def, int playerSeries
) {
    std::vector<Monster> encounter;

    // Generate main boss
    encounter.push_back(generateNamedMonster(def, playerSeries));

    // Generate minions if configured
    for (int i = 0; i < def.minionCount && i < def.minionNames.size(); ++i) {
        encounter.push_back(generateNamedMinion(def.minionNames[i], playerSeries, i + 1));
    }

    return encounter;
}
