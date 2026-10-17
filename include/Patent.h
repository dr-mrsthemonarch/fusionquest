#ifndef PATENT_H
#define PATENT_H

#include <string>
#include <unordered_map>
#include <vector>
#include "CombatEntity.h"
#include <set>
#include "PatentLibrary.h"

class Character;
class CombatEntity;
class PatentLibrary;
struct PatentDefinition;

enum class PatentType {
    ATTACK,
    HEAL,
    DEBT,
    DEFENSE,
    BUFF,
    DEBUFF,
    AOE
};

enum class PatentTarget {
    SELF,
    ALLY,
    ENEMY,
    ALL_ALLIES,
    ALL_ENEMIES
};

class Patent {
private:
    std::string name_;
    int level_;
    int seriesRequired_;
    int currentPower_;
    int maxLevel_;
    PatentType type_;
    int basePower_;
    int cost_;
    PatentTarget target_;

    // Helper methods
    int calculateCost() const;

    int calculatePowerIncrease(int primeValue);

    PatentTarget getDefaultTargetForType(PatentType type);

    // Effect application methods
    void applyAttackEffect(CombatEntity *caster, CombatEntity *target) const;

    void applyHealEffect(CombatEntity *caster, CombatEntity *target) const;

    void applyDefenseEffect(CombatEntity *caster, CombatEntity *target) const;

    void applyBuffEffect(CombatEntity *caster, CombatEntity *target) const;

    void applyDebuffEffect(CombatEntity *caster, CombatEntity *target) const;

public:
    // Static members
    static std::unordered_map<std::string, Patent> characterPatents;
    static std::vector<std::string> patentNames;
    static std::vector<int> primes;
    static std::vector<std::string> patentTypeSuffixes;
    static std::unique_ptr<PatentLibrary> patentLibrary; // Changed name to avoid conflict

    void setCustomModifiers(int hustle, int agility, int ndas, int luck, int synergy, int techDebt, int duration) {
        customHustleModifier_ = hustle;
        customAgilityModifier_ = agility;
        customNdasModifier_ = ndas;
        customLuckModifier_ = luck;
        customSynergyModifier_ = synergy;
        customTechnicalDebtModifier_ = techDebt;
        customDuration_ = duration;
        hasCustomModifiers_ = true;
    }

    // ADD DEFAULT CONSTRUCTOR
    Patent() = default;

    // Existing constructors
    Patent(const std::string &name, int seriesRequired, PatentType type = PatentType::ATTACK, int basePower = 10);

    Patent(const std::string &name, int seriesRequired, PatentType type, int basePower, int cost, PatentTarget target);

    // Core functionality
    void levelUp(int characterSeries);

    bool canLevelUp(int characterSeries) const;

    void applyEffect(CombatEntity *caster, CombatEntity *target) const;

    // Getters
    std::string getName() const;

    std::string getFullName() const;

    std::string getTypeName() const;

    std::string getTargetName() const;

    int getLevel() const;

    int getCurrentPower() const;

    int getSeriesRequired() const;

    PatentType getType() const;

    int getCost() const;

    PatentTarget getTarget() const;

    bool hasCustomModifiers() const { return hasCustomModifiers_; }

    int getCustomHustleMod() const { return customHustleModifier_; }

    int getCustomAgilityMod() const { return customAgilityModifier_; }

    int getCustomNdasMod() const { return customNdasModifier_; }

    int getCustomLuckMod() const { return customLuckModifier_; }

    int getCustomSynergyMod() const { return customSynergyModifier_; }

    int getCustomTechDebtMod() const { return customTechnicalDebtModifier_; }

    int getCustomDuration() const { return customDuration_; }

    // Static methods
    static void initializePrimes();

    static bool isPrimeSeries(int series);

    static void loadPatentNames(const std::string &filename);

    static void generateRandomPatent(Character *character);

    static void generateStartingPatents(Character *character);

    static void initializeRandomPatents(Character *character, int count);

    static void levelUpCharacterPatents(Character *character);

    static void printCharacterPatents();

    static void usePatent(const std::string &patentName, CombatEntity *caster, CombatEntity *target);

    static std::vector<std::string> getPatentsByType(PatentType type);

    static std::unique_ptr<PatentLibrary> PatentLibrary;

    void applyAOEEffect(CombatEntity *caster, std::vector<CombatEntity *> targets) const;

    static Patent createFromNamedDef(const PatentDefinition& def);


private:
    int customHustleModifier_ = 0;
    int customAgilityModifier_ = 0;
    int customNdasModifier_ = 0;
    int customLuckModifier_ = 0;
    int customSynergyModifier_ = 0;
    int customTechnicalDebtModifier_ = 0;
    int customDuration_ = 0;
    bool hasCustomModifiers_ = false;

    // Helper static methods
    static PatentType getRandomPatentType();

    static std::string generatePatentName(PatentType type);

    static int calculateBasePower(PatentType type);
};

#endif
