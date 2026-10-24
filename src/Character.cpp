
#include "../include/Character.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <iostream>
#include <algorithm>
#include "../include/ItemGenerator.h"
#include "../include/Patent.h"
#include "../include/CombatEntity.h"
#include "../include/TemporaryEffect.h"

// Define the constant that's missing
constexpr double FUNDING_PER_LEVEL = 1000.0;

Character::Character(const std::string &name, const std::string &employeeNr, const std::string &position, const std::string &startupName)
: name_(name), employeeNr_(employeeNr), position_(position), startupName_(startupName) {

    // Initialize base profile
    baseProfile_.runway = 80;
    baseProfile_.hypeEnergy = 10;
    baseProfile_.hustle = 20;
    baseProfile_.agility = 10;
    baseProfile_.ndas = 9;
    baseProfile_.luck = 5;
    baseProfile_.synergy = 5;
    baseProfile_.technicalDebt = -10;
    baseProfile_.busFactor = 1.0f;

    profile_ = baseProfile_;

    // Initialize current stats
    runway_ = baseProfile_.runway;
    maxRunway_ = baseProfile_.runway;
    hypeEnergy_ = baseProfile_.hypeEnergy;
    maxHypeEnergy_ = baseProfile_.hypeEnergy;
    hustle_ = baseProfile_.hustle;
    agility_ = baseProfile_.agility;
    ndas_ = baseProfile_.ndas;
    luck_ = baseProfile_.luck;
    synergy_ = baseProfile_.synergy;
    technicalDebt_ = baseProfile_.technicalDebt;
    busFactor_ = baseProfile_.busFactor;
}

std::unordered_map<std::string, int> Character::inventory;
std::unordered_map<std::string, int> Character::inventoryCounts;
std::function<bool(const std::string&)> Character::sellProtected;

int Character::getSellableEquipmentCount() const {
    int n = 0;
    for (const auto& e : equipmentInventory_) if (!isSellProtected(e.name)) ++n;
    return n;
}

ItemGenerator g_itemGenerator;

void initializeGlobalItemGenerator() {
    static bool initialized = false;
    if (!initialized) {
        if (!g_itemGenerator.loadWordLists("config/items/")) {
            std::cerr << "ERROR: Could not load config files from config/items/" << std::endl;
            std::cerr << "Expected path: bin/config/items/" << std::endl;
        } else {
            std::cout << "Successfully loaded config files from config/items/" << std::endl;
        }
        initialized = true;
    }
}

void Character::takeDamage(int damage) {
    int defenseShield = effectsManager_.getDefenseShield();
    int actualDamage = std::max(1, damage - defenseShield);

    if (defenseShield > 0) {
        std::cout << name_ << "'s defense shield absorbs " << (damage - actualDamage) << " damage!\n";
    }

    runway_ = std::max(0, runway_ - actualDamage);
    profile_.runway = runway_;
}

bool Character::canUsePatents() const {
    return !Patent::characterPatents.empty() && hypeEnergy_ > 0;
}

void generateRandomItem(Character *character) {
    initializeGlobalItemGenerator();
    g_itemGenerator.generateRandomItem(character);
}

void Character::addEquipmentToInventory(const Equipment &equipment) {
    equipmentInventory_.push_back(equipment);
}

void Character::addMoney(int amount) {
    // money never negative, never overflows
    long long m = static_cast<long long>(money_) + amount;
    money_ = static_cast<int>(std::clamp<long long>(m, 0, 2000000000LL));
}

