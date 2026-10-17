// SPG Engine made by drmrsthemonarch
#include "../include/SpecialDropsLibrary.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

SpecialDropsLibrary::SpecialDropsLibrary() : nextUniqueID_(1) {
}

std::string SpecialDropsLibrary::getDropTypeString(SpecialDropType type) const {
    switch (type) {
        case SpecialDropType::NEMESIS_DROP: return "Nemesis";
        case SpecialDropType::SHADOW_PLAYER_DROP: return "Shadow Player";
        case SpecialDropType::SAFETY_MANAGER_DROP: return "Safety Manager";
        case SpecialDropType::CONFERENCE_DROP: return "Conference";
        case SpecialDropType::PAPER_WRITING_DROP: return "Paper Writing";
        case SpecialDropType::MONTHLY_REVIEW_DROP: return "Monthly Boss";
        case SpecialDropType::NAMED_MONSTER_DROP: return "Named Monster";
        default: return "Unknown";
    }
}

std::string SpecialDropsLibrary::getDropTypeIcon(SpecialDropType type) const {
    switch (type) {
        case SpecialDropType::NEMESIS_DROP: return "⚡";
        case SpecialDropType::SHADOW_PLAYER_DROP: return "👤";
        case SpecialDropType::SAFETY_MANAGER_DROP: return "🛡️";
        case SpecialDropType::CONFERENCE_DROP: return "🎤";
        case SpecialDropType::PAPER_WRITING_DROP: return "📄";
        case SpecialDropType::MONTHLY_REVIEW_DROP: return "📅";
        case SpecialDropType::NAMED_MONSTER_DROP: return "📜";
        default: return "🎁";
    }
}

void SpecialDropsLibrary::addSpecialDrop(const SpecialDrop &drop) {
    SpecialDrop newDrop = drop;
    newDrop.uniqueID = nextUniqueID_++;
    library_.push_back(newDrop);

    std::cout << "\n✨ SPECIAL DROP ACQUIRED! ✨" << std::endl;
    std::cout << getDropTypeIcon(drop.type) << " " << drop.name << std::endl;
    std::cout << "Type: " << getDropTypeString(drop.type) << std::endl;
    std::cout << "Added to permanent library (ID: " << newDrop.uniqueID << ")" << std::endl;
}

Equipment SpecialDropsLibrary::generateNemesisEquipment(int playerSeries, int nemesisPowerLevel, const std::string &nemesisName) {
    Equipment nemesisGear;

    // Determine equipment type based on power level
    EquipmentType types[] = {EquipmentType::LAPTOP, EquipmentType::PHONE, EquipmentType::BACKPACK, EquipmentType::HELMET, EquipmentType::BIKE, EquipmentType::JACKET, EquipmentType::GLOVES, EquipmentType::HEADPHONES};

    nemesisGear.type = types[nemesisPowerLevel % 8];
    nemesisGear.rarity = EquipmentRarity::LEGENDARY;
    nemesisGear.equipmentclass = *"Competitor";

    // Name based on power level
    if (nemesisPowerLevel == 0) {
        nemesisGear.name = "Competitor Memento";
    } else {
        nemesisGear.name = "Competitor Relic [+" + std::to_string(nemesisPowerLevel) + "]";
    }

    // Powerful stats scaling with power level
    double powerMult = 1.0 + (nemesisPowerLevel * 0.5); // +50% per evolution

    nemesisGear.runway = static_cast<int>(30 + playerSeries * 5) * powerMult;
    nemesisGear.hypeEnergy = static_cast<int>(25 + playerSeries * 4) * powerMult;
    nemesisGear.hustle = static_cast<int>(10 + playerSeries * 2) * powerMult;
    nemesisGear.agility = static_cast<int>(8 + playerSeries * 1.5) * powerMult;
    nemesisGear.ndas = static_cast<int>(5 + playerSeries) * powerMult;
    nemesisGear.luck = static_cast<int>(6 + playerSeries * 1.2) * powerMult;
    nemesisGear.synergy = static_cast<int>(7 + playerSeries * 1.5) * powerMult;
    nemesisGear.technicalDebt = -static_cast<int>(3 + playerSeries * 0.5) * powerMult;
    nemesisGear.busFactor = 2.0f * powerMult;
    nemesisGear.equipmentvalue = 10 * powerMult; // Extremely valuable but unsellable

    if (nemesisGear.type == EquipmentType::BACKPACK) {
        nemesisGear.inventoryLimit = 50; // Huge inventory
    }

    return nemesisGear;
}

