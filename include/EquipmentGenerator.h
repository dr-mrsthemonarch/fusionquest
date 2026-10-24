
#ifndef EQUIPMENTGENERATOR_H
#define EQUIPMENTGENERATOR_H

#include <vector>
#include <string>
#include <random>
#include "Equipment.h"

class EquipmentGenerator {
public:
    EquipmentGenerator();

    bool loadWordLists(const std::string& configPath);
    Equipment generateRandomEquipment();
    Equipment generateSpecificEquipment(EquipmentType type);
    Equipment generateSpecificEquipmentForPlayer(EquipmentType type, int playerSeries);
    Equipment generateBossDropEquipment(int playerSeries); // Special boss drops with enhanced rarity
  std::string equipmentTypeToString(EquipmentType type) const;

private:
    // Word lists for generating equipment names
    std::vector<std::string> adjectives_;
    std::vector<std::string> materials_;
    std::vector<std::string> suffixes_;

    mutable std::mt19937 gen_;

    // Base stat calculation methods
    float calculateBaseStat(int playerSeries, double statMultiplier) const;
    int calculateInventoryLimit(int playerSeries) const;
    double calculateEquipmentValue(const Equipment& equipment) const;

    // Equipment-specific stat application
    void applyLaptopStats(Equipment& equipment, int playerSeries);
    void applyBikeStats(Equipment& equipment, int playerSeries);
    void applyPhoneStats(Equipment& equipment, int playerSeries);
    void applyHeadphonesStats(Equipment& equipment, int playerSeries);
    void applyJacketStats(Equipment& equipment, int playerSeries);
    void applyBackpackStats(Equipment& equipment, int playerSeries);
    void applyGlovesStats(Equipment& equipment, int playerSeries);
    void applyHelmetStats(Equipment& equipment, int playerSeries);

    // Rarity system for boss drops
    EquipmentRarity rollBossDropRarity() const;
    void applyRarityBonus(Equipment& equipment, EquipmentRarity rarity) const;

    std::string generateEquipmentName(EquipmentType type);
    std::string getRandomAdjective();
    std::string getRandomMaterial();
    std::string getRandomSuffix();
};

#endif // EQUIPMENTGENERATOR_H