void Character::addFunding(double funding_) {
    int oldSeries = series_;
    double elapsedSeconds = seconds;

    // Add the funding to current accumulated funding
    currentFunding_ += funding_;

    const int MAX_ITER = 10000;
    int iter = 0;

    while (currentFunding_ >= requiredFundingForNextLevel(elapsedSeconds)) {
        if (++iter > MAX_ITER) {
            std::cerr << "[Warning] addFunding loop exceeded safety limit — check logic.\n";
            break;
        }

        double cost = requiredFundingForNextLevel(elapsedSeconds);

        if (currentFunding_ >= cost) {
            series_++;

            // Update BASE stats
            baseProfile_.runway += 20 + (series_ - 1) * 5;
            baseProfile_.hypeEnergy += 15 + (series_ - 1) * 3;
            baseProfile_.hustle += 3 + (series_ - 1);
            baseProfile_.agility += 4 + (series_ - 1) * 0.5;
            baseProfile_.ndas += 2 + (series_ - 1);
            baseProfile_.luck += 1 + (series_ - 1) * 0.3;
            baseProfile_.synergy += 3 + (series_ - 1) * 0.5;
            baseProfile_.technicalDebt -= 4 - (series_ - 1) / 2;
            baseProfile_.busFactor += 0.2f + (series_ - 1) * 0.05f;

            // Recalculate stats with equipment bonuses
            recalculateStats();

            // Store the new max values
            maxRunway_ = profile_.runway;
            maxHypeEnergy_ = profile_.hypeEnergy;

            // Restore to full health on level-up
            runway_ = maxRunway_;
            hypeEnergy_ = maxHypeEnergy_;

            // Reset funding to 0 (like XP resetting after level-up)
            currentFunding_ = 0;

            std::cout << name_ << " has leveled up to level " << series_
                      << "! Runway: " << runway_ << "/" << maxRunway_
                      << ", Hype Energy: " << hypeEnergy_ << "/" << maxHypeEnergy_
                      << ", Hustle: " << hustle_
                      << ", Agility: " << agility_
                      << ", NDAs: " << ndas_
                      << ", Luck: " << luck_
                      << ", Synergy: " << synergy_
                      << ", Technical Debt: " << technicalDebt_
                      << ", Bus Factor: " << busFactor_ << "\n";
        }

        seconds = 1 + elapsedSeconds;
    }

    if (series_ > oldSeries && oldSeries > 0) {  // ADD: && oldSeries > 0
        Patent::initializeRandomPatents(this, 1);
        Patent::levelUpCharacterPatents(this);
    }
}

void Character::addInventoryItem(const std::string &itemName, int value) {
    value = std::clamp(value, 0, 1000000);
    if (isInventoryFull()) {
        std::cout << "\n⚠️  INVENTORY FULL! ⚠️" << std::endl;
        std::cout << "Your backpack is at capacity (" << getInventoryCount() << "/" << getInventoryLimit() << " items)" << std::endl;
        std::cout << "You must sell all inventory items to make room!" << std::endl;

        // Force sell all inventory
        int moneyEarned = sellAllInventory();

        std::cout << "Sold all inventory for $" << moneyEarned << std::endl;
        std::cout << "Now adding new item: " << itemName << std::endl;
    }

    inventory[itemName] = value;
    inventoryCounts[itemName]++;
    std::cout << "Added " << itemName << " to inventory (" << getInventoryCount() << "/" << getInventoryLimit() << ")" << std::endl;
}

void Character::createRandomItems() {
    generateRandomItem(this);
}

std::string Character::equipmentTypeToString(EquipmentType type) const {
    // Added const here
    switch (type) {
        case EquipmentType::LAPTOP: return "Laptop";
        case EquipmentType::PHONE: return "Phone";
        case EquipmentType::BACKPACK: return "Backpack";
        case EquipmentType::HELMET: return "Helmet";
        case EquipmentType::BIKE: return "Bike";
        case EquipmentType::JACKET: return "Jacket";
        case EquipmentType::GLOVES: return "Gloves";
        case EquipmentType::HEADPHONES: return "Headphones";
        default: return "Unknown";
    }
}

bool Character::equipItem(const Equipment &equipment) {
    // Find the item in inventory by comparing multiple properties to ensure uniqueness
    auto it = std::find_if(equipmentInventory_.begin(), equipmentInventory_.end(), [&equipment](const Equipment &inv_item) {
        return inv_item.name == equipment.name && inv_item.type == equipment.type && inv_item.equipmentvalue == equipment.equipmentvalue;
    });

    if (it == equipmentInventory_.end()) {
        std::cout << "Equipment " << equipment.name << " not found in inventory!" << std::endl;
        return false;
    }

    // Store the equipment we found before any modifications
    Equipment itemToEquip = *it;

    // Remove from inventory first to avoid iterator issues
    equipmentInventory_.erase(it);

    // Check if there's already equipment in this slot
    auto slotIt = equippedItems_.find(equipment.type);
    if (slotIt != equippedItems_.end()) {
        // Unequip current item first
        std::cout << "Unequipping " << slotIt->second.name << " from " << equipmentTypeToString(equipment.type) << " slot" << std::endl;
        equipmentInventory_.push_back(slotIt->second);
    }

    // Equip the new item
    equippedItems_[equipment.type] = itemToEquip;
    recalculateStats();

    std::cout << "Equipped " << itemToEquip.name << " to " << equipmentTypeToString(equipment.type) << " slot" << std::endl;
    return true;
}

