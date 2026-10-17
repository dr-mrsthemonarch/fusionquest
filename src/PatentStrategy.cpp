//
// Created by drmrsthemonarch on 22.11.25.
//

#include "../include/PatentStrategy.h"
#include "../include/Monster.h"
#include <algorithm>
#include <iostream>

// ==================== AggressiveStrategy ====================

std::shared_ptr<Patent> AggressiveStrategy::selectPatent(
    Monster* caster,
    CombatEntity* target,
    std::vector<std::shared_ptr<Patent>>& availablePatents
) {
    auto affordable = getAffordablePatents(caster->getHypeEnergy(), availablePatents);
    if (affordable.empty()) return nullptr;

    // Prioritize attack patents (80% of the time)
    std::uniform_int_distribution<> strategyDist(0, 99);
    if (strategyDist(rng_) < 80) {
        auto attacks = getPatentsByType(PatentType::ATTACK, affordable);
        if (!attacks.empty()) {
            std::uniform_int_distribution<> selectDist(0, attacks.size() - 1);
            return attacks[selectDist(rng_)];
        }
    }

    // If low health (<30%), consider healing
    if (caster->getRunway() < caster->getMaxRunway() * 0.3) {
        auto heals = getPatentsByType(PatentType::HEAL, affordable);
        if (!heals.empty()) {
            std::uniform_int_distribution<> selectDist(0, heals.size() - 1);
            return heals[selectDist(rng_)];
        }
    }

    // Fallback: any affordable patent
    std::uniform_int_distribution<> selectDist(0, affordable.size() - 1);
    return affordable[selectDist(rng_)];
}

bool AggressiveStrategy::shouldUsePatent(Monster* caster, CombatEntity* target) {
    // Use patents frequently (40% chance each turn if energy available)
    if (caster->getHypeEnergy() < 10) return false;

    std::uniform_int_distribution<> chanceDist(0, 99);
    return chanceDist(rng_) < 40;
}

// ==================== BalancedStrategy ====================

std::shared_ptr<Patent> BalancedStrategy::selectPatent(
    Monster* caster,
    CombatEntity* target,
    std::vector<std::shared_ptr<Patent>>& availablePatents
) {
    auto affordable = getAffordablePatents(caster->getHypeEnergy(), availablePatents);
    if (affordable.empty()) return nullptr;

    float healthPercent = static_cast<float>(caster->getRunway()) / caster->getMaxRunway();

    // Critical health (<25%): HEAL
    if (healthPercent < 0.25f) {
        auto heals = getPatentsByType(PatentType::HEAL, affordable);
        if (!heals.empty()) {
            std::uniform_int_distribution<> selectDist(0, heals.size() - 1);
            return heals[selectDist(rng_)];
        }
    }

    // Low health (<50%): DEFENSE
    if (healthPercent < 0.5f) {
        auto defenses = getPatentsByType(PatentType::DEFENSE, affordable);
        if (!defenses.empty()) {
            std::uniform_int_distribution<> selectDist(0, defenses.size() - 1);
            return defenses[selectDist(rng_)];
        }
    }

    // Healthy (>70%): Attack or Buff
    if (healthPercent > 0.7f) {
        std::uniform_int_distribution<> strategyDist(0, 99);
        if (strategyDist(rng_) < 60) {
            // 60% attack
            auto attacks = getPatentsByType(PatentType::ATTACK, affordable);
            if (!attacks.empty()) {
                std::uniform_int_distribution<> selectDist(0, attacks.size() - 1);
                return attacks[selectDist(rng_)];
            }
        } else {
            // 40% buff self
            auto buffs = getPatentsByType(PatentType::BUFF, affordable);
            if (!buffs.empty()) {
                std::uniform_int_distribution<> selectDist(0, buffs.size() - 1);
                return buffs[selectDist(rng_)];
            }
        }
    }

    // Medium health: Mix of all
    std::uniform_int_distribution<> selectDist(0, affordable.size() - 1);
    return affordable[selectDist(rng_)];
}

bool BalancedStrategy::shouldUsePatent(Monster* caster, CombatEntity* target) {
    if (caster->getHypeEnergy() < 8) return false;

    // Use more frequently when health is low
    float healthPercent = static_cast<float>(caster->getRunway()) / caster->getMaxRunway();
    int baseChance = 30;
    if (healthPercent < 0.3f) baseChance = 70; // Much more likely when low
    else if (healthPercent < 0.5f) baseChance = 50;

    std::uniform_int_distribution<> chanceDist(0, 99);
    return chanceDist(rng_) < baseChance;
}

// ==================== DebuffStrategy ====================

