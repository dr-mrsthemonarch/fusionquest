// SPG Engine made by drmrsthemonarch
#include "../include/ConfigStore.h"
#include "../include/EquipmentGenerator.h"
#include <fstream>
#include <iostream>
#include <algorithm>

EquipmentGenerator::EquipmentGenerator() : gen_(std::random_device{}()) {}

bool EquipmentGenerator::loadWordLists(const std::string& configPath) {
    // Load adjectives
    ConfigFile adjectivesFile(configPath + "adjectives.txt");
    if (adjectivesFile.is_open()) {
        std::string line;
        while (std::getline(adjectivesFile, line)) {
            if (!line.empty()) {
                adjectives_.push_back(line);
            }
        }
        adjectivesFile.close();
    }

    // Load materials
    ConfigFile materialsFile(configPath + "materials.txt");
    if (materialsFile.is_open()) {
        std::string line;
        while (std::getline(materialsFile, line)) {
            if (!line.empty()) {
                materials_.push_back(line);
            }
        }
        materialsFile.close();
    }

    // Load suffixes
    ConfigFile suffixesFile(configPath + "suffixes.txt");
    if (suffixesFile.is_open()) {
        std::string line;
        while (std::getline(suffixesFile, line)) {
            if (!line.empty()) {
                suffixes_.push_back(line);
            }
        }
        suffixesFile.close();
    }

    // Add default values if files weren't loaded
    if (adjectives_.empty()) {
        adjectives_ = {"Advanced", "Quantum", "Digital", "Smart", "Pro", "Elite", "Premium", "Tactical"};
    }
    if (materials_.empty()) {
        materials_ = {"Carbon", "Titanium", "Silicon", "Polymer", "Graphene", "Aluminum", "Steel"};
    }
    if (suffixes_.empty()) {
        suffixes_ = {"X1", "Pro", "Max", "Ultra", "Elite", "Prime", "Core", "Edge"};
    }

    return !adjectives_.empty() && !materials_.empty() && !suffixes_.empty();
}

Equipment EquipmentGenerator::generateRandomEquipment() {
    // Generate random equipment type
    std::uniform_int_distribution<int> typeDist(0, static_cast<int>(EquipmentType::HELMET));
    EquipmentType randomType = static_cast<EquipmentType>(typeDist(gen_));

    // Use series 1 as default for random generation
    return generateSpecificEquipmentForPlayer(randomType, 1);
}

Equipment EquipmentGenerator::generateSpecificEquipment(EquipmentType type) {
    // Use series 1 as default when player series not specified
    return generateSpecificEquipmentForPlayer(type, 1);
}

Equipment EquipmentGenerator::generateSpecificEquipmentForPlayer(EquipmentType type, int playerSeries) {
    Equipment equipment;
    equipment.type = type;
    equipment.name = generateEquipmentName(type);
    equipment.rarity = EquipmentRarity::COMMON; // Default rarity for regular equipment

    // Reset all stats to 0
    equipment.runway = 0;
    equipment.hypeEnergy = 0;
    equipment.hustle = 0;
    equipment.agility = 0;
    equipment.ndas = 0;
    equipment.luck = 0;
    equipment.synergy = 0;
    equipment.technicalDebt = 0;
    equipment.busFactor = 0.0f;
    equipment.inventoryLimit = 0;

    // Apply equipment-specific stats based on player series
    switch (type) {
        case EquipmentType::LAPTOP:
            applyLaptopStats(equipment, playerSeries);
            break;
        case EquipmentType::BIKE:
            applyBikeStats(equipment, playerSeries);
            break;
        case EquipmentType::PHONE:
            applyPhoneStats(equipment, playerSeries);
            break;
        case EquipmentType::HEADPHONES:
            applyHeadphonesStats(equipment, playerSeries);
            break;
        case EquipmentType::JACKET:
            applyJacketStats(equipment, playerSeries);
            break;
        case EquipmentType::BACKPACK:
            applyBackpackStats(equipment, playerSeries);
            break;
        case EquipmentType::GLOVES:
            applyGlovesStats(equipment, playerSeries);
            break;
        case EquipmentType::HELMET:
            applyHelmetStats(equipment, playerSeries);
            break;
    }

    // Calculate equipment value based on stats
    equipment.equipmentvalue = calculateEquipmentValue(equipment);

    return equipment;
}

Equipment EquipmentGenerator::generateBossDropEquipment(int playerSeries) {
    // Generate random equipment type
    std::uniform_int_distribution<int> typeDist(0, static_cast<int>(EquipmentType::HELMET));
    EquipmentType randomType = static_cast<EquipmentType>(typeDist(gen_));

    // Generate base equipment
    Equipment equipment = generateSpecificEquipmentForPlayer(randomType, playerSeries);

    // Roll for enhanced rarity (boss drops have better rarity chances)
    equipment.rarity = rollBossDropRarity();

    // Apply rarity bonus
    applyRarityBonus(equipment, equipment.rarity);

    // Update name to reflect rarity
    std::string rarityPrefix;
    switch (equipment.rarity) {
        case EquipmentRarity::UNCOMMON: rarityPrefix = "Uncommon"; break;
        case EquipmentRarity::RARE: rarityPrefix = "Rare"; break;
        case EquipmentRarity::EPIC: rarityPrefix = "Epic"; break;
        case EquipmentRarity::LEGENDARY: rarityPrefix = "Legendary"; break;
        default: break;
    }
    equipment.name = rarityPrefix + equipment.name;

    // Recalculate value with rarity bonus
    equipment.equipmentvalue = calculateEquipmentValue(equipment);

    return equipment;
}

float EquipmentGenerator::calculateBaseStat(int playerSeries, int statMultiplier) const {
    // Base formula: (playerSeries * 2 + 5) * statMultiplier
    // statMultiplier: 1 = +, 2 = ++
    float baseStat = (playerSeries * 1.2 + 5) * statMultiplier;

    // Add some randomness (±20%)
    std::uniform_real_distribution<float> variance(0.8f, 1.2f);
    return static_cast<int>(baseStat * variance(gen_));
}

int EquipmentGenerator::calculateInventoryLimit(int playerSeries) const {
    // Backpack inventory limit formula: 10 + (playerSeries * 5)
    int baseLimit = 10 + (playerSeries * 5);

    // Add some randomness (±2 slots)
    std::uniform_int_distribution<int> variance(-2, 2);
    return std::max(5, baseLimit + variance(gen_)); // Minimum 5 slots
}

void EquipmentGenerator::applyLaptopStats(Equipment& equipment, int playerSeries) {
    // Laptops -> Hustle + and Technical debt ++ and NDAs +
    equipment.hustle = calculateBaseStat(playerSeries, 1.1);           // +
    equipment.technicalDebt = calculateBaseStat(playerSeries, 1.8);    // ++
    equipment.ndas = calculateBaseStat(playerSeries, 1.3);             // +
}

void EquipmentGenerator::applyBikeStats(Equipment& equipment, int playerSeries) {
    // Bikes -> agility ++
    equipment.agility = calculateBaseStat(playerSeries, 2);          // ++
}

void EquipmentGenerator::applyPhoneStats(Equipment& equipment, int playerSeries) {
    // Phone -> Synergy ++
    equipment.synergy = calculateBaseStat(playerSeries, 2);          // ++
}

void EquipmentGenerator::applyHeadphonesStats(Equipment& equipment, int playerSeries) {
    // Headphones -> Technical debt +
    equipment.technicalDebt = calculateBaseStat(playerSeries, 1.5);    // +
}

void EquipmentGenerator::applyJacketStats(Equipment& equipment, int playerSeries) {
    // Jacket -> Luck +
    equipment.luck = calculateBaseStat(playerSeries, 2);             // +
}

void EquipmentGenerator::applyBackpackStats(Equipment& equipment, int playerSeries) {
    // Backpack -> Is the inventory limit of the character
    equipment.inventoryLimit = calculateInventoryLimit(playerSeries);
}

void EquipmentGenerator::applyGlovesStats(Equipment& equipment, int playerSeries) {
    // Gloves -> Synergy ++ and Hustle ++ and Agility ++
    equipment.synergy = calculateBaseStat(playerSeries, 2);          // ++
    equipment.hustle = calculateBaseStat(playerSeries, 2);           // ++
    equipment.agility = calculateBaseStat(playerSeries, 2);          // ++
}

void EquipmentGenerator::applyHelmetStats(Equipment& equipment, int playerSeries) {
    // Helmet -> NDAs ++
    equipment.ndas = calculateBaseStat(playerSeries, 2);             // ++
}

EquipmentRarity EquipmentGenerator::rollBossDropRarity() const {
    std::uniform_int_distribution<int> rarityRoll(1, 10000);
    int roll = rarityRoll(gen_);

    // Boss drops have enhanced rarity chances
    if (roll <= 10) return EquipmentRarity::LEGENDARY;      // 0.001% chance
    else if (roll <= 1500) return EquipmentRarity::EPIC;     // 15% chance
    else if (roll <= 3000) return EquipmentRarity::RARE;     // 30% chance
    else if (roll <= 8000) return EquipmentRarity::UNCOMMON; // 30% chance
    else return EquipmentRarity::COMMON;                   // 20% chance
}

