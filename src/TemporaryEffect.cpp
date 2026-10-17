//
// Created by drmrsthemonarch on 20.11.25.
//

#include "../include/TemporaryEffect.h"
#include <algorithm>
#include <sstream>

std::string TemporaryEffectsManager::getDisplayColor() const {
    if (effects_.empty()) return "#FFFFFF";

    const TemporaryEffect* highestPriority = nullptr;
    int maxPriority = -1;

    for (const auto& effect : effects_) {
        int priority = effect.getColorPriority();
        if (priority > maxPriority) {
            maxPriority = priority;
            highestPriority = &effect;
        }
    }

    return highestPriority ? highestPriority->getEffectColor() : "#FFFFFF";
}

TemporaryEffect::TemporaryEffect(EffectType type, int magnitude, int duration, const std::string& sourceName)
    : type_(type), magnitude_(magnitude), remainingDuration_(duration), sourceName_(sourceName) {
}

std::string TemporaryEffect::getDescription() const {
    std::stringstream ss;

    switch (type_) {
        case EffectType::HUSTLE_BOOST:
            ss << "Hustle +" << magnitude_;
            break;
        case EffectType::HUSTLE_REDUCTION:
            ss << "Hustle -" << magnitude_;
            break;
        case EffectType::AGILITY_BOOST:
            ss << "Agile-ity +" << magnitude_;
            break;
        case EffectType::AGILITY_REDUCTION:
            ss << "Agile-ity -" << magnitude_;
            break;
        case EffectType::NDAS_BOOST:
            ss << "NDAs +" << magnitude_;
            break;
        case EffectType::NDAS_REDUCTION:
            ss << "NDAs -" << magnitude_;
            break;
        case EffectType::LUCK_BOOST:
            ss << "Luck +" << magnitude_;
            break;
        case EffectType::LUCK_REDUCTION:
            ss << "Luck -" << magnitude_;
            break;
        case EffectType::SYNERGY_BOOST:
            ss << "Synergy +" << magnitude_;
            break;
        case EffectType::TD_BOOST:
            ss << "Technical Debt +" << magnitude_;
            break;
        case EffectType::SYNERGY_REDUCTION:
            ss << "Synergy -" << magnitude_;
            break;
        case EffectType::DEFENSE_SHIELD:
            ss << "Defense Shield +" << magnitude_;
            break;
        case EffectType::DAMAGE_OVER_TIME:
            ss << "DoT -" << magnitude_ << " HP/turn";
            break;
        case EffectType::REGENERATION:
            ss << "Regen +" << magnitude_ << " HP/turn";
            break;
    }

    ss << " (" << remainingDuration_ << " turns)";
    return ss.str();
}

std::string TemporaryEffect::getEffectIcon() const {
    switch (type_) {
        case EffectType::HUSTLE_BOOST: return "⚔️↑";
        case EffectType::HUSTLE_REDUCTION: return "⚔️↓";
        case EffectType::AGILITY_BOOST: return "🔄↑";
        case EffectType::AGILITY_REDUCTION: return "🔄↓";
        case EffectType::NDAS_BOOST: return "🛡️↑";
        case EffectType::NDAS_REDUCTION: return "🛡️↓";
        case EffectType::LUCK_BOOST: return "🍀↑";
        case EffectType::LUCK_REDUCTION: return "🍀↓";
        case EffectType::SYNERGY_BOOST: return "🤝↑";
        case EffectType::SYNERGY_REDUCTION: return "🤝↓";
        case EffectType::DEFENSE_SHIELD: return "🛡️✨";
        case EffectType::DAMAGE_OVER_TIME: return "💀";
        case EffectType::REGENERATION: return "💚";
        case EffectType::TD_BOOST: return "x";
        default: return "✨";
    }
}

std::string TemporaryEffect::getEffectColor() const {
    switch (type_) {
        case EffectType::DAMAGE_OVER_TIME:
            return "#FF9800"; // Orange
        case EffectType::HUSTLE_REDUCTION:
        case EffectType::AGILITY_REDUCTION:
        case EffectType::NDAS_REDUCTION:
        case EffectType::LUCK_REDUCTION:
        case EffectType::SYNERGY_REDUCTION:
            return "#F44336"; // Red
        case EffectType::DEFENSE_SHIELD:
            return "#9C27B0"; // Purple
        case EffectType::HUSTLE_BOOST:
        case EffectType::AGILITY_BOOST:
        case EffectType::NDAS_BOOST:
        case EffectType::LUCK_BOOST:
        case EffectType::SYNERGY_BOOST:
            return "#4CAF50"; // Green
        case EffectType::REGENERATION:
            return "#00BCD4"; // Cyan
        case EffectType::TD_BOOST:
            return "#FFEB3B"; // Yellow
        default:
            return "#FFFFFF"; // White
    }
}

