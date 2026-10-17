//
// Created by drmrsthemonarch
//

#ifndef BATTLEENCOUNTER_H
#define BATTLEENCOUNTER_H

#include <vector>
#include <memory>
#include "Monster.h"
#include "CombatEntity.h"

class BattleEncounter {
public:
    BattleEncounter();

    // Add enemies to the encounter
    void addEnemy(std::unique_ptr<Monster> enemy);

    // Get all enemies (alive and dead)
    std::vector<Monster*> getAllEnemies();

    // Get only alive enemies
    std::vector<Monster*> getAliveEnemies();

    // Get enemy at index (nullptr if invalid)
    Monster* getEnemy(size_t index);

    // Check if any enemies are alive
    bool hasAliveEnemies() const;

    // Get count of alive enemies
    int getAliveCount() const;

    // Get total enemy count
    int getTotalCount() const;

    // Auto-target enemy (lowest HP)
    Monster* getLowestHPEnemy();

    // Calculate total rewards from all defeated enemies
    int getTotalMoneyReward() const;

    // Get all item rewards
    std::vector<std::string> getAllItemRewards() const;

    // Get all equipment drops
    std::vector<Equipment> getAllEquipmentDrops() const;

    // Check if encounter contains boss
    bool hasBoss() const;

    // Check if encounter contains special enemy
    bool hasSpecialEnemy() const;

    // Clear all enemies
    void clear();

private:
    std::vector<std::unique_ptr<Monster>> enemies_;
};

#endif // BATTLEENCOUNTER_H