std::shared_ptr<Patent> DebuffStrategy::selectPatent(
    Monster* caster,
    CombatEntity* target,
    std::vector<std::shared_ptr<Patent>>& availablePatents
) {
    auto affordable = getAffordablePatents(caster->getHypeEnergy(), availablePatents);
    if (affordable.empty()) return nullptr;

    // Priority 1: Debuff enemy (70% of the time)
    std::uniform_int_distribution<> strategyDist(0, 99);
    if (strategyDist(rng_) < 70) {
        auto debuffs = getPatentsByType(PatentType::DEBUFF, affordable);
        if (!debuffs.empty()) {
            std::uniform_int_distribution<> selectDist(0, debuffs.size() - 1);
            return debuffs[selectDist(rng_)];
        }
    }

    // Priority 2: Defense if health low (<40%)
    if (caster->getRunway() < caster->getMaxRunway() * 0.4) {
        auto defenses = getPatentsByType(PatentType::DEFENSE, affordable);
        if (!defenses.empty()) {
            std::uniform_int_distribution<> selectDist(0, defenses.size() - 1);
            return defenses[selectDist(rng_)];
        }
    }

    // Priority 3: Attack
    auto attacks = getPatentsByType(PatentType::ATTACK, affordable);
    if (!attacks.empty()) {
        std::uniform_int_distribution<> selectDist(0, attacks.size() - 1);
        return attacks[selectDist(rng_)];
    }

    // Fallback
    std::uniform_int_distribution<> selectDist(0, affordable.size() - 1);
    return affordable[selectDist(rng_)];
}

bool DebuffStrategy::shouldUsePatent(Monster* caster, CombatEntity* target) {
    if (caster->getHypeEnergy() < 8) return false;

    // Debuffers use patents frequently (50% chance)
    std::uniform_int_distribution<> chanceDist(0, 99);
    return chanceDist(rng_) < 50;
}

// ==================== TacticalStrategy ====================

std::shared_ptr<Patent> TacticalStrategy::selectPatent(
    Monster* caster,
    CombatEntity* target,
    std::vector<std::shared_ptr<Patent>>& availablePatents
) {
    auto affordable = getAffordablePatents(caster->getHypeEnergy(), availablePatents);
    if (affordable.empty()) return nullptr;

    turnsActive_++;

    float healthPercent = static_cast<float>(caster->getRunway()) / caster->getMaxRunway();

    // Emergency healing
    if (healthPercent < 0.25f) {
        auto heals = getPatentsByType(PatentType::HEAL, affordable);
        if (!heals.empty()) {
            std::uniform_int_distribution<> selectDist(0, heals.size() - 1);
            return heals[selectDist(rng_)];
        }
    }

    // Opening move: Buff self if haven't yet
    if (turnsActive_ <= 2 && !hasBuffedSelf_) {
        auto buffs = getPatentsByType(PatentType::BUFF, affordable);
        if (!buffs.empty()) {
            hasBuffedSelf_ = true;
            std::uniform_int_distribution<> selectDist(0, buffs.size() - 1);
            return buffs[selectDist(rng_)];
        }
    }

    // Second move: Debuff enemy if haven't yet
    if (turnsActive_ <= 4 && !hasDebuffedEnemy_) {
        auto debuffs = getPatentsByType(PatentType::DEBUFF, affordable);
        if (!debuffs.empty()) {
            hasDebuffedEnemy_ = true;
            std::uniform_int_distribution<> selectDist(0, debuffs.size() - 1);
            return debuffs[selectDist(rng_)];
        }
    }

    // After setup: Weighted selection
    std::vector<std::pair<PatentType, int>> weights = {
        {PatentType::ATTACK, 40},
        {PatentType::DEBUFF, 30},
        {PatentType::BUFF, 15},
        {PatentType::DEFENSE, 10},
        {PatentType::HEAL, 5}
    };

    std::uniform_int_distribution<> weightDist(0, 99);
    int roll = weightDist(rng_);
    int cumulative = 0;

    for (const auto& [type, weight] : weights) {
        cumulative += weight;
        if (roll < cumulative) {
            auto patents = getPatentsByType(type, affordable);
            if (!patents.empty()) {
                std::uniform_int_distribution<> selectDist(0, patents.size() - 1);
                return patents[selectDist(rng_)];
            }
        }
    }

    // Fallback
    std::uniform_int_distribution<> selectDist(0, affordable.size() - 1);
    return affordable[selectDist(rng_)];
}

bool TacticalStrategy::shouldUsePatent(Monster* caster, CombatEntity* target) {
    if (caster->getHypeEnergy() < 12) return false;

    // Tactical enemies use patents very frequently (60% chance)
    // Always use in opening turns
    if (turnsActive_ <= 3) return true;

    std::uniform_int_distribution<> chanceDist(0, 99);
    return chanceDist(rng_) < 60;
}

// ==================== MirrorStrategy ====================