int TemporaryEffect::getColorPriority() const {
    switch (type_) {
        case EffectType::DAMAGE_OVER_TIME:
            return 5; // Highest priority
        case EffectType::HUSTLE_REDUCTION:
        case EffectType::AGILITY_REDUCTION:
        case EffectType::NDAS_REDUCTION:
        case EffectType::LUCK_REDUCTION:
        case EffectType::SYNERGY_REDUCTION:
            return 4;
        case EffectType::DEFENSE_SHIELD:
            return 3;
        case EffectType::HUSTLE_BOOST:
        case EffectType::AGILITY_BOOST:
        case EffectType::NDAS_BOOST:
        case EffectType::LUCK_BOOST:
        case EffectType::SYNERGY_BOOST:
            return 2;
        case EffectType::REGENERATION:
            return 1;
        default:
            return 0;
    }
}

void TemporaryEffectsManager::addEffect(const TemporaryEffect& effect) {
    effects_.push_back(effect);
}

void TemporaryEffectsManager::clearExpiredEffects() {
    effects_.erase(
        std::remove_if(effects_.begin(), effects_.end(),
            [](const TemporaryEffect& effect) { return effect.isExpired(); }),
        effects_.end()
    );
}

void TemporaryEffectsManager::clearAllEffects() {
    effects_.clear();
}

int TemporaryEffectsManager::getTotalModifier(EffectType boostType, EffectType reductionType) const {
    int total = 0;

    for (const auto& effect : effects_) {
        if (effect.getType() == boostType) {
            total += effect.getMagnitude();
        } else if (effect.getType() == reductionType) {
            total -= effect.getMagnitude();
        }
    }

    return total;
}

int TemporaryEffectsManager::getHustleModifier() const {
    return getTotalModifier(EffectType::HUSTLE_BOOST, EffectType::HUSTLE_REDUCTION);
}

int TemporaryEffectsManager::getAgilityModifier() const {
    return getTotalModifier(EffectType::AGILITY_BOOST, EffectType::AGILITY_REDUCTION);
}

int TemporaryEffectsManager::getNdasModifier() const {
    return getTotalModifier(EffectType::NDAS_BOOST, EffectType::NDAS_REDUCTION);
}

int TemporaryEffectsManager::getLuckModifier() const {
    return getTotalModifier(EffectType::LUCK_BOOST, EffectType::LUCK_REDUCTION);
}

int TemporaryEffectsManager::getSynergyModifier() const {
    return getTotalModifier(EffectType::SYNERGY_BOOST, EffectType::SYNERGY_REDUCTION);
}

int TemporaryEffectsManager::getTechnicalDebtModifier() const {
    return getTotalModifier(EffectType::TD_BOOST, EffectType::TD_BOOST);
}

int TemporaryEffectsManager::getDefenseShield() const {
    int total = 0;
    for (const auto& effect : effects_) {
        if (effect.getType() == EffectType::DEFENSE_SHIELD) {
            total += effect.getMagnitude();
        }
    }
    return total;
}

int TemporaryEffectsManager::processDamageOverTime() const {
    int totalDamage = 0;
    for (const auto& effect : effects_) {
        if (effect.getType() == EffectType::DAMAGE_OVER_TIME) {
            totalDamage += effect.getMagnitude();
        }
    }
    return totalDamage;
}

int TemporaryEffectsManager::processRegeneration() const {
    int totalRegen = 0;
    for (const auto& effect : effects_) {
        if (effect.getType() == EffectType::REGENERATION) {
            totalRegen += effect.getMagnitude();
        }
    }
    return totalRegen;
}

void TemporaryEffectsManager::processTurn() {
    // Decrement all effect durations
    for (auto& effect : effects_) {
        effect.decrementDuration();
    }

    // Remove expired effects
    clearExpiredEffects();
}

bool TemporaryEffectsManager::hasEffect(EffectType type) const {
    return std::any_of(effects_.begin(), effects_.end(),
        [type](const TemporaryEffect& effect) {
            return effect.getType() == type;
        });
}

std::string TemporaryEffectsManager::getEffectsSummary() const {
    if (effects_.empty()) {
        return "No active effects";
    }

    std::stringstream ss;
    ss << "Active Effects (" << effects_.size() << "):\n";

    for (const auto& effect : effects_) {
        ss << "  " << effect.getEffectIcon() << " "
           << effect.getDescription()
           << " [" << effect.getSourceName() << "]\n";
    }

    return ss.str();
}