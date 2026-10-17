#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <string>

// Equipment rarity levels - affects stat multipliers and drop rates
enum class EquipmentRarity {
  COMMON,      // 1.0x multiplier - most common
  UNCOMMON,    // 1.2x multiplier
  RARE,        // 1.5x multiplier
  EPIC,        // 2.0x multiplier
  LEGENDARY    // 3.0x multiplier - rarest
};

enum class EquipmentType {
  LAPTOP,
  PHONE,
  BACKPACK,
  HELMET,
  BIKE,
  JACKET,
  GLOVES,
  HEADPHONES
};

struct Equipment {
  std::string name;
  EquipmentType type;
  EquipmentRarity rarity = EquipmentRarity::COMMON;  // NEW: Rarity system
  // int equipmentclass = 1;
  std::string equipmentclass;
  int equipmentvalue = 0;
  int runway = 0;
  int hypeEnergy = 0;
  int hustle = 0;
  int agility = 0;
  int ndas = 0;
  int luck = 0;
  int synergy = 0;
  int technicalDebt = 0;
  float busFactor = 0.0f;
  int inventoryLimit = 0;  // NEW: Used by backpacks to set inventory capacity
};

// Helper function to convert rarity to string
inline std::string rarityToString(EquipmentRarity rarity) {
  switch (rarity) {
    case EquipmentRarity::COMMON: return "Common";
    case EquipmentRarity::UNCOMMON: return "Uncommon";
    case EquipmentRarity::RARE: return "Rare";
    case EquipmentRarity::EPIC: return "Epic";
    case EquipmentRarity::LEGENDARY: return "Legendary";
    default: return "Unknown";
  }
}

// Helper function to get rarity color code (for terminal display)
inline std::string rarityToColor(EquipmentRarity rarity) {
  switch (rarity) {
    case EquipmentRarity::COMMON: return "\033[37m";      // White
    case EquipmentRarity::UNCOMMON: return "\033[32m";    // Green
    case EquipmentRarity::RARE: return "\033[34m";        // Blue
    case EquipmentRarity::EPIC: return "\033[35m";        // Purple
    case EquipmentRarity::LEGENDARY: return "\033[33m";   // Gold
    default: return "\033[0m";                             // Reset
  }
}

#endif // EQUIPMENT_H