const CharacterProfile &Character::getProfile() const {
    return profile_;
}

std::unordered_map<std::string, double> Character::getStats() const {
    return {{"Runway", profile_.runway}, {"Hype Energy", profile_.hypeEnergy}, {"Hustle", profile_.hustle}, {"Agility", profile_.agility}, {"NDAs", profile_.ndas}, {"Luck", profile_.luck}, {"Synergy", profile_.synergy}, {"Technical Debt", profile_.technicalDebt}, {"Bus Factor", profile_.busFactor}};
}

const std::unordered_map<std::string, int>& Character::getInventoryMap() const {
    return inventory;
}

int Character::getSeries() const {
    return series_;
}

int Character::getRunway() const {
    return runway_;
}

int Character::getHypeEnergy() const {
    return hypeEnergy_;
}

int Character::getHustle() const {
    return hustle_;
}

int Character::getAgility() const {
    return agility_;
}

int Character::getNDAs() const {
    return ndas_;
}

std::string Character::getName() const {
    return name_;
}

std::string Character::getStartup() const {
    return startupName_;
}

int Character::getLuck() const {
    return luck_;
}

int Character::getSynergy() const {
    return synergy_;
}

int Character::getTechnicalDebt() const {
    return technicalDebt_;
}

float Character::getBusFactor() const {
    return busFactor_;
}

double Character::getCurrentFunding() const {
    return currentFunding_;
}

double Character::getRequiredFunding() const {
    return requiredFundingForNextLevel(seconds);
}

double Character::getFundingProgress() const {
    double required = requiredFundingForNextLevel(seconds);
    if (required <= 0) return 0.0;
    return (currentFunding_ / required) * 100.0;
}

int Character::getInventoryLimit() const {
    auto backpack = equippedItems_.find(EquipmentType::BACKPACK);
    if (backpack != equippedItems_.end()) {
        return backpack->second.inventoryLimit;
    }
    return 5; // Default minimum inventory without backpack
}

int Character::getInventoryCount() const {
    int total = 0;
    for (const auto &kv: inventoryCounts) total += kv.second;
    return total;
}

int Character::getMaxRunway() const {
    return maxRunway_;
}

int Character::getMaxHypeEnergy() const {
    return maxHypeEnergy_;
}

int Character::getInventoryValue(const std::string &itemName) const {
    auto it = inventory.find(itemName);
    if (it != inventory.end()) {
        return it->second;
    }
    return 0;
}

int Character::getMoney() const {
    return money_;
}

Equipment *Character::getEquippedItem(EquipmentType type) {
    auto it = equippedItems_.find(type);
    if (it != equippedItems_.end()) {
        return &(it->second);
    }
    return nullptr;
}

void Character::handleDeath() {
    if (isDead_) return; // Already handled

    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "All of the money is gone!" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    std::cout << name_ << " blew all of the money on drugs and prostitutes" << std::endl;
    std::cout << "All assets must be liquidated...to try and save the company" << std::endl;

    isDead_ = true;

    // Liquidate everything
    int moneyFromInventory = sellAllInventoryForBankruptcy();
    int moneyFromEquipment = sellAllEquipmentForBankruptcy();
    // int moneyFromEquipped = sellAllEquippedItemsForBankruptcy(); // maybe consider selling later for balance, but for now keep all equipment
    // int moneyFromPatents = sellAllPatentsForBankruptcy();

    int totalMoneyLost = money_;
    money_ = 0;

    std::cout << "\n=== BANKRUPTCY SUMMARY ===" << std::endl;
    std::cout << "Total lost: $" << (moneyFromInventory + moneyFromEquipment + /*moneyFromPatents + */ totalMoneyLost) << std::endl;

    // Set runway to 1 (critical condition)
    runway_ = 1;
    baseProfile_.runway = maxRunway_; // Keep max runway

    std::cout << "\n🏥 EMERGENCY STATUS 🏥" << std::endl;
    std::cout << "Runway: 1 (CRITICAL - must visit town to recover)" << std::endl;
    std::cout << "You must reach a town to restore health!" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    isDead_ = false; // Allow recovery
}

