//
// Created by drmrsthemonarch on 19.11.25.
//

#ifndef COMBATENTITY_H
#define COMBATENTITY_H

#include <string>

#include "TemporaryEffect.h"

class CombatEntity {
public:
    virtual ~CombatEntity() = default;

    // Core stats
    virtual int getRunway() const = 0;
    virtual int getMaxRunway() const = 0;
    virtual void setRunway(int runway) = 0;
    virtual int getHypeEnergy() const = 0;
    virtual int getMaxHypeEnergy() const = 0;  // ADDED
    virtual void setHypeEnergy(int energy) = 0;
    virtual int getHustle() const = 0;
    virtual int getNDAs() const = 0;
    virtual int getAgility() const = 0;
    virtual int getLuck() const = 0;
    virtual int getSynergy() const = 0;

    // Modified stats (with temporary effects applied)
    virtual int getModifiedHustle() const = 0;      // ADDED
    virtual int getModifiedAgility() const = 0;     // ADDED
    virtual int getModifiedNDAs() const = 0;        // ADDED
    virtual int getModifiedLuck() const = 0;        // ADDED
    virtual int getModifiedSynergy() const = 0;     // ADDED

    // Combat actions
    virtual void takeDamage(int damage) = 0;
    virtual bool isAlive() const = 0;
    virtual std::string getName() const = 0;

    // Patent system
    virtual bool canUsePatents() const = 0;

    virtual void addTemporaryEffect(const TemporaryEffect& effect) = 0;
    virtual void processTurnEffects() = 0;
    virtual TemporaryEffectsManager& getEffectsManager() = 0;
};

#endif