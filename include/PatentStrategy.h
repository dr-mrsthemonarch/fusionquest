//
// Created by drmrsthemonarch on 22.11.25.
//

#ifndef PATENTSTRATEGY_H
#define PATENTSTRATEGY_H

#include "Patent.h"
#include "Monster.h"
#include "CombatEntity.h"
#include <memory>
#include <vector>
#include <random>

// Forward declarations
class Monster;
class CombatEntity;


class PatentStrategy {
public:
    virtual ~PatentStrategy() = default;

    virtual std::shared_ptr<Patent> selectPatent(
        Monster* caster,
        CombatEntity* target,
        std::vector<std::shared_ptr<Patent>>& availablePatents
    ) = 0;


    virtual bool shouldUsePatent(Monster* caster, CombatEntity* target) = 0;

protected:
    std::mt19937 rng_{std::random_device{}()};

    // Helper: Get affordable patents
    std::vector<std::shared_ptr<Patent>> getAffordablePatents(
        int currentHype,
        const std::vector<std::shared_ptr<Patent>>& patents
    ) {
        std::vector<std::shared_ptr<Patent>> affordable;
        for (const auto& patent : patents) {
            if (currentHype >= patent->getCost()) {
                affordable.push_back(patent);
            }
        }
        return affordable;
    }

    // Helper: Get patents by type
    std::vector<std::shared_ptr<Patent>> getPatentsByType(
        PatentType type,
        const std::vector<std::shared_ptr<Patent>>& patents
    ) {
        std::vector<std::shared_ptr<Patent>> result;
        for (const auto& patent : patents) {
            if (patent->getType() == type) {
                result.push_back(patent);
            }
        }
        return result;
    }
};

class AggressiveStrategy : public PatentStrategy {
public:
    std::shared_ptr<Patent> selectPatent(
        Monster* caster,
        CombatEntity* target,
        std::vector<std::shared_ptr<Patent>>& availablePatents
    ) override;

    bool shouldUsePatent(Monster* caster, CombatEntity* target) override;
};


class BalancedStrategy : public PatentStrategy {
public:
    std::shared_ptr<Patent> selectPatent(
        Monster* caster,
        CombatEntity* target,
        std::vector<std::shared_ptr<Patent>>& availablePatents
    ) override;

    bool shouldUsePatent(Monster* caster, CombatEntity* target) override;
};


class DebuffStrategy : public PatentStrategy {
public:
    std::shared_ptr<Patent> selectPatent(
        Monster* caster,
        CombatEntity* target,
        std::vector<std::shared_ptr<Patent>>& availablePatents
    ) override;

    bool shouldUsePatent(Monster* caster, CombatEntity* target) override;
};


class TacticalStrategy : public PatentStrategy {
public:
    std::shared_ptr<Patent> selectPatent(
        Monster* caster,
        CombatEntity* target,
        std::vector<std::shared_ptr<Patent>>& availablePatents
    ) override;

    bool shouldUsePatent(Monster* caster, CombatEntity* target) override;

private:
    // Track what we've used to avoid repetition
    int turnsActive_ = 0;
    bool hasBuffedSelf_ = false;
    bool hasDebuffedEnemy_ = false;
};


class MirrorStrategy : public PatentStrategy {
public:
    std::shared_ptr<Patent> selectPatent(
        Monster* caster,
        CombatEntity* target,
        std::vector<std::shared_ptr<Patent>>& availablePatents
    ) override;

    bool shouldUsePatent(Monster* caster, CombatEntity* target) override;
};


class DefensiveStrategy : public PatentStrategy {
public:
    std::shared_ptr<Patent> selectPatent(
        Monster* caster,
        CombatEntity* target,
        std::vector<std::shared_ptr<Patent>>& availablePatents
    ) override;

    bool shouldUsePatent(Monster* caster, CombatEntity* target) override;
};

#endif // PATENTSTRATEGY_H
