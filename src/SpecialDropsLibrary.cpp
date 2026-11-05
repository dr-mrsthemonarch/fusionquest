// SPG Engine made by drmrsthemonarch
#include "../include/SpecialDropsLibrary.h"
#include "../include/GameMath.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace {
// stats come from GameMath (gear tables). this file only picks type / name / rarity.
void applyGear(Equipment& e, const GameMath::GearStats& g) {
    e.runway = g.runway; e.hypeEnergy = g.hype; e.hustle = g.hustle; e.agility = g.agility;
    e.ndas = g.ndas; e.luck = g.luck; e.synergy = g.synergy; e.technicalDebt = g.techDebt;
    e.busFactor = g.bus; e.equipmentvalue = g.value;
    e.inventoryLimit = e.type == EquipmentType::BACKPACK ? g.inventoryLimit : 0;
}
}  // namespace

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
        case SpecialDropType::QUEST_DROP: return "Quest";
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
        case SpecialDropType::QUEST_DROP: return "🏆";
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
    Equipment gear;
    EquipmentType types[] = {EquipmentType::LAPTOP, EquipmentType::PHONE, EquipmentType::BACKPACK, EquipmentType::HELMET, EquipmentType::BIKE, EquipmentType::JACKET, EquipmentType::GLOVES, EquipmentType::HEADPHONES};
    gear.type = types[nemesisPowerLevel % 8];
    gear.rarity = EquipmentRarity::LEGENDARY;
    gear.equipmentclass = "Competitor";
    gear.name = nemesisPowerLevel == 0 ? "Competitor Memento" : "Competitor Relic [+" + std::to_string(nemesisPowerLevel) + "]";
    applyGear(gear, GameMath::gearNemesis(playerSeries, nemesisPowerLevel));
    return gear;
}

Equipment SpecialDropsLibrary::generateShadowPlayerEquipment(int playerSeries, const std::string &playerName) {
    Equipment gear;
    gear.type = EquipmentType::JACKET;
    gear.rarity = EquipmentRarity::LEGENDARY;
    gear.equipmentclass = "Shadow";
    gear.name = "Shadow's Reflection";
    applyGear(gear, GameMath::gearShadow(playerSeries));
    return gear;
}

Equipment SpecialDropsLibrary::generateSafetyManagerEquipment(int playerSeries) {
    Equipment gear;
    gear.type = EquipmentType::HELMET;
    gear.rarity = EquipmentRarity::RARE;
    gear.equipmentclass = "Safety";
    gear.name = "Safety Manager's Hardhat";
    applyGear(gear, GameMath::gearSafety(playerSeries));
    return gear;
}

Equipment SpecialDropsLibrary::generateConferenceEquipment(int playerSeries) {
    Equipment gear;
    gear.type = EquipmentType::HEADPHONES;
    gear.rarity = EquipmentRarity::RARE;
    gear.equipmentclass = "Conference";
    gear.name = "Keynote Speaker's Headset";
    applyGear(gear, GameMath::gearConference(playerSeries));
    return gear;
}

Equipment SpecialDropsLibrary::generatePaperWritingEquipment(int playerSeries) {
    Equipment gear;
    gear.type = EquipmentType::LAPTOP;
    gear.rarity = EquipmentRarity::RARE;
    gear.equipmentclass = "Academic";
    gear.name = "Researcher's Workstation";
    applyGear(gear, GameMath::gearPaper(playerSeries));
    return gear;
}

Equipment SpecialDropsLibrary::generateMonthlyReviewEquipment(int year, int month, int playerSeries) {
    Equipment gear;
    EquipmentType types[] = {EquipmentType::LAPTOP, EquipmentType::PHONE, EquipmentType::BACKPACK, EquipmentType::HELMET, EquipmentType::BIKE, EquipmentType::JACKET, EquipmentType::GLOVES, EquipmentType::HEADPHONES};
    gear.type = types[month % 8];
    gear.rarity = EquipmentRarity::EPIC;
    gear.equipmentclass = "Monthly";
    std::vector<std::string> monthNames = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    gear.name = monthNames[month - 1] + " Trophy (Year " + std::to_string(year) + ")";
    applyGear(gear, GameMath::gearReview(year, playerSeries));
    return gear;
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
    applyGear(cert, GameMath::gearNamed(playerSeries, rarity));
    return cert;
}
Equipment SpecialDropsLibrary::generateQuestEquipment(int playerSeries, EquipmentRarity rarity, const std::string& questTitle) {
    Equipment eq;
    eq.type = static_cast<EquipmentType>(rand() % 8);
    eq.rarity = rarity;
    eq.equipmentclass = "Quest";
    eq.name = questTitle + " Trophy";
    applyGear(eq, GameMath::gearQuest(playerSeries, rarity));
    return eq;
}
