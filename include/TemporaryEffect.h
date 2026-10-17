//
// Created by drmrsthemonarch on 20.11.25.
//

#ifndef TEMPORARYEFFECT_H
#define TEMPORARYEFFECT_H

#include <string>
#include <vector>
#include <memory>

enum class EffectType {
    HUSTLE_BOOST,
    HUSTLE_REDUCTION,
    AGILITY_BOOST,
    AGILITY_REDUCTION,
    NDAS_BOOST,
    NDAS_REDUCTION,
    LUCK_BOOST,
    LUCK_REDUCTION,
    SYNERGY_BOOST,
    SYNERGY_REDUCTION,
    DEFENSE_SHIELD,
    DAMAGE_OVER_TIME,
    REGENERATION,
    TD_BOOST
};

class TemporaryEffect {
public:
    TemporaryEffect(EffectType type, int magnitude, int duration, const std::string& sourceName);

    // Getters
    EffectType getType() const { return type_; }
    int getMagnitude() const { return magnitude_; }
    int getRemainingDuration() const { return remainingDuration_; }
    std::string getSourceName() const { return sourceName_; }
    bool isExpired() const { return remainingDuration_ <= 0; }

    // Duration management
    void decrementDuration() { if (remainingDuration_ > 0) remainingDuration_--; }

    // Effect description
    std::string getDescription() const;
    std::string getEffectIcon() const;
    std::string getEffectColor() const;
    int getColorPriority() const;

private:
    EffectType type_;
    int magnitude_;
    int remainingDuration_;
    std::string sourceName_;
};

class TemporaryEffectsManager {
public:
    // Add/remove effects
    void addEffect(const TemporaryEffect& effect);
    void clearExpiredEffects();
    void clearAllEffects();

    // Get total modifiers for each stat
    int getHustleModifier() const;
    int getAgilityModifier() const;
    int getNdasModifier() const;
    int getLuckModifier() const;
    int getSynergyModifier() const;
    int getTechnicalDebtModifier() const;
    int getDefenseShield() const;

    // Process effects that trigger each turn
    int processDamageOverTime() const;
    int processRegeneration() const;

    // Tick down all effect durations
    void processTurn();

    // Query effects
    bool hasEffect(EffectType type) const;
    std::vector<TemporaryEffect> getActiveEffects() const { return effects_; }
    int getEffectCount() const { return effects_.size(); }

    // Display
    std::string getEffectsSummary() const;
    int getColorPriority() const;
    std::string getDisplayColor() const;
private:
    std::vector<TemporaryEffect> effects_;

    int getTotalModifier(EffectType boostType, EffectType reductionType) const;
};

#endif // TEMPORARYEFFECT_H