void Character::initializeRandomItems() {
    srand(time(nullptr));
    for (int i = 0; i < 1; ++i) {
        generateRandomItem(this);
    }
}

bool Character::isPrime(int num) {
    if (num <= 1) return false;
    if (num <= 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;

    for (long long i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) return false;
    }
    return true;
}

bool Character::isAlive() const {
    return runway_ > 0 && !isDead_;
}

bool Character::isInventoryFull() const {
    return getInventoryCount() >= getInventoryLimit();
}

void Character::printInventory() const {
    std::cout << "Character's Inventory:" << std::endl;
    for (const auto &item: inventory) {
        std::cout << "Item: " << item.first << ", Value: " << item.second << "\n";
    }
}

void Character::printInventoryStatus() const {
    std::cout << "\n=== INVENTORY STATUS ===" << std::endl;
    std::cout << "Items: " << getInventoryCount() << "/" << getInventoryLimit() << std::endl;

    auto backpack = equippedItems_.find(EquipmentType::BACKPACK);
    if (backpack != equippedItems_.end()) {
        std::cout << "Backpack: " << backpack->second.name << " (Limit: " << backpack->second.inventoryLimit << ")" << std::endl;
    } else {
        std::cout << "No backpack equipped (Default limit: 5)" << std::endl;
    }

    if (!inventory.empty()) {
        std::cout << "\nItems in inventory:" << std::endl;
        int totalValue = 0;
        for (const auto &item: inventory) {
            std::cout << "  - " << item.first << " ($" << item.second << ")" << std::endl;
            totalValue += item.second;
        }
        std::cout << "Total inventory value: $" << totalValue << std::endl;
    } else {
        std::cout << "Inventory is empty." << std::endl;
    }
}

bool Character::purchaseEquipment(const Equipment &equipment) {
    if (money_ >= equipment.equipmentvalue) {
        money_ -= equipment.equipmentvalue;
        addEquipmentToInventory(equipment);
        std::cout << "Purchased " << equipment.name << " for $" << equipment.equipmentvalue << std::endl;
        std::cout << "Remaining money: $" << money_ << std::endl;
        return true;
    } else {
        std::cout << "Not enough money to purchase " << equipment.name << ". Need $" << equipment.equipmentvalue << ", have $" << money_ << std::endl;
        return false;
    }
}

void Character::printEquipment() const {
    std::cout << "\n=== EQUIPPED ITEMS ===" << std::endl;
    if (equippedItems_.empty()) {
        std::cout << "No items equipped." << std::endl;
    } else {
        for (const auto &slot: equippedItems_) {
            const Equipment &eq = slot.second;
            std::cout << "- " << equipmentTypeToString(slot.first) << ": " << eq.name << " (Class: " << eq.equipmentclass << ", Value: $" << eq.equipmentvalue << ")" << std::endl;
            std::cout << "  Stats: R+" << eq.runway << " H+" << eq.hypeEnergy << " Hu+" << eq.hustle << " A+" << eq.agility << " N+" << eq.ndas << " L+" << eq.luck << " S+" << eq.synergy << " TD" << eq.technicalDebt << " BF+" << eq.busFactor << std::endl;
        }
    }

    std::cout << "\n=== EQUIPMENT INVENTORY ===" << std::endl;
    if (equipmentInventory_.empty()) {
        std::cout << "No equipment in inventory." << std::endl;
    } else {
        for (const auto &eq: equipmentInventory_) {
            std::cout << "- " << eq.name << " (" << equipmentTypeToString(eq.type) << ", Class: " << eq.equipmentclass << ", Value: $" << eq.equipmentvalue << ")" << std::endl;
        }
    }
    std::cout << "\nMoney: $" << money_ << std::endl;
}

void Character::recalculateStats() {
    // Start with base profile (untouched by damage/temporary effects)
    profile_ = baseProfile_;

    // Add equipment bonuses from all equipped slots
    for (const auto &slot: equippedItems_) {
        const Equipment &eq = slot.second;
        profile_.runway += eq.runway;
        profile_.hypeEnergy += eq.hypeEnergy;
        profile_.hustle += eq.hustle;
        profile_.agility += eq.agility;
        profile_.ndas += eq.ndas;
        profile_.luck += eq.luck;
        profile_.synergy += eq.synergy;
        profile_.technicalDebt += eq.technicalDebt;
        profile_.busFactor += eq.busFactor;
    }

    hustle_ = profile_.hustle;
    agility_ = profile_.agility;
    ndas_ = profile_.ndas;
    luck_ = profile_.luck;
    synergy_ = profile_.synergy;
    technicalDebt_ = profile_.technicalDebt;
    busFactor_ = profile_.busFactor;

}

