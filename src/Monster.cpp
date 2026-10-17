// SPG Engine made by drmrsthemonarch
#include "../include/Monster.h"
#include "../include/Patent.h"
#include <algorithm>
#include <iostream>
#include "../include/PatentStrategy.h"

Monster::Monster()
    : health(0), maxHealth(0), attack(0), defense(0), speed(0),
      moneyReward(0), hypeEnergy(0), maxHypeEnergy(0), strategy_(nullptr) {
}

// ADD: Explicit destructor
Monster::~Monster() = default;

void Monster::takeDamage(int damage) {
    int defenseShield = effectsManager_.getDefenseShield();
    int actualDamage = std::max(1, damage - defenseShield);

    if (defenseShield > 0) {
        std::cout << name << "'s defense shield absorbs " << (damage - actualDamage) << " damage!\n";
    }

    health = std::max(0, health - actualDamage);
}

int Monster::calculateDamage() const {
    return static_cast<int>(attack * (0.8 + (rand() % 41) / 100.0));
}

std::string Monster::getDifficultyString() const {
  switch (difficulty) {
    case MonsterDifficulty::EASY:      return "Easy";
    case MonsterDifficulty::MEDIUM:    return "Medium";
    case MonsterDifficulty::HARD:      return "Hard";
    case MonsterDifficulty::LEGENDARY: return "Legendary";
    default: return "Unknown";
  }
}

void Monster::addPatent(const std::shared_ptr<Patent>& patent) {
    patents.push_back(patent);
}

void Monster::setStrategy(std::unique_ptr<PatentStrategy> strategy) {
    strategy_ = std::move(strategy);
}

// NEW: Strategic patent usage
bool Monster::useStrategicPatent(CombatEntity* target) {
    if (!canUsePatents() || !strategy_) {
        return false; // No patents or no strategy
    }

    // Ask strategy if we should use a patent this turn
    if (!strategy_->shouldUsePatent(this, target)) {
        return false;
    }

    // Let strategy select the best patent
    auto selectedPatent = strategy_->selectPatent(this, target, patents);

    if (!selectedPatent) {
        return false; // Strategy couldn't find suitable patent
    }

    // Use the selected patent
    if (hypeEnergy >= selectedPatent->getCost()) {
        // Determine target based on patent type
        CombatEntity* patentTarget = target;
        if (selectedPatent->getTarget() == PatentTarget::SELF) {
            patentTarget = this;
        }

        selectedPatent->applyEffect(this, patentTarget);
        std::cout << name << " strategically uses " << selectedPatent->getFullName() << "!\n";
        return true;
    }

    return false;
}

// OLD: Keep for backward compatibility but prefer useStrategicPatent
bool Monster::useRandomPatent(CombatEntity* target) {
    if (patents.empty() || hypeEnergy <= 0) return false;

    // Find affordable patents
    std::vector<std::shared_ptr<Patent>> affordablePatents;
    for (const auto& patent : patents) {
        if (hypeEnergy >= patent->getCost()) {
            affordablePatents.push_back(patent);
        }
    }

    if (affordablePatents.empty()) return false;

    // Select random affordable patent
    int randomIndex = rand() % affordablePatents.size();
    auto& patent = affordablePatents[randomIndex];

    // Use the patent
    hypeEnergy -= patent->getCost();

    // Apply patent effect based on type
    switch (patent->getType()) {
        case PatentType::ATTACK:
            if (target) {
                int damage = patent->getCurrentPower() + (attack / 2);
                target->takeDamage(damage);
            }
            break;
        case PatentType::HEAL:
            if (this == target) {
                int healAmount = patent->getCurrentPower();
                health = std::min(maxHealth, health + healAmount);
            }
            break;
        // Add other patent types as needed
        default:
            break;
    }

    return true;
}

void Monster::addTemporaryEffect(const TemporaryEffect& effect) {
    effectsManager_.addEffect(effect);
    std::cout << name << " gained effect: " << effect.getDescription() << "\n";
}

void Monster::processTurnEffects() {
    // Process damage over time
    int dotDamage = effectsManager_.processDamageOverTime();
    if (dotDamage > 0) {
        health = std::max(0, health - dotDamage);
        std::cout << name << " takes " << dotDamage << " damage from DoT effects\n";
    }

    // Process regeneration
    int regenAmount = effectsManager_.processRegeneration();
    if (regenAmount > 0) {
        int oldHealth = health;
        health = std::min(maxHealth, health + regenAmount);
        int actualRegen = health - oldHealth;
        if (actualRegen > 0) {
            std::cout << name << " regenerates " << actualRegen << " health\n";
        }
    }

    // Tick down effect durations and remove expired effects
    effectsManager_.processTurn();
}

int Monster::getModifiedHustle() const {
    return std::max(1, attack + effectsManager_.getHustleModifier());
}

int Monster::getModifiedAgility() const {
    return std::max(1, speed + effectsManager_.getAgilityModifier());
}

int Monster::getModifiedNDAs() const {
    return std::max(0, defense + effectsManager_.getNdasModifier() + effectsManager_.getDefenseShield());
}

int Monster::getModifiedLuck() const {
    return std::max(1, effectsManager_.getLuckModifier());
}

int Monster::getModifiedSynergy() const {
    return std::max(1, effectsManager_.getSynergyModifier());
}