std::shared_ptr<Patent> MirrorStrategy::selectPatent(
    Monster* caster,
    CombatEntity* target,
    std::vector<std::shared_ptr<Patent>>& availablePatents
) {
    auto affordable = getAffordablePatents(caster->getHypeEnergy(), availablePatents);
    if (affordable.empty()) return nullptr;

    // Shadow player mirrors player behavior intelligently
    float healthPercent = static_cast<float>(caster->getRunway()) / caster->getMaxRunway();
    float targetHealthPercent = static_cast<float>(target->getRunway()) / target->getMaxRunway();

    // If we're low, prioritize survival
    if (healthPercent < 0.3f) {
        auto heals = getPatentsByType(PatentType::HEAL, affordable);
        if (!heals.empty()) {
            std::uniform_int_distribution<> selectDist(0, heals.size() - 1);
            return heals[selectDist(rng_)];
        }
        auto defenses = getPatentsByType(PatentType::DEFENSE, affordable);
        if (!defenses.empty()) {
            std::uniform_int_distribution<> selectDist(0, defenses.size() - 1);
            return defenses[selectDist(rng_)];
        }
    }

    // If target is low, go for the kill
    if (targetHealthPercent < 0.3f) {
        auto attacks = getPatentsByType(PatentType::ATTACK, affordable);
        if (!attacks.empty()) {
            std::uniform_int_distribution<> selectDist(0, attacks.size() - 1);
            return attacks[selectDist(rng_)];
        }
    }

    // Otherwise, use a balanced mix
    std::vector<std::pair<PatentType, int>> weights = {
        {PatentType::ATTACK, 30},
        {PatentType::DEBUFF, 25},
        {PatentType::BUFF, 20},
        {PatentType::DEFENSE, 15},
        {PatentType::HEAL, 10}
    };

    std::uniform_int_distribution<> weightDist(0, 99);
    int roll = weightDist(rng_);
    int cumulative = 0;

    for (const auto& [type, weight] : weights) {
        cumulative += weight;
        if (roll < cumulative) {
            auto patents = getPatentsByType(type, affordable);
            if (!patents.empty()) {
                std::uniform_int_distribution<> selectDist(0, patents.size() - 1);
                return patents[selectDist(rng_)];
            }
        }
    }

    // Fallback: random patent
    std::uniform_int_distribution<> selectDist(0, affordable.size() - 1);
    return affordable[selectDist(rng_)];
}

bool MirrorStrategy::shouldUsePatent(Monster* caster, CombatEntity* target) {
    if (caster->getHypeEnergy() < 10) return false;

    // Mirror uses patents very frequently (70% chance)
    // This makes shadow player extremely dangerous
    std::uniform_int_distribution<> chanceDist(0, 99);
    return chanceDist(rng_) < 70;
}

// ==================== DefensiveStrategy ====================

std::shared_ptr<Patent> DefensiveStrategy::selectPatent(
    Monster* caster,
    CombatEntity* target,
    std::vector<std::shared_ptr<Patent>>& availablePatents
) {
    auto affordable = getAffordablePatents(caster->getHypeEnergy(), availablePatents);
    if (affordable.empty()) return nullptr;

    float healthPercent = static_cast<float>(caster->getRunway()) / caster->getMaxRunway();

    // Very low health: HEAL
    if (healthPercent < 0.4f) {
        auto heals = getPatentsByType(PatentType::HEAL, affordable);
        if (!heals.empty()) {
            std::uniform_int_distribution<> selectDist(0, heals.size() - 1);
            return heals[selectDist(rng_)];
        }
    }

    // Medium health: DEFENSE
    if (healthPercent < 0.7f) {
        auto defenses = getPatentsByType(PatentType::DEFENSE, affordable);
        if (!defenses.empty()) {
            std::uniform_int_distribution<> selectDist(0, defenses.size() - 1);
            return defenses[selectDist(rng_)];
        }
    }

    // Healthy: BUFF or occasional attack
    std::uniform_int_distribution<> strategyDist(0, 99);
    if (strategyDist(rng_) < 30) {
        auto attacks = getPatentsByType(PatentType::ATTACK, affordable);
        if (!attacks.empty()) {
            std::uniform_int_distribution<> selectDist(0, attacks.size() - 1);
            return attacks[selectDist(rng_)];
        }
    }

    auto buffs = getPatentsByType(PatentType::BUFF, affordable);
    if (!buffs.empty()) {
        std::uniform_int_distribution<> selectDist(0, buffs.size() - 1);
        return buffs[selectDist(rng_)];
    }

    // Fallback
    std::uniform_int_distribution<> selectDist(0, affordable.size() - 1);
    return affordable[selectDist(rng_)];
}

bool DefensiveStrategy::shouldUsePatent(Monster* caster, CombatEntity* target) {
    if (caster->getHypeEnergy() < 8) return false;

    float healthPercent = static_cast<float>(caster->getRunway()) / caster->getMaxRunway();

    // Use very frequently when injured
    if (healthPercent < 0.5f) {
        std::uniform_int_distribution<> chanceDist(0, 99);
        return chanceDist(rng_) < 80;
    }

    // Normal usage when healthy
    std::uniform_int_distribution<> chanceDist(0, 99);
    return chanceDist(rng_) < 35;
}