void Character::restoreToFull() {
    runway_ = maxRunway_;
    hypeEnergy_ = maxHypeEnergy_;

    std::cout << "\n✨ FULLY RESTORED! ✨" << std::endl;
    std::cout << "Runway: " << runway_ << "/" << maxRunway_ << std::endl;
    std::cout << "Hype Energy: " << hypeEnergy_ << "/" << maxHypeEnergy_ << std::endl;
}

double Character::requiredFundingForNextLevel(double elapsedSeconds) const {
    constexpr double TOTAL_GAME_TIME = 20.0 * 365 * 24 * 60 * 60; // 20 years
    constexpr double FUNDING_BASE = FUNDING_PER_LEVEL;
    constexpr double EXPONENT = 2.8970554778884745; // polynomial scaling
    constexpr double K = 1.0; // sigmoid steepness
    constexpr double C = 0.5; // midpoint
    constexpr double MIN_SLOWDOWN = 0.05;

    // Convert elapsed time into normalized progression (0 → 1)
    double p = std::clamp(elapsedSeconds / TOTAL_GAME_TIME, 0.0, 1.0);

    // Logistic slowdown to simulate plateau
    double slowdown = 1.0 / (1.0 + std::exp(K * (p - C)));
    slowdown = std::max(slowdown, MIN_SLOWDOWN);

    // Add small randomness to hide deterministic feel
    double noise = 0.95 + ((rand() % 11) / 100.0);

    // Funding cost increases with time and prime progression pressure
    return FUNDING_BASE * std::pow(series_, EXPONENT) / (slowdown * noise);
}

void Character::removeInventoryItem(const std::string &itemName) {
    if (inventory.find(itemName) != inventory.end()) {
        inventory.erase(itemName);
        inventoryCounts.erase(itemName);
    }
}

bool Character::spendMoney(int amount) {
    if (money_ >= amount) {
        money_ -= amount;
        return true;
    }
    return false;
}

bool Character::sellEquipment(const std::string &equipmentName) {
    // Find the equipment in inventory (not equipped items)
    auto it = std::find_if(equipmentInventory_.begin(), equipmentInventory_.end(), [&equipmentName](const Equipment &item) {
        return item.name == equipmentName && !isSellProtected(item.name);
    });

    if (it == equipmentInventory_.end()) {
        std::cout << "Equipment '" << equipmentName << "' not found in inventory!" << std::endl;
        std::cout << "Note: You can only sell unequipped items." << std::endl;
        return false;
    }

    // Calculate sell price (typically 50-75% of original value)
    int sellPrice = static_cast<int>(it->equipmentvalue * 0.6); // 60% of original value

    std::cout << "Selling " << it->name << " for $" << sellPrice << std::endl;

    // Add money and remove item
    addMoney(sellPrice);
    equipmentInventory_.erase(it);

    std::cout << "Sale complete! Current money: $" << money_ << std::endl;
    return true;
}

int Character::sellAllEquipment() {
    if (equipmentInventory_.empty()) {
        std::cout << "No equipment to sell." << std::endl;
        return 0;
    }

    int totalEarned = 0;
    int itemsSold = 0;

    std::cout << "Selling all inventory equipment..." << std::endl;

    // Iterate backwards or use while loop to avoid index issues
    for (size_t i = equipmentInventory_.size(); i-- > 0;) {
        const Equipment &item = equipmentInventory_[i];
        if (isSellProtected(item.name)) continue; // special drops never sell
        int sellPrice = static_cast<int>(item.equipmentvalue * 0.6);

        std::cout << "- Sold " << item.name << " for $" << sellPrice << std::endl;

        totalEarned += sellPrice;
        itemsSold++;
        equipmentInventory_.erase(equipmentInventory_.begin() + i);
    }

    addMoney(totalEarned);

    std::cout << "Sale complete! Sold " << itemsSold << " items for $" << totalEarned << std::endl;
    std::cout << "Current money: $" << money_ << std::endl;

    return totalEarned;
}

bool Character::sellEquipmentByIndex(size_t index) {
    if (index >= equipmentInventory_.size()) {
        std::cout << "Invalid equipment index! Available items: 0-" << (equipmentInventory_.size() - 1) << std::endl;
        return false;
    }

    if (isSellProtected(equipmentInventory_[index].name)) {
        std::cout << "Special drops cannot be sold!" << std::endl;
        return false;
    }
    const Equipment &item = equipmentInventory_[index];
    int sellPrice = static_cast<int>(item.equipmentvalue * 0.6); // 60% of original value

    std::cout << "Selling " << item.name << " for $" << sellPrice << std::endl;

    addMoney(sellPrice);
    equipmentInventory_.erase(equipmentInventory_.begin() + index);

    std::cout << "Sale complete! Current money: $" << money_ << std::endl;
    return true;
}

int Character::sellAllEquippedItems() {
    if (equippedItems_.empty()) {
        std::cout << "No equipped items to sell." << std::endl;
        return 0;
    }

    int totalEarned = 0;
    int itemsSold = 0;

    std::cout << "\nLiquidating all equipped items..." << std::endl;

    for (const auto &slot: equippedItems_) {
        const Equipment &item = slot.second;
        int sellPrice = static_cast<int>(item.equipmentvalue * 0.6);
        std::cout << "- Unequipped and sold " << item.name << " from " << equipmentTypeToString(slot.first) << " slot for $" << sellPrice << std::endl;
        totalEarned += sellPrice;
        itemsSold++;
    }

    // Clear all equipped items
    equippedItems_.clear();
    recalculateStats(); // Recalculate stats without equipment

    std::cout << "Equipped items liquidation complete! Sold " << itemsSold << " items for $" << totalEarned << std::endl;

    return totalEarned;
}

int Character::sellAllInventoryForBankruptcy() {
    if (inventory.empty()) {
        std::cout << "No inventory items to sell." << std::endl;
        return 0;
    }

    int totalEarned = 0;
    int itemsSold = 0;

    std::cout << "\nLiquidating all inventory items..." << std::endl;

    for (const auto &item: inventory) {
        auto cit = inventoryCounts.find(item.first);
        int n = cit == inventoryCounts.end() ? 1 : cit->second;
        std::cout << "- Sold " << item.first << " x" << n << " for $" << item.second * n << std::endl;
        totalEarned += item.second * n;
        itemsSold++;
    }

    // Clear the inventory
    inventory.clear();
    inventoryCounts.clear();

    std::cout << "Inventory liquidation complete! Sold " << itemsSold << " items for $" << totalEarned << std::endl;

    return totalEarned;
}

int Character::sellAllEquipmentForBankruptcy() {
    if (equipmentInventory_.empty()) {
        std::cout << "No equipment to sell." << std::endl;
        return 0;
    }

    int totalEarned = 0;
    int itemsSold = 0;

    std::cout << "\nLiquidating all equipment..." << std::endl;

    for (size_t i = equipmentInventory_.size(); i-- > 0;) {
        const Equipment &item = equipmentInventory_[i];
        if (isSellProtected(item.name)) continue; // special drops never sell
        int sellPrice = static_cast<int>(item.equipmentvalue * 0.6);
        std::cout << "- Sold " << item.name << " for $" << sellPrice << std::endl;
        totalEarned += sellPrice;
        itemsSold++;
        equipmentInventory_.erase(equipmentInventory_.begin() + i);
    }

    std::cout << "Equipment liquidation complete! Sold " << itemsSold << " items for $" << totalEarned << std::endl;

    // Don't add money during bankruptcy
    return totalEarned;
}

int Character::sellAllEquippedItemsForBankruptcy() {
    if (equippedItems_.empty()) {
        std::cout << "No equipped items to sell." << std::endl;
        return 0;
    }

    int totalEarned = 0;
    int itemsSold = 0;

    std::cout << "\nLiquidating all equipped items..." << std::endl;

    for (const auto &slot: equippedItems_) {
        const Equipment &item = slot.second;
        int sellPrice = static_cast<int>(item.equipmentvalue * 0.6);
        std::cout << "- Unequipped and sold " << item.name << " from " << equipmentTypeToString(slot.first) << " slot for $" << sellPrice << std::endl;
        totalEarned += sellPrice;
        itemsSold++;
    }

    // Clear all equipped items
    equippedItems_.clear();
    recalculateStats(); // Recalculate stats without equipment

    std::cout << "Equipped items liquidation complete! Sold " << itemsSold << " items for $" << totalEarned << std::endl;

    // Don't add money during bankruptcy
    return totalEarned;
}

