//
// Created by drmrsthemonarch
//

#include "../include/BattleEncounter.h"
#include <algorithm>

BattleEncounter::BattleEncounter() {}

void BattleEncounter::addEnemy(std::unique_ptr<Monster> enemy) {
    enemies_.push_back(std::move(enemy));
}

std::vector<Monster*> BattleEncounter::getAllEnemies() {
    std::vector<Monster*> result;
    for (auto& enemy : enemies_) {
        result.push_back(enemy.get());
    }
    return result;
}

std::vector<Monster*> BattleEncounter::getAliveEnemies() {
    std::vector<Monster*> result;
    for (auto& enemy : enemies_) {
        if (enemy->isAlive()) {
            result.push_back(enemy.get());
        }
    }
    return result;
}

Monster* BattleEncounter::getEnemy(size_t index) {
    if (index >= enemies_.size()) return nullptr;
    return enemies_[index].get();
}

bool BattleEncounter::hasAliveEnemies() const {
    for (const auto& enemy : enemies_) {
        if (enemy->isAlive()) {
            return true;
        }
    }
    return false;
}

int BattleEncounter::getAliveCount() const {
    int count = 0;
    for (const auto& enemy : enemies_) {
        if (enemy->isAlive()) {
            count++;
        }
    }
    return count;
}

int BattleEncounter::getTotalCount() const {
    return static_cast<int>(enemies_.size());
}

Monster* BattleEncounter::getLowestHPEnemy() {
    Monster* target = nullptr;
    float lowestPercent = 2.0f; // Start above 100%

    for (auto& enemy : enemies_) {
        if (!enemy->isAlive()) continue;

        float hpPercent = static_cast<float>(enemy->health) / enemy->maxHealth;
        if (hpPercent < lowestPercent) {
            lowestPercent = hpPercent;
            target = enemy.get();
        }
    }

    return target;
}

int BattleEncounter::getTotalMoneyReward() const {
    int total = 0;
    for (const auto& enemy : enemies_) {
        total += enemy->moneyReward;
    }
    return total;
}

std::vector<std::string> BattleEncounter::getAllItemRewards() const {
    std::vector<std::string> rewards;
    for (const auto& enemy : enemies_) {
        if (!enemy->itemReward.empty() && enemy->type == MonsterType::MOB) {
            rewards.push_back(enemy->itemReward);
        }
    }
    return rewards;
}

std::vector<Equipment> BattleEncounter::getAllEquipmentDrops() const {
    std::vector<Equipment> drops;
    // Equipment drops handled separately in victory handler
    return drops;
}

bool BattleEncounter::hasBoss() const {
    for (const auto& enemy : enemies_) {
        if (enemy->type == MonsterType::BOSS ||
            enemy->type == MonsterType::NEMESIS ||
            enemy->type == MonsterType::REVIEW) {
            return true;
        }
    }
    return false;
}

bool BattleEncounter::hasSpecialEnemy() const {
    for (const auto& enemy : enemies_) {
        if (enemy->type == MonsterType::NEMESIS ||
            enemy->name.find("Shadow:") != std::string::npos ||
            enemy->name.find("Safety Manager") != std::string::npos) {
            return true;
        }
    }
    return false;
}

void BattleEncounter::clear() {
    enemies_.clear();
}