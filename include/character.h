#ifndef CHARACTER_H
#define CHARACTER_H

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include "../include/CombatEntity.h"
#include "Equipment.h"
#include "Patent.h"
#include "TemporaryEffect.h"

class Patent; // Forward declaration

struct CharacterProfile {
    int runway = 80;
    int hypeEnergy = 10;
    int hustle = 20;
    int agility = 10;
    int ndas = 9;
    int luck = 5;
    int synergy = 5;
    int technicalDebt = -10;
    float busFactor = 1.0f;
};

class Character : public CombatEntity {
public:
    Character(const std::string &name, const std::string &employeeNr, const std::string &position, const std::string &startupName);

    // Combatant interface implementation
    int getRunway() const override;

    int getMaxRunway() const override;

    void setRunway(int runway) override;

    int getHypeEnergy() const override;

    int getHustle() const override;

    int getNDAs() const override;

    int getAgility() const override;

    int getLuck() const override;

    int getSynergy() const override;

    void takeDamage(int damage) override;

    bool isAlive() const override;

    std::string getName() const override;

    bool canUsePatents() const override;

    void addFunding(double funding_);

    double requiredFundingForNextLevel(double elapsedSeconds) const;

    int getSeries() const;

    std::string getStartup() const;

    int getTechnicalDebt() const;

    float getBusFactor() const;

    void addInventoryItem(const std::string &itemName, int value);

    void removeInventoryItem(const std::string &itemName);

    int getInventoryValue(const std::string &itemName) const;

    void initializeRandomItems();

    void createRandomItems();

    void printInventory() const;

    void setElapsedSeconds(double elapsedSeconds);

    double getElapsedSeconds() const { return seconds; }

    // Equipment system
    bool purchaseEquipment(const Equipment &equipment);

    bool equipItem(const Equipment &equipment);

    bool unequipItem(EquipmentType type);

    void addEquipmentToInventory(const Equipment &equipment);

    bool sellEquipment(const std::string &equipmentName); // Add this method
    int sellAllEquipment();

    bool sellEquipmentByIndex(size_t index); // Alternative method
    // Death/bankruptcy system
    void handleDeath();

    // Enhanced selling methods for bankruptcy
    int sellAllInventory();

    int sellAllPatents();

    int sellAllEquippedItems();

    // NEW: Inventory limit management
    int getInventoryLimit() const; // Get current max inventory based on backpack

    int getInventoryCount() const; // Get number of items in inventory

    bool isInventoryFull() const; // Check if inventory is at capacity

    void printInventoryStatus() const; // Display inventory status

    double getCurrentFunding() const;

    double getRequiredFunding() const;

    double getFundingProgress() const;

    // Get maximum values
    int getMaxHypeEnergy() const;

    // Restore character to full health/energy
    void restoreToFull();

    // Hype Energy management
    void setHypeEnergy(int value) override {
        hypeEnergy_ = std::max(0, std::min(value, maxHypeEnergy_));
    }

    void printEquipment() const;

    int getMoney() const;

    void addMoney(int amount);

    bool spendMoney(int amount);

    Equipment *getEquippedItem(EquipmentType type);

    bool isPrime(int num);

    const CharacterProfile &getProfile() const;

    std::unordered_map<std::string, double> getStats() const;

    const std::unordered_map<std::string, int> &getInventoryMap() const;

    TemporaryEffectsManager &getEffectsManager() override;

    void addTemporaryEffect(const TemporaryEffect &effect) override;

    void processTurnEffects() override;

    // Get modified stats (base + equipment + temporary effects)
    int getModifiedHustle() const override;

    int getModifiedAgility() const override;

    int getModifiedNDAs() const override;

    int getModifiedLuck() const override;

    int getModifiedSynergy() const override;

private:
    TemporaryEffectsManager effectsManager_;
    std::string name_;
    std::string employeeNr_; // Employee number
    std::string position_; // Position
    std::string startupName_; // Startup Name
    CharacterProfile profile_;
    CharacterProfile baseProfile_; // Base stats without equipment
    double seconds = 0.0;
    int funding_ = 0; // experience
    int currentFunding_ = 0;
    int series_ = 0; // Level
    int money_ = 1000; // Starting money
    int runway_ = 0; // Runway (HP equivalent)
    int hypeEnergy_ = 0; // Hype Energy (MP equivalent)
    int hustle_ = 0; // Hustle (Attack)
    int agility_ = 0; // Agility (Speed)
    int ndas_ = 0; // NDAs (Defense)
    int luck_ = 0; // Luck (Luck)
    int synergy_ = 0; // Synergy (Charisma)
    int technicalDebt_ = 0; // Technical Debt
    float busFactor_ = 1.0f; // Bus Factor
    bool isDead_ = false;
    int maxRunway_; // Maximum runway (full health)
    int maxHypeEnergy_; // Maximum hype energy (full MP)
    int sellAllInventoryForBankruptcy();

    int sellAllEquipmentForBankruptcy();

    int sellAllEquippedItemsForBankruptcy();

    int sellAllPatentsForBankruptcy();


    // Equipment slots - one for each type
    std::map<EquipmentType, Equipment> equippedItems_;
    std::vector<Equipment> equipmentInventory_;

    static constexpr int FUNDING_PER_LEVEL = 1000;

    void updateRunway();

    void updateHypeEnergy();

    void updateHustle();

    void updateAgility();

    void updateNDAs();

    void updateLuck();

    void updateSynergy();

    void updateTechnicalDebt();

    void recalculateStats();

    std::string equipmentTypeToString(EquipmentType type) const; // Added const here
    static std::unordered_map<std::string, int> inventory;
};

#endif  // CHARACTER_H