int Character::sellAllPatentsForBankruptcy() {
    if (Patent::characterPatents.empty()) {
        std::cout << "No patents to sell." << std::endl;
        return 0;
    }

    int totalEarned = 0;
    int patentsSold = 0;

    std::cout << "\nLiquidating all patents..." << std::endl;

    for (const auto &patent: Patent::characterPatents) {
        int salePrice = 100 + (patent.second.getLevel() * 50); // Base $100 + $50 per level
        std::cout << "- Sold " << patent.second.getFullName() << " (Level " << patent.second.getLevel() << ") for $" << salePrice << std::endl;
        totalEarned += salePrice;
        patentsSold++;
    }

    // Clear all patents
    Patent::characterPatents.clear();

    std::cout << "Patent liquidation complete! Sold " << patentsSold << " patents for $" << totalEarned << std::endl;

    // Don't add money during bankruptcy
    return totalEarned;
}

void Character::setRunway(int runway) {
    runway_ = runway;
    profile_.runway = runway;
    // REMOVED: baseProfile_.runway = runway; <-- This was destroying the base stats!
}

int Character::sellAllInventory() {
    if (inventory.empty()) {
        std::cout << "No inventory items to sell." << std::endl;
        return 0;
    }

    int totalEarned = 0;
    int itemsSold = 0;

    std::cout << "\nSelling all inventory items..." << std::endl;

    for (const auto &item: inventory) {
        auto cit = inventoryCounts.find(item.first);
        int n = cit == inventoryCounts.end() ? 1 : cit->second;
        std::cout << "- Sold " << item.first << " x" << n << " for $" << item.second * n << std::endl;
        totalEarned += item.second * n;
        itemsSold++;
    }

    // Clear the inventory
    inventory.clear();
    inventoryCounts.clear();

    std::cout << "Sold " << itemsSold << " items for $" << totalEarned << std::endl;

    // Add money to character
    addMoney(totalEarned);
    std::cout << "Current money: $" << money_ << std::endl;

    return totalEarned;
}

void Character::setElapsedSeconds(double elapsedSeconds) {
    seconds = elapsedSeconds;
}

bool Character::unequipItem(EquipmentType type) {
    auto it = equippedItems_.find(type);
    if (it != equippedItems_.end()) {
        equipmentInventory_.push_back(it->second);
        std::cout << "Unequipped " << it->second.name << " from " << equipmentTypeToString(type) << " slot" << std::endl;
        equippedItems_.erase(it);
        recalculateStats();
        return true;
    } else {
        std::cout << "No equipment equipped in " << equipmentTypeToString(type) << " slot!" << std::endl;
        return false;
    }
}

TemporaryEffectsManager& Character::getEffectsManager() {
    return effectsManager_;
}

void Character::addTemporaryEffect(const TemporaryEffect& effect) {
    effectsManager_.addEffect(effect);
    std::cout << name_ << " gained effect: " << effect.getDescription() << "\n";
}

void Character::processTurnEffects() {
    // Process damage over time
    int dotDamage = effectsManager_.processDamageOverTime();
    if (dotDamage > 0) {
        runway_ = std::max(0, runway_ - dotDamage);
        std::cout << name_ << " takes " << dotDamage << " damage from DoT effects\n";
    }

    // Process regeneration
    int regenAmount = effectsManager_.processRegeneration();
    if (regenAmount > 0) {
        int oldRunway = runway_;
        runway_ = std::min(maxRunway_, runway_ + regenAmount);
        int actualRegen = runway_ - oldRunway;
        if (actualRegen > 0) {
            std::cout << name_ << " regenerates " << actualRegen << " runway\n";
        }
    }

    effectsManager_.processTurn();
}

int Character::getModifiedHustle() const {
    return std::max(1, hustle_ + effectsManager_.getHustleModifier());
}

int Character::getModifiedAgility() const {
    return std::max(1, agility_ + effectsManager_.getAgilityModifier());
}

int Character::getModifiedNDAs() const {
    return std::max(0, ndas_ + effectsManager_.getNdasModifier() + effectsManager_.getDefenseShield());
}

int Character::getModifiedLuck() const {
    return std::max(1, luck_ + effectsManager_.getLuckModifier());
}

int Character::getModifiedSynergy() const {
    return std::max(1, synergy_ + effectsManager_.getSynergyModifier());
}

