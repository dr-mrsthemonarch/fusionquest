//
// Created by drmrsthemonarch on 23.11.25.
//
#include "../include/ConfigStore.h"
#include "../include/NamedMonsterLibrary.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

NamedMonsterLibrary::NamedMonsterLibrary() {}

bool NamedMonsterLibrary::loadFromFile(const std::string& filename) {
    ConfigFile file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open named monsters config: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Start of monster block
        if (line == "[MONSTER]") {
            NamedMonsterDef monster;
            if (parseMonsterBlock(file, monster)) {
                monsters_.push_back(monster);
                std::cout << "Loaded named monster: " << monster.name
                          << " (Day " << monster.encounterDay << ")" << std::endl;
            }
        }
    }

    std::cout << "Loaded " << monsters_.size() << " named monsters" << std::endl;
    return !monsters_.empty();
}

bool NamedMonsterLibrary::parseMonsterBlock(std::istream& file, NamedMonsterDef& monster) {
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#') continue;
        if (line == "[END]") return true;

        if (line == "[PATENT]") {
            PatentDefinition patent;
            if (parsePatentBlock(file, patent)) {
                monster.patents.push_back(patent);
            }
            continue;
        }

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        if (key == "name") monster.name = value;
        else if (key == "encounter_day") monster.encounterDay = std::stoi(value);
        else if (key == "base_health") monster.baseHealth = std::stoi(value);
        else if (key == "base_attack") monster.baseAttack = std::stoi(value);
        else if (key == "base_defense") monster.baseDefense = std::stoi(value);
        else if (key == "base_speed") monster.baseSpeed = std::stoi(value);
        else if (key == "strategy") monster.strategyType = value;
        else if (key == "equipment_type") monster.equipmentType = stringToEquipmentType(value);
        else if (key == "equipment_rarity") monster.equipmentRarity = stringToEquipmentRarity(value);
        else if (key == "minion_count") monster.minionCount = std::stoi(value);  // NEW
        else if (key == "minion_name") monster.minionNames.push_back(value);     // NEW
    }

    return false;
}

bool NamedMonsterLibrary::parsePatentBlock(std::istream& file, PatentDefinition& patent) {
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#') continue;

        // End of patent block (start of monster block or end marker)
        if (line == "[PATENT]" || line == "[END]") {
            // Put line back for parent parser
            file.seekg(-static_cast<int>(line.length()) - 1, std::ios::cur);
            return true;
        }

        // Parse key=value
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        // Parse patent attributes
        if (key == "name") patent.name = value;
        else if (key == "type") patent.type = stringToPatentType(value);
        else if (key == "power") patent.basePower = std::stoi(value);
        else if (key == "cost") patent.cost = std::stoi(value);
        else if (key == "target") patent.target = stringToPatentTarget(value);
        else if (key == "hustle_mod") patent.hustleModifier = std::stoi(value);
        else if (key == "agility_mod") patent.agilityModifier = std::stoi(value);
        else if (key == "ndas_mod") patent.ndasModifier = std::stoi(value);
        else if (key == "luck_mod") patent.luckModifier = std::stoi(value);
        else if (key == "synergy_mod") patent.synergyModifier = std::stoi(value);
        else if (key == "technical_debt_mod") patent.technicalDebtModifier = std::stoi(value);
        else if (key == "duration") patent.duration = std::stoi(value);
    }

    return false;
}

NamedMonsterDef* NamedMonsterLibrary::checkForEncounter(double elapsedSeconds, int playerSeries) {
    // If already blocking, return that monster
    if (blockingMonster_) {
        return blockingMonster_;
    }

    // Convert elapsed seconds to days
    const double SECONDS_PER_DAY = 24 * 60 * 60;
    int currentDay = static_cast<int>(elapsedSeconds / SECONDS_PER_DAY);

    // Check if any undefeated monster should appear
    for (auto& monster : monsters_) {
        if (!monster.defeated && currentDay >= monster.encounterDay) {
            blockingMonster_ = &monster;
            std::cout << "\n🚨 NAMED MONSTER ENCOUNTER! 🚨" << std::endl;
            std::cout << "📜 " << monster.name << " blocks your path!" << std::endl;
            return blockingMonster_;
        }
    }

    return nullptr;
}

void NamedMonsterLibrary::recordDefeat(const std::string& monsterName) {
    for (auto& monster : monsters_) {
        if (monster.name == monsterName) {
            monster.defeated = true;
            blockingMonster_ = nullptr;
            std::cout << "✅ " << monsterName << " defeated permanently!" << std::endl;
            break;
        }
    }
}

// Helper functions
std::string NamedMonsterLibrary::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

PatentType NamedMonsterLibrary::stringToPatentType(const std::string& str) {
    if (str == "ATTACK") return PatentType::ATTACK;
    if (str == "HEAL") return PatentType::HEAL;
    if (str == "DEFENSE") return PatentType::DEFENSE;
    if (str == "BUFF") return PatentType::BUFF;
    if (str == "DEBUFF") return PatentType::DEBUFF;
    return PatentType::ATTACK; // Default
}

PatentTarget NamedMonsterLibrary::stringToPatentTarget(const std::string& str) {
    if (str == "SELF") return PatentTarget::SELF;
    if (str == "ENEMY") return PatentTarget::ENEMY;
    if (str == "ALLY") return PatentTarget::ALLY;
    return PatentTarget::ENEMY; // Default
}

EquipmentType NamedMonsterLibrary::stringToEquipmentType(const std::string& str) {
    if (str == "LAPTOP") return EquipmentType::LAPTOP;
    if (str == "PHONE") return EquipmentType::PHONE;
    if (str == "BACKPACK") return EquipmentType::BACKPACK;
    if (str == "HELMET") return EquipmentType::HELMET;
    if (str == "BIKE") return EquipmentType::BIKE;
    if (str == "JACKET") return EquipmentType::JACKET;
    if (str == "GLOVES") return EquipmentType::GLOVES;
    if (str == "HEADPHONES") return EquipmentType::HEADPHONES;
    return EquipmentType::LAPTOP; // Default
}

EquipmentRarity NamedMonsterLibrary::stringToEquipmentRarity(const std::string& str) {
    if (str == "COMMON") return EquipmentRarity::COMMON;
    if (str == "UNCOMMON") return EquipmentRarity::UNCOMMON;
    if (str == "RARE") return EquipmentRarity::RARE;
    if (str == "EPIC") return EquipmentRarity::EPIC;
    if (str == "LEGENDARY") return EquipmentRarity::LEGENDARY;
    return EquipmentRarity::RARE; // Default
}

