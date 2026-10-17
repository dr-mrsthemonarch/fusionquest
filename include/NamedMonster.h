//
// Created by drmrsthemonarch on 23.11.25.
//

#ifndef NAMEDMONSTER_H
#define NAMEDMONSTER_H

#include <string>
#include <vector>
#include <memory>
#include "Monster.h"
#include "Patent.h"
#include "Equipment.h"

// Forward declarations
class PatentStrategy;

// Named patent definition - extends regular patents with stat modifiers
struct NamedPatentDef {
    std::string name;
    PatentType type;
    int basePower;
    int cost;
    PatentTarget target;

    // Stat modifiers (applied as buffs/debuffs)
    int hustleModifier = 0;
    int agilityModifier = 0;
    int ndasModifier = 0;
    int luckModifier = 0;
    int synergyModifier = 0;
    int technicalDebtModifier = 0;
    int duration = 3; // Default 3 turns
};

// Named monster definition
struct NamedMonsterDef {
    std::string name;
    int encounterDay;

    int baseHealth;
    int baseAttack;
    int baseDefense;
    int baseSpeed;

    std::string strategyType;
    std::vector<PatentDefinition> patents;

    EquipmentType equipmentType;
    EquipmentRarity equipmentRarity;

    bool defeated = false;

    // NEW: Minion configuration
    int minionCount = 0;
    std::vector<std::string> minionNames;
};

// Monster generator for named monsters
class NamedMonsterSystem {
public:
    NamedMonsterSystem();

    // Generate monster instance from definition, scaled to player level
    Monster generateNamedMonster(const NamedMonsterDef& def, int playerSeries);

    // Create named patent as Patent object
    std::shared_ptr<Patent> createNamedPatent(const PatentDefinition& patentDef, int playerSeries);


    std::vector<Monster> generateNamedEncounter(const NamedMonsterDef &def, int playerSeries);
private:
    // Scale stats linearly with player series
    int scaleStatToSeries(int baseStat, int playerSeries) const;

    // Convert string to strategy
    std::unique_ptr<PatentStrategy> createStrategy(const std::string& strategyType);

    Monster generateNamedMinion(const std::string &minionName, int playerSeries, int index);

};

#endif // NAMEDMONSTER_H