Equipment SpecialDropsLibrary::generateShadowPlayerEquipment(int playerSeries, const std::string &playerName) {
    Equipment shadowGear;

    shadowGear.type = EquipmentType::JACKET;
    shadowGear.rarity = EquipmentRarity::LEGENDARY;
    shadowGear.equipmentclass = "Shadow";
    shadowGear.name = "Shadow's Reflection";

    // Extremely powerful - once per game item
    shadowGear.runway = 50 + playerSeries * 8;
    shadowGear.hypeEnergy = 40 + playerSeries * 6;
    shadowGear.hustle = 15 + playerSeries * 3;
    shadowGear.agility = 12 + playerSeries * 2.5;
    shadowGear.ndas = 10 + playerSeries * 2;
    shadowGear.luck = 20 + playerSeries * 30;
    shadowGear.synergy = 10 + playerSeries * 2;
    shadowGear.technicalDebt = (5 + playerSeries);
    shadowGear.busFactor = -1e-2f;
    shadowGear.equipmentvalue = 1e9; // Priceless

    return shadowGear;
}

Equipment SpecialDropsLibrary::generateSafetyManagerEquipment(int playerSeries) {
    Equipment safetyGear;

    safetyGear.type = EquipmentType::HELMET;
    safetyGear.rarity = EquipmentRarity::RARE;
    safetyGear.equipmentclass = "Safety";
    safetyGear.name = "Safety Manager's Hardhat";

    // High defense focus
    safetyGear.runway = 20 + playerSeries * 3;
    safetyGear.hypeEnergy = 10 + playerSeries * 2;
    safetyGear.hustle = 3 + playerSeries;
    safetyGear.agility = 12 + playerSeries * 2; // High agility (safety awareness)
    safetyGear.ndas = 15 + playerSeries * 3;
    safetyGear.luck = 2 + playerSeries * 0.5;
    safetyGear.synergy = 5 + playerSeries;
    safetyGear.technicalDebt = (10 + playerSeries * 2); // Reduces technical debt significantly
    safetyGear.busFactor = -1e-41f;
    safetyGear.equipmentvalue = 50000;

    return safetyGear;
}

Equipment SpecialDropsLibrary::generateConferenceEquipment(int playerSeries) {
    Equipment confGear;

    confGear.type = EquipmentType::HEADPHONES;
    confGear.rarity = EquipmentRarity::RARE;
    confGear.equipmentclass = "Conference";
    confGear.name = "Keynote Speaker's Headset";

    // Synergy and luck focus
    confGear.runway = 15 + playerSeries * 2;
    confGear.hypeEnergy = 20 + playerSeries * 3;
    confGear.hustle = 5 + playerSeries;
    confGear.agility = 6 + playerSeries;
    confGear.ndas = 3 + playerSeries * 0.5;
    confGear.luck = 15 + playerSeries * 3; // High luck (networking)
    confGear.synergy = 20 + playerSeries * 4;
    confGear.technicalDebt = 10 + playerSeries * 2;
    confGear.busFactor = -1e-41f;
    confGear.equipmentvalue = 30000;

    return confGear;
}

