// SPG Engine made by drmrsthemonarch
#include "../include/Tuning.h"
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

float EquipmentGenerator::calculateBaseStat(int playerSeries, double statMultiplier) const {
    // Base formula: (playerSeries * 2 + 5) * statMultiplier
    // statMultiplier: 1 = +, 2 = ++
    float baseStat = (playerSeries * T("equip.per_series") + T("equip.base")) * statMultiplier;

    // Add some randomness (±20%)
    const float v = static_cast<float>(T("equip.variance"));
    std::uniform_real_distribution<float> variance(1.0f - v, 1.0f + v);
    return static_cast<int>(baseStat * variance(gen_));
}

int EquipmentGenerator::calculateInventoryLimit(int playerSeries) const {
    int baseLimit = static_cast<int>(T("equip.backpack_base") + playerSeries * T("equip.backpack_per"));
    const int v = std::max(0, static_cast<int>(T("equip.backpack_var")));
    std::uniform_int_distribution<int> variance(-v, v);
    return std::max(static_cast<int>(T("equip.backpack_min")), baseLimit + variance(gen_));
}

void EquipmentGenerator::applyLaptopStats(Equipment& eq, int s) {
    eq.hustle = calculateBaseStat(s, T("equip.laptop_hustle"));
    eq.technicalDebt = calculateBaseStat(s, T("equip.laptop_td"));
    eq.ndas = calculateBaseStat(s, T("equip.laptop_ndas"));
}
void EquipmentGenerator::applyBikeStats(Equipment& eq, int s) { eq.agility = calculateBaseStat(s, T("equip.bike_agility")); }
void EquipmentGenerator::applyPhoneStats(Equipment& eq, int s) { eq.synergy = calculateBaseStat(s, T("equip.phone_synergy")); }
void EquipmentGenerator::applyHeadphonesStats(Equipment& eq, int s) { eq.technicalDebt = calculateBaseStat(s, T("equip.headphones_td")); }
void EquipmentGenerator::applyJacketStats(Equipment& eq, int s) { eq.luck = calculateBaseStat(s, T("equip.jacket_luck")); }
void EquipmentGenerator::applyBackpackStats(Equipment& eq, int s) { eq.inventoryLimit = calculateInventoryLimit(s); }
void EquipmentGenerator::applyGlovesStats(Equipment& eq, int s) {
    eq.synergy = calculateBaseStat(s, T("equip.gloves_each"));
    eq.hustle = calculateBaseStat(s, T("equip.gloves_each"));
    eq.agility = calculateBaseStat(s, T("equip.gloves_each"));
}
void EquipmentGenerator::applyHelmetStats(Equipment& eq, int s) { eq.ndas = calculateBaseStat(s, T("equip.helmet_ndas")); }

EquipmentRarity EquipmentGenerator::rollBossDropRarity() const {
    std::uniform_int_distribution<int> rarityRoll(1, 10000);
    int roll = rarityRoll(gen_);
    if (roll <= T("rarity.cum_legendary")) return EquipmentRarity::LEGENDARY;
    if (roll <= T("rarity.cum_epic")) return EquipmentRarity::EPIC;
    if (roll <= T("rarity.cum_rare")) return EquipmentRarity::RARE;
    if (roll <= T("rarity.cum_uncommon")) return EquipmentRarity::UNCOMMON;
    return EquipmentRarity::COMMON;
}

void EquipmentGenerator::applyRarityBonus(Equipment& equipment, EquipmentRarity rarity) const {
    double m = 1.0;
    switch (rarity) {
        case EquipmentRarity::UNCOMMON: m = T("rarity.stat_uncommon"); break;
        case EquipmentRarity::RARE: m = T("rarity.stat_rare"); break;
        case EquipmentRarity::EPIC: m = T("rarity.stat_epic"); break;
        case EquipmentRarity::LEGENDARY: m = T("rarity.stat_legendary"); break;
        default: break;
    }
    auto sc = [m](int& v) { if (v > 0) v = static_cast<int>(v * m); };
    sc(equipment.runway); sc(equipment.hypeEnergy); sc(equipment.hustle); sc(equipment.agility);
    sc(equipment.ndas); sc(equipment.luck); sc(equipment.synergy); sc(equipment.technicalDebt);
    sc(equipment.inventoryLimit);
    if (equipment.busFactor > 0.0f) equipment.busFactor = static_cast<float>(equipment.busFactor * m);
}

double EquipmentGenerator::calculateEquipmentValue(const Equipment& q) const {
    double v = 0.0;
    v += q.runway * T("equip.val_runway");
    v += q.hypeEnergy * T("equip.val_hype");
    v += q.hustle * T("equip.val_hustle");
    v += q.agility * T("equip.val_agility");
    v += q.ndas * T("equip.val_ndas");
    v += q.luck * T("equip.val_luck");
    v += q.synergy * T("equip.val_synergy");
    v += q.technicalDebt * T("equip.val_td");
    v += q.busFactor * T("equip.val_bus");
    v += q.inventoryLimit * T("equip.val_inv");
    double rm = 1.0;
    switch (q.rarity) {
        case EquipmentRarity::UNCOMMON: rm = T("rarity.val_uncommon"); break;
        case EquipmentRarity::RARE: rm = T("rarity.val_rare"); break;
        case EquipmentRarity::EPIC: rm = T("rarity.val_epic"); break;
        case EquipmentRarity::LEGENDARY: rm = T("rarity.val_legendary"); break;
        default: break;
    }
    return std::max(T("equip.val_min"), v * rm);
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