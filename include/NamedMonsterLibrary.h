//
// Created by drmrsthemonarch on 23.11.25.
//

#ifndef NAMEDMONSTERLIBRARY_H
#define NAMEDMONSTERLIBRARY_H

#include "NamedMonster.h"
#include <vector>
#include <memory>



class NamedMonsterLibrary {
public:
    NamedMonsterLibrary();

    // Load named monsters from config file
    bool loadFromFile(const std::string& filename);

    // Check if any named monster should appear at current time
    NamedMonsterDef* checkForEncounter(double elapsedSeconds, int playerSeries);

    // Mark monster as defeated
    void recordDefeat(const std::string& monsterName);

    // Check if blocking encounter is active
    bool hasBlockingEncounter() const { return blockingMonster_ != nullptr; }

    // Get current blocking monster
    NamedMonsterDef* getBlockingMonster() { return blockingMonster_; }

    // Clear blocking state
    void clearBlockingEncounter() { blockingMonster_ = nullptr; }

    // Get all definitions
    const std::vector<NamedMonsterDef>& getMonsters() const { return monsters_; }

    NamedMonsterSystem& getMonsterSystem() { return monsterSystem_; }

private:
    std::vector<NamedMonsterDef> monsters_;
    NamedMonsterDef* blockingMonster_ = nullptr;

    NamedMonsterSystem monsterSystem_;

    // Parsing helpers
    bool parseMonsterBlock(std::ifstream& file, NamedMonsterDef& monster);
    bool parsePatentBlock(std::ifstream& file, PatentDefinition& patent);

    std::string trim(const std::string& str);
    PatentType stringToPatentType(const std::string& str);
    PatentTarget stringToPatentTarget(const std::string& str);
    EquipmentType stringToEquipmentType(const std::string& str);
    EquipmentRarity stringToEquipmentRarity(const std::string& str);
};

#endif // NAMEDMONSTERLIBRARY_H