Equipment SpecialDropsLibrary::generatePaperWritingEquipment(int playerSeries) {
    Equipment paperGear;

    paperGear.type = EquipmentType::LAPTOP;
    paperGear.rarity = EquipmentRarity::RARE;
    paperGear.equipmentclass = "Academic";
    paperGear.name = "Researcher's Workstation";

    // Agility and NDAs focus
    paperGear.runway = 18 + playerSeries * 2.5;
    paperGear.hypeEnergy = 15 + playerSeries * 2;
    paperGear.hustle = 8 + playerSeries * 1.5;
    paperGear.agility = 15 + playerSeries * 3; // High agility (writing skill)
    paperGear.ndas = 1 + playerSeries * 1.1; // High NDAs (citations/references)
    paperGear.luck = 3 + playerSeries * 0.5;
    paperGear.synergy = 8 + playerSeries * 1.5;
    paperGear.technicalDebt = 5 + playerSeries*3; // Papers create tech debt
    paperGear.busFactor = -1e-41f;
    paperGear.equipmentvalue = 40000;

    return paperGear;
}

Equipment SpecialDropsLibrary::generateMonthlyReviewEquipment(int year, int month, int playerSeries) {
    Equipment reviewGear;

    // Cycle through equipment types
    EquipmentType types[] = {EquipmentType::LAPTOP, EquipmentType::PHONE, EquipmentType::BACKPACK, EquipmentType::HELMET, EquipmentType::BIKE, EquipmentType::JACKET, EquipmentType::GLOVES, EquipmentType::HEADPHONES};
    reviewGear.type = types[month % 8];

    reviewGear.rarity = EquipmentRarity::EPIC;
    reviewGear.equipmentclass = "Monthly";

    std::vector<std::string> monthNames = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    reviewGear.name = monthNames[month - 1] + " Trophy (Year " + std::to_string(year) + ")";

    // Scale with year
    double yearMult = 1.0 + (year * 0.3);

    reviewGear.runway = static_cast<int>((15 + playerSeries * 2) * yearMult);
    reviewGear.hypeEnergy = static_cast<int>((12 + playerSeries * 1.5) * yearMult);
    reviewGear.hustle = static_cast<int>((6 + playerSeries) * yearMult);
    reviewGear.agility = static_cast<int>((5 + playerSeries * 0.8) * yearMult);
    reviewGear.ndas = static_cast<int>((4 + playerSeries * 0.6) * yearMult);
    reviewGear.luck = static_cast<int>((3 + playerSeries * 0.5) * yearMult);
    reviewGear.synergy = static_cast<int>((5 + playerSeries * 0.8) * yearMult);
    reviewGear.technicalDebt = static_cast<int>((2 + playerSeries * 3) * yearMult);
    reviewGear.busFactor = -1e-21f * yearMult;
    reviewGear.equipmentvalue = static_cast<int>(20000 * yearMult);

    if (reviewGear.type == EquipmentType::BACKPACK) {
        reviewGear.inventoryLimit = 20 + (year * 2);
    }

    return reviewGear;
}

bool SpecialDropsLibrary::isSpecialEquipment(const std::string &equipmentName) const {
    for (const auto &drop: library_) {
        if (drop.equipment.name == equipmentName) {
            return true;
        }
    }
    return false;
}

void SpecialDropsLibrary::markAsEquipped(int uniqueID, bool equipped) {
    for (auto &drop: library_) {
        if (drop.uniqueID == uniqueID) {
            drop.isEquipped = equipped;
            break;
        }
    }
}

const SpecialDrop *SpecialDropsLibrary::getDrop(int uniqueID) const {
    for (const auto &drop: library_) {
        if (drop.uniqueID == uniqueID) {
            return &drop;
        }
    }
    return nullptr;
}

int SpecialDropsLibrary::getDropsByType(SpecialDropType type) const {
    int count = 0;
    for (const auto &drop: library_) {
        if (drop.type == type) {
            count++;
        }
    }
    return count;
}

