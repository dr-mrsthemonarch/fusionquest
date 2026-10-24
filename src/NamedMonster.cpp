//
// Created by drmrsthemonarch on 23.11.25.
//

#include "../include/Tuning.h"
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
    monster.moneyReward = static_cast<int>(T("named.money_base") + playerSeries * T("named.money_per"));
    monster.itemReward = "NAMED_EQUIPMENT";

    // Hype energy for patents
    monster.maxHypeEnergy = static_cast<int>(T("named.hype_base") + playerSeries * T("named.hype_per"));
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
    int scaledPower = static_cast<int>(patentDef.basePower + playerSeries * T("named.patent_pow_per"));

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
    double growthRate = baseStat * T("named.growth_per_series");
    return static_cast<int>((baseStat + (playerSeries * growthRate)) * T("named.power_mult"));
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
    minion.maxHealth = scaleStatToSeries(static_cast<int>(T("named.minion_hp")), playerSeries);
    minion.health = minion.maxHealth;
    minion.attack = scaleStatToSeries(static_cast<int>(T("named.minion_atk")), playerSeries);
    minion.defense = scaleStatToSeries(static_cast<int>(T("named.minion_def")), playerSeries);
    minion.speed = scaleStatToSeries(static_cast<int>(T("named.minion_spd")), playerSeries);

    minion.moneyReward = 100 + (playerSeries * 15);
    minion.itemReward = "EQUIPMENT";
    minion.itemValue = 40 + playerSeries * 8;

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
