//
// Created by drmrsthemonarch
//

#ifndef MONSTER_H
#define MONSTER_H

#include "CombatEntity.h"
#include <string>
#include <vector>
#include <memory>
#include "Equipment.h"
#include "CombatEntity.h"
#include "TemporaryEffect.h"
class Patent;
class PatentStrategy;

enum class MonsterType {
  MOB,
  BOSS,
  NEMESIS,
  REVIEW
};

enum class MonsterDifficulty {
  EASY,
  MEDIUM,
  HARD,
  LEGENDARY
};

class Patent; // Forward declaration

class Monster : public CombatEntity {
public:
  Monster();
  ~Monster() override;

  Monster(const Monster&) = delete;
  Monster& operator=(const Monster&) = delete;

  // ADD: Allow move operations
  Monster(Monster&&) noexcept = default;
  Monster& operator=(Monster&&) noexcept = default;

  MonsterType type;
  MonsterDifficulty difficulty;
  std::string name;
  int level;

  // Core stats (from CombatEntity interface)
  int health;
  int maxHealth;
  int attack;
  int defense;
  int speed;
  int moneyReward;
  std::string itemReward;
  int itemValue;

  // Patent system for monsters
  // std::vector<std::shared_ptr<Patent>> patents;
  int hypeEnergy;
  int maxHypeEnergy;
  int getMaxHypeEnergy() const override { return maxHypeEnergy; }
  int getModifiedHustle() const override;
  int getModifiedAgility() const override;
  int getModifiedNDAs() const override;
  int getModifiedLuck() const override;
  int getModifiedSynergy() const override;


  // CombatEntity interface implementation
  int getRunway() const override { return health; }
  int getMaxRunway() const override { return maxHealth; }
  void setRunway(int runway) override { health = runway; }
  int getHypeEnergy() const override { return hypeEnergy; }
  void setHypeEnergy(int energy) override { hypeEnergy = std::min(energy, maxHypeEnergy); }
  int getHustle() const override { return attack; }
  int getNDAs() const override { return defense; }
  int getAgility() const override { return speed; }
  int getLuck() const override { return level; } // Monsters use level as luck
  int getSynergy() const override { return level / 2; } // Monsters use half level as synergy

  void takeDamage(int damage) override;
  bool isAlive() const override { return health > 0; }
  std::string getName() const override { return name; }

  bool canUsePatents() const override { return !patents.empty() && hypeEnergy > 0; }
  void addPatent(const std::shared_ptr<Patent>& patent);
  bool useRandomPatent(CombatEntity* target); // OLD: Simple random
  bool useStrategicPatent(CombatEntity* target); // NEW: Strategic patent use
  // Patent strategy
  void setStrategy(std::unique_ptr<PatentStrategy> strategy);
  PatentStrategy* getStrategy() const { return strategy_.get(); }

  // Temporary effects
  void addTemporaryEffect(const TemporaryEffect& effect) override;
  void processTurnEffects() override;
  TemporaryEffectsManager& getEffectsManager() override { return effectsManager_; }
  // Monster-specific methods
  int calculateDamage() const;
  std::string getDifficultyString() const;


private:
  std::vector<std::shared_ptr<Patent>> patents;
  std::unique_ptr<PatentStrategy> strategy_;
  TemporaryEffectsManager effectsManager_;
};

#endif