std::vector<SpecialDrop> SpecialDropsLibrary::getDropsInYear(int year) const {
    std::vector<SpecialDrop> drops;
    for (const auto &drop: library_) {
        if (drop.yearAcquired == year) {
            drops.push_back(drop);
        }
    }
    return drops;
}

bool SpecialDropsLibrary::hasDropFromSource(const std::string &sourceName) const {
    for (const auto &drop: library_) {
        if (drop.acquiredFrom == sourceName) {
            return true;
        }
    }
    return false;
}

void SpecialDropsLibrary::displayLibrary() const {
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║              📚 SPECIAL DROPS LIBRARY 📚                       ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << std::endl;

    if (library_.empty()) {
        std::cout << "\n📭 Library is empty. Defeat special enemies to collect unique items!" << std::endl;
        return;
    }

    std::cout << "\nTotal Special Items: " << library_.size() << std::endl;
    std::cout << "⚠️  These items are UNSELLABLE and kept forever\n" << std::endl;

    for (const auto &drop: library_) {
        std::cout << "──────────────────────────────────────────────────────────────" << std::endl;
        std::cout << getDropTypeIcon(drop.type) << " [ID: " << drop.uniqueID << "] " << drop.name << (drop.isEquipped ? " ✓ EQUIPPED" : "") << std::endl;
        std::cout << "Type: " << getDropTypeString(drop.type) << " | From: " << drop.acquiredFrom << std::endl;
        std::cout << "Acquired: Year " << drop.yearAcquired << ", Month " << drop.monthAcquired << std::endl;

        const Equipment &eq = drop.equipment;
        std::cout << "Stats: R+" << eq.runway << " H+" << eq.hypeEnergy << " Hu+" << eq.hustle << " A+" << eq.agility << " N+" << eq.ndas << " L+" << eq.luck << " S+" << eq.synergy << " TD" << eq.technicalDebt << " BF+" << eq.busFactor << std::endl;
    }
    std::cout << "──────────────────────────────────────────────────────────────" << std::endl;
}

void SpecialDropsLibrary::displayByType(SpecialDropType type) const {
    std::cout << "\n" << getDropTypeIcon(type) << " " << getDropTypeString(type) << " Drops:" << std::endl;

    int count = 0;
    for (const auto &drop: library_) {
        if (drop.type == type) {
            std::cout << "  [" << drop.uniqueID << "] " << drop.name << " (Year " << drop.yearAcquired << ")" << std::endl;
            count++;
        }
    }

    if (count == 0) {
        std::cout << "  No items of this type yet." << std::endl;
    }
}

void SpecialDropsLibrary::displayStatistics() const {
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                  📊 LIBRARY STATISTICS 📊                      ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << std::endl;

    std::cout << "\nTotal Items: " << library_.size() << std::endl;
    std::cout << "\nBy Type:" << std::endl;
    std::cout << " Competitor: " << getDropsByType(SpecialDropType::NEMESIS_DROP) << std::endl;
    std::cout << " Shadow Player: " << getDropsByType(SpecialDropType::SHADOW_PLAYER_DROP) << std::endl;
    std::cout << " Safety Manager: " << getDropsByType(SpecialDropType::SAFETY_MANAGER_DROP) << std::endl;
    std::cout << " Conference: " << getDropsByType(SpecialDropType::CONFERENCE_DROP) << std::endl;
    std::cout << " Paper Writing: " << getDropsByType(SpecialDropType::PAPER_WRITING_DROP) << std::endl;
    std::cout << " Monthly Boss: " << getDropsByType(SpecialDropType::MONTHLY_REVIEW_DROP) << std::endl;

    int equipped = 0;
    for (const auto &drop: library_) {
        if (drop.isEquipped) equipped++;
    }
    std::cout << "\nCurrently Equipped: " << equipped << "/" << library_.size() << std::endl;
}

