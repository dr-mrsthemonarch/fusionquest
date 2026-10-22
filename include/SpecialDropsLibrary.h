#ifndef SPECIALDROPSLIBRARY_H
#define SPECIALDROPSLIBRARY_H

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include "Equipment.h"

enum class SpecialDropType {
    NEMESIS_DROP,
    SHADOW_PLAYER_DROP,
    SAFETY_MANAGER_DROP,
    CONFERENCE_DROP,
    PAPER_WRITING_DROP,
    MONTHLY_REVIEW_DROP,
    NAMED_MONSTER_DROP,
    QUEST_DROP
};

struct SpecialDrop {
    std::string name;
    std::string description;
    SpecialDropType type;
    Equipment equipment; // The actual equipment item
    std::string acquiredFrom; // Which enemy dropped it
    int yearAcquired; // Game year when obtained
    int monthAcquired; // Game month when obtained
    double gameTimeAcquired; // Exact game time
    bool isEquipped; // Currently equipped?

    // Special properties
    std::string loreText; // Flavor text about the item
    int uniqueID; // Unique identifier
};

Equipment generateNamedMonsterCertificate(
    const std::string& monsterName,
    EquipmentType type,
    EquipmentRarity rarity,
    int playerSeries
);

class SpecialDropsLibrary {
public:
    SpecialDropsLibrary();

    // Add special drops to library (CANNOT BE REMOVED)
    void addSpecialDrop(const SpecialDrop &drop);

    // Create special equipment from different sources
    Equipment generateNemesisEquipment(int playerSeries, int nemesisPowerLevel, const std::string &nemesisName);

    Equipment generateShadowPlayerEquipment(int playerSeries, const std::string &playerName);

    Equipment generateSafetyManagerEquipment(int playerSeries);

    Equipment generateConferenceEquipment(int playerSeries);

    Equipment generatePaperWritingEquipment(int playerSeries);

    Equipment generateMonthlyReviewEquipment(int year, int month, int playerSeries);

    // View library
    void displayLibrary() const;

    void displayByType(SpecialDropType type) const;

    void displayByYear(int year) const;

    void displayEquippedItems() const;

    void displayDetailedItem(int uniqueID) const;

    const std::vector<SpecialDrop> &getLibrary() const;

    Equipment generateNamedMonsterCertificate(const std::string &monsterName, EquipmentType type, EquipmentRarity rarity, int playerSeries);

    Equipment generateQuestEquipment(int playerSeries, EquipmentRarity rarity, const std::string &questTitle);

    // Search and query
    int getTotalDrops() const { return static_cast<int>(library_.size()); }

    std::string getDropTypeString(SpecialDropType type) const;

    std::string getDropTypeIcon(SpecialDropType type) const;

    int getDropsByType(SpecialDropType type) const;

    std::vector<SpecialDrop> getDropsInYear(int year) const;

    bool hasDropFromSource(const std::string &sourceName) const;

    // Check if equipment is special (unsellable)
    bool isSpecialEquipment(const std::string &equipmentName) const;

    // Update equipped status when player equips/unequips
    void markAsEquipped(int uniqueID, bool equipped);

    // Get specific drop
    const SpecialDrop *getDrop(int uniqueID) const;

    // Statistics
    void displayStatistics() const;

private:
    std::vector<SpecialDrop> library_;
    int nextUniqueID_;


};

#endif // SPECIALDROPSLIBRARY_H