void EquipmentGenerator::applyRarityBonus(Equipment& equipment, EquipmentRarity rarity) const {
    float multiplier = 1.0f;

    switch (rarity) {
        case EquipmentRarity::UNCOMMON: multiplier = 1.2f; break;
        case EquipmentRarity::RARE: multiplier = 1.5f; break;
        case EquipmentRarity::EPIC: multiplier = 2.0f; break;
        case EquipmentRarity::LEGENDARY: multiplier = 5.0f; break;
        default: multiplier = 1.0f; break;
    }

    // Apply multiplier to all non-zero stats
    if (equipment.runway > 0) equipment.runway = static_cast<int>(equipment.runway * multiplier);
    if (equipment.hypeEnergy > 0) equipment.hypeEnergy = static_cast<int>(equipment.hypeEnergy * multiplier);
    if (equipment.hustle > 0) equipment.hustle = static_cast<int>(equipment.hustle * multiplier);
    if (equipment.agility > 0) equipment.agility = static_cast<int>(equipment.agility * multiplier);
    if (equipment.ndas > 0) equipment.ndas = static_cast<int>(equipment.ndas * multiplier);
    if (equipment.luck > 0) equipment.luck = static_cast<int>(equipment.luck * multiplier);
    if (equipment.synergy > 0) equipment.synergy = static_cast<int>(equipment.synergy * multiplier);
    if (equipment.technicalDebt > 0) equipment.technicalDebt = static_cast<int>(equipment.technicalDebt * multiplier);
    if (equipment.inventoryLimit > 0) equipment.inventoryLimit = static_cast<int>(equipment.inventoryLimit * multiplier);

    // Bus factor gets special treatment
    if (equipment.busFactor > 0.0f) equipment.busFactor *= multiplier;
}

double EquipmentGenerator::calculateEquipmentValue(const Equipment& equipment) const {
    // Base value calculation based on total stats
    double totalValue = 0.0;

    // Weight different stats differently
    totalValue += equipment.runway * 10.0;        // High value for survival
    totalValue += equipment.hypeEnergy * 8.0;     // Energy is valuable
    totalValue += equipment.hustle * 12.0;        // Attack power premium
    totalValue += equipment.agility * 12.0;       // Speed premium
    totalValue += equipment.ndas * 6.0;           // Defense Moderate utility
    totalValue += equipment.luck * 8.0;           // Good Luck
    totalValue += equipment.synergy * 8.0;        // Good Charisma
    totalValue += equipment.technicalDebt * 3.0;  //
    totalValue += equipment.busFactor * 1.0;      // High value multiplier
    totalValue += equipment.inventoryLimit * 25.0; // Inventory slots are valuable

    // Apply rarity multiplier to value
    float rarityMultiplier = 1.0f;
    switch (equipment.rarity) {
        case EquipmentRarity::UNCOMMON: rarityMultiplier = 1.5f; break;
        case EquipmentRarity::RARE: rarityMultiplier = 2.5f; break;
        case EquipmentRarity::EPIC: rarityMultiplier = 4.0f; break;
        case EquipmentRarity::LEGENDARY: rarityMultiplier = 7.0f; break;
        default: rarityMultiplier = 1.0f; break;
    }

    totalValue *= rarityMultiplier;

    // Minimum value of 50
    return std::max(50.0, totalValue);
}

// Name generation methods
std::string EquipmentGenerator::generateEquipmentName(EquipmentType type) {
    std::string baseName = equipmentTypeToString(type);

    // 70% chance to add adjective, 50% chance to add material, 30% chance to add suffix
    std::string name = baseName;

    std::uniform_int_distribution<int> chance(0, 100);

    if (chance(gen_) < 70 && !adjectives_.empty()) {
        name = getRandomAdjective() + " " + name;
    }

    if (chance(gen_) < 50 && !materials_.empty()) {
        name = getRandomMaterial() + " " + name;
    }

    if (chance(gen_) < 30 && !suffixes_.empty()) {
        name = name + " " + getRandomSuffix();
    }

    return name;
}

std::string EquipmentGenerator::getRandomAdjective() {
    if (adjectives_.empty()) return "";
    std::uniform_int_distribution<size_t> dist(0, adjectives_.size() - 1);
    return adjectives_[dist(gen_)];
}

std::string EquipmentGenerator::getRandomMaterial() {
    if (materials_.empty()) return "";
    std::uniform_int_distribution<size_t> dist(0, materials_.size() - 1);
    return materials_[dist(gen_)];
}

std::string EquipmentGenerator::getRandomSuffix() {
    if (suffixes_.empty()) return "";
    std::uniform_int_distribution<size_t> dist(0, suffixes_.size() - 1);
    return suffixes_[dist(gen_)];
}

std::string EquipmentGenerator::equipmentTypeToString(EquipmentType type) const {
    switch (type) {
        case EquipmentType::LAPTOP: return "Laptop";
        case EquipmentType::BIKE: return "Bike";
        case EquipmentType::PHONE: return "Phone";
        case EquipmentType::HEADPHONES: return "Headphones";
        case EquipmentType::JACKET: return "Jacket";
        case EquipmentType::BACKPACK: return "Backpack";
        case EquipmentType::GLOVES: return "Gloves";
        case EquipmentType::HELMET: return "Helmet";
        default: return "Unknown";
    }
}