void SpecialDropsLibrary::displayDetailedItem(int uniqueID) const {
    const SpecialDrop *drop = getDrop(uniqueID);
    if (!drop) {
        std::cout << "Item ID " << uniqueID << " not found!" << std::endl;
        return;
    }

    std::cout << "\n╔════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    ITEM DETAILS                                ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << std::endl;

    std::cout << "\n" << getDropTypeIcon(drop->type) << " " << drop->name << std::endl;
    std::cout << "ID: " << drop->uniqueID << " | Type: " << getDropTypeString(drop->type) << std::endl;
    std::cout << "Status: " << (drop->isEquipped ? "✓ EQUIPPED" : "In Library") << std::endl;
    std::cout << "\nAcquisition:" << std::endl;
    std::cout << "  From: " << drop->acquiredFrom << std::endl;
    std::cout << "  Year: " << drop->yearAcquired << " | Month: " << drop->monthAcquired << std::endl;

    const Equipment &eq = drop->equipment;
    std::cout << "\nEquipment Stats:" << std::endl;
    std::cout << "  Class: " << eq.equipmentclass << " | Rarity: " << static_cast<int>(eq.rarity) << std::endl;
    std::cout << "  Runway: +" << eq.runway << std::endl;
    std::cout << "  Hype Energy: +" << eq.hypeEnergy << std::endl;
    std::cout << "  Hustle: +" << eq.hustle << std::endl;
    std::cout << "  Agility: +" << eq.agility << std::endl;
    std::cout << "  NDAs: +" << eq.ndas << std::endl;
    std::cout << "  Luck: +" << eq.luck << std::endl;
    std::cout << "  Synergy: +" << eq.synergy << std::endl;
    std::cout << "  Technical Debt: " << eq.technicalDebt << std::endl;
    std::cout << "  Bus Factor: +" << eq.busFactor << std::endl;

    if (!drop->loreText.empty()) {
        std::cout << "\n📖 Lore: " << drop->loreText << std::endl;
    }

    std::cout << "\n⚠️  This item cannot be sold!" << std::endl;
}

const std::vector<SpecialDrop>& SpecialDropsLibrary::getLibrary() const { return library_; }

Equipment SpecialDropsLibrary::generateNamedMonsterCertificate(
    const std::string& monsterName,
    EquipmentType type,
    EquipmentRarity rarity,
    int playerSeries
) {
    Equipment cert;

    cert.type = type;
    cert.rarity = rarity;
    cert.equipmentclass = "Named";
    cert.name = monsterName + " Certificate";

    // Base stats for the equipment type (use existing generator)
    // Then apply rarity multiplier
    float multiplier = 1.0f;
    switch (rarity) {
        case EquipmentRarity::UNCOMMON: multiplier = 1.2f; break;
        case EquipmentRarity::RARE: multiplier = 1.5f; break;
        case EquipmentRarity::EPIC: multiplier = 2.0f; break;
        case EquipmentRarity::LEGENDARY: multiplier = 5.0f; break;
        default: multiplier = 1.0f; break;
    }

    // Scale base stats with series and rarity
    cert.runway = static_cast<int>((20 + playerSeries * 3) * multiplier);
    cert.hypeEnergy = static_cast<int>((15 + playerSeries * 2) * multiplier);
    cert.hustle = static_cast<int>((8 + playerSeries * 1.5) * multiplier);
    cert.agility = static_cast<int>((8 + playerSeries * 1.5) * multiplier);
    cert.ndas = static_cast<int>((6 + playerSeries) * multiplier);
    cert.luck = static_cast<int>((5 + playerSeries) * multiplier);
    cert.synergy = static_cast<int>((7 + playerSeries * 1.5) * multiplier);
    cert.technicalDebt = -static_cast<int>((3 + playerSeries * 0.5) * multiplier);
    cert.busFactor = 1.5f * multiplier;
    cert.equipmentvalue = 1000000; // Priceless

    if (type == EquipmentType::BACKPACK) {
        cert.inventoryLimit = static_cast<int>(30 * multiplier);
    }

    return cert;
}