#include "../include/Patent.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <random>
#include "../include/CombatEntity.h"
#include "../include/Character.h"
#include "../include/TemporaryEffect.h"
#include "../include/PatentDefinition.h"

std::unordered_map<std::string, Patent> Patent::characterPatents;
std::vector<std::string> Patent::patentNames;
std::vector<int> Patent::primes;
std::unique_ptr<PatentLibrary> Patent::PatentLibrary = nullptr;

std::vector<std::string> Patent::patentTypeSuffixes = {"Strike", "Barrier", "Heal", "Boost", "Curse", "Aura", "Shield", "Wave", "Surge", "Field", "Rush", "Guard", "Blessing", "Hex", "Vortex"};

void Patent::initializePrimes() {
    if (!primes.empty()) return;

    primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229};

    std::cout << "Initialized " << primes.size() << " prime numbers for patent leveling\n";
}

bool Patent::isPrimeSeries(int series) {
    for (int prime: primes) {
        if (prime == series) {
            return true;
        }
    }
    return false;
}

Patent::Patent(const std::string &name, int seriesRequired, PatentType type, int basePower)
    : name_(name), level_(1), seriesRequired_(seriesRequired), currentPower_(basePower), maxLevel_(10), type_(type), basePower_(basePower), cost_(calculateCost()), target_(getDefaultTargetForType(type)) {
}

Patent::Patent(const std::string &name, int seriesRequired, PatentType type, int basePower, int cost, PatentTarget target)
    : name_(name), level_(1), seriesRequired_(seriesRequired), currentPower_(basePower), maxLevel_(10), type_(type), basePower_(basePower), cost_(cost), target_(target) {
}

void Patent::applyAOEEffect(CombatEntity* caster, std::vector<CombatEntity*> targets) const {
    if (targets.empty()) return;

    if (caster->getHypeEnergy() < cost_) {
        std::cout << "Not enough hype energy to use " << name_ << "!\n";
        return;
    }

    // Consume hype energy
    caster->setHypeEnergy(caster->getHypeEnergy() - cost_);

    std::cout << caster->getName() << " uses " << getFullName() << " (AOE)! ";

    // AOE damage to all targets
    int totalDamage = 0;
    for (auto* target : targets) {
        if (!target || !target->isAlive()) continue;

        int damage = currentPower_ + (caster->getHustle() / 3); // Slightly reduced per-target
        int finalDamage = std::max(1, damage - (target->getNDAs() / 3));

        target->takeDamage(finalDamage);
        totalDamage += finalDamage;
    }

    std::cout << "Dealt total " << totalDamage << " damage across "
              << targets.size() << " enemies!\n";
}

Patent Patent::createFromNamedDef(const PatentDefinition& def) {
    Patent patent(def.name, def.seriesRequired, def.type, def.basePower, def.cost, def.target);

    // Apply custom modifiers if any exist
    bool hasCustomMods = def.hustleModifier != 0 || def.agilityModifier != 0 ||
                        def.ndasModifier != 0 || def.luckModifier != 0 ||
                        def.synergyModifier != 0 || def.technicalDebtModifier != 0;

    if (hasCustomMods) {
        patent.setCustomModifiers(
            def.hustleModifier,
            def.agilityModifier,
            def.ndasModifier,
            def.luckModifier,
            def.synergyModifier,
            def.technicalDebtModifier,
            def.duration
        );
    }

    return patent;
}

int Patent::calculateCost() const {
    switch (type_) {
        case PatentType::ATTACK:
            return 5 + (basePower_ / 10);
        case PatentType::AOE:
            return static_cast<int>((7 + (basePower_ / 8)) * 1.3); // 30% more expensive
        case PatentType::HEAL:
            return 8 + (basePower_ / 8);
        case PatentType::DEFENSE:
            return 6 + (basePower_ / 12);
        case PatentType::BUFF:
            return 7 + (basePower_ / 15);
        case PatentType::DEBUFF:
            return 7 + (basePower_ / 15);
        default:
            return 10;
    }
}

PatentTarget Patent::getDefaultTargetForType(PatentType type) {
    switch (type) {
        case PatentType::ATTACK:
        case PatentType::DEBUFF:
            return PatentTarget::ENEMY;
        case PatentType::HEAL:
        case PatentType::DEFENSE:
        case PatentType::BUFF:
            return PatentTarget::SELF;
        default:
            return PatentTarget::SELF;
    }
}

void Patent::levelUp(int characterSeries) {
    if (canLevelUp(characterSeries)) {
        level_++;

        // Power increases based on level and prime factor - scaled for 10 levels
        int primeIndex = level_ - 1;
        int primeValue = (primeIndex < primes.size()) ? primes[primeIndex] : primes.back();

        // Different power scaling based on patent type
        int powerIncrease = calculatePowerIncrease(primeValue);
        currentPower_ += powerIncrease;

        // Reduce cost slightly with level ups (efficiency improvement)
        if (level_ % 2 == 0) {
            cost_ = std::max(1, cost_ - 1);
        }

        std::cout << name_ << " leveled up to " << getFullName() << "! Power: " << currentPower_ << " (+" << powerIncrease << "), Cost: " << cost_ << " hype energy\n";
    }
}

int Patent::calculatePowerIncrease(int primeValue) {
    switch (type_) {
        case PatentType::ATTACK:
            return 25 + (level_ * 4) + (primeValue / 2);
        case PatentType::HEAL:
            return 20 + (level_ * 3) + (primeValue / 3);
        case PatentType::DEFENSE:
            return 15 + (level_ * 2) + (primeValue / 4);
        case PatentType::BUFF:
            return 10 + (level_ * 2) + (primeValue / 5);
        case PatentType::DEBUFF:
            return 10 + (level_ * 2) + (primeValue / 5);
        default:
            return 20 + (level_ * 3) + (primeValue / 3);
    }
}

bool Patent::canLevelUp(int characterSeries) const {
    if (level_ >= maxLevel_) return false;
    if (!isPrimeSeries(characterSeries)) return false;

    // Calculate cumulative prime requirement: sum of primes up to current level
    int cumulativePrimes = 0;
    for (int i = 0; i < level_; i++) {
        cumulativePrimes += (i < primes.size()) ? primes[i] : primes.back();
    }

    return characterSeries >= (seriesRequired_ + cumulativePrimes);
}

void Patent::applyEffect(CombatEntity *caster, CombatEntity *target) const {
    if (!caster || !target) return;

    if (caster->getHypeEnergy() < cost_) {
        std::cout << "Not enough hype energy to use " << name_ << "!\n";
        return;
    }

    // Consume hype energy
    caster->setHypeEnergy(caster->getHypeEnergy() - cost_);

    std::cout << caster->getName() << " uses " << getFullName() << "! ";

    switch (type_) {
        case PatentType::ATTACK:
            applyAttackEffect(caster, target);
            break;
        case PatentType::HEAL:
            applyHealEffect(caster, target);
            break;
        case PatentType::DEFENSE:
            applyDefenseEffect(caster, target);
            break;
        case PatentType::BUFF:
            applyBuffEffect(caster, target);
            break;
        case PatentType::DEBUFF:
            applyDebuffEffect(caster, target);
            break;
    }
}

void Patent::applyAttackEffect(CombatEntity *caster, CombatEntity *target) const {
    int damage = currentPower_ + (caster->getHustle() / 2);
    int finalDamage = std::max(1, damage - (target->getNDAs() / 3));

    target->takeDamage(finalDamage);
    std::cout << "Deals " << finalDamage << " damage to " << target->getName() << "!\n";
}

void Patent::applyHealEffect(CombatEntity *caster, CombatEntity *target) const {
    int healAmount = currentPower_ + (caster->getSynergy() / 2);
    int currentRunway = target->getRunway();
    int maxRunway = target->getMaxRunway();

    int actualHeal = std::min(healAmount, maxRunway - currentRunway);
    target->setRunway(currentRunway + actualHeal);

    std::cout << "Heals " << actualHeal << " runway for " << target->getName() << "!\n";

    // High-level heal patents also grant regeneration
    if (level_ >= 5) {
        int regenAmount = currentPower_ / 20;
        int regenTurns = 2;
        TemporaryEffect regenEffect(EffectType::REGENERATION, regenAmount, regenTurns, name_);
        target->addTemporaryEffect(regenEffect);
        std::cout << target->getName() << " gains regeneration (+" << regenAmount << " HP/turn) for " << regenTurns << " turns!\n";
    }
}

void Patent::applyDefenseEffect(CombatEntity *caster, CombatEntity *target) const {
    int defenseBoost = currentPower_ / 2;
    int turns = 2 + (level_ / 3);

    // Create a defense shield effect
    TemporaryEffect shieldEffect(EffectType::DEFENSE_SHIELD, defenseBoost, turns, name_);
    target->addTemporaryEffect(shieldEffect);

    std::cout << target->getName() << " gains a defense shield (+" << defenseBoost << " defense) for " << turns << " turns!\n";
}

void Patent::applyBuffEffect(CombatEntity *caster, CombatEntity *target) const {
    // Check if this is a custom named patent with specific modifiers
    if (hasCustomModifiers_) {
        int turns = customDuration_ > 0 ? customDuration_ : (2 + (level_ / 4));

        // Apply each non-zero modifier as separate effect
        if (customHustleModifier_ != 0) {
            EffectType type = customHustleModifier_ > 0 ? EffectType::HUSTLE_BOOST : EffectType::HUSTLE_REDUCTION;
            TemporaryEffect effect(type, std::abs(customHustleModifier_), turns, name_);
            target->addTemporaryEffect(effect);
            std::cout << target->getName() << "'s Hustle "
                      << (customHustleModifier_ > 0 ? "+" : "") << customHustleModifier_
                      << " for " << turns << " turns!\n";
        }

        if (customAgilityModifier_ != 0) {
            EffectType type = customAgilityModifier_ > 0 ? EffectType::AGILITY_BOOST : EffectType::AGILITY_REDUCTION;
            TemporaryEffect effect(type, std::abs(customAgilityModifier_), turns, name_);
            target->addTemporaryEffect(effect);
            std::cout << target->getName() << "'s Agility "
                      << (customAgilityModifier_ > 0 ? "+" : "") << customAgilityModifier_
                      << " for " << turns << " turns!\n";
        }

        if (customNdasModifier_ != 0) {
            EffectType type = customNdasModifier_ > 0 ? EffectType::NDAS_BOOST : EffectType::NDAS_REDUCTION;
            TemporaryEffect effect(type, std::abs(customNdasModifier_), turns, name_);
            target->addTemporaryEffect(effect);
            std::cout << target->getName() << "'s NDAs "
                      << (customNdasModifier_ > 0 ? "+" : "") << customNdasModifier_
                      << " for " << turns << " turns!\n";
        }

        if (customLuckModifier_ != 0) {
            EffectType type = customLuckModifier_ > 0 ? EffectType::LUCK_BOOST : EffectType::LUCK_REDUCTION;
            TemporaryEffect effect(type, std::abs(customLuckModifier_), turns, name_);
            target->addTemporaryEffect(effect);
            std::cout << target->getName() << "'s Luck "
                      << (customLuckModifier_ > 0 ? "+" : "") << customLuckModifier_
                      << " for " << turns << " turns!\n";
        }

        if (customSynergyModifier_ != 0) {
            EffectType type = customSynergyModifier_ > 0 ? EffectType::SYNERGY_BOOST : EffectType::SYNERGY_REDUCTION;
            TemporaryEffect effect(type, std::abs(customSynergyModifier_), turns, name_);
            target->addTemporaryEffect(effect);
            std::cout << target->getName() << "'s Synergy "
                      << (customSynergyModifier_ > 0 ? "+" : "") << customSynergyModifier_
                      << " for " << turns << " turns!\n";
        }

        // Technical debt (no existing effect type, just report)
        if (customTechnicalDebtModifier_ != 0) {
            std::cout << target->getName() << "'s Technical Debt "
                      << (customTechnicalDebtModifier_ > 0 ? "+" : "") << customTechnicalDebtModifier_
                      << " (narrative effect)\n";
        }

        return;
    }

    // Original buff logic for non-custom patents
    int statChoice = std::hash<std::string>{}(name_) % 4;
    int boostAmount = currentPower_ / 10;
    int turns = 2 + (level_ / 4);

    EffectType effectType;
    std::string statName;

    switch (statChoice) {
        case 0:
            effectType = EffectType::HUSTLE_BOOST;
            statName = "Hustle";
            break;
        case 1:
            effectType = EffectType::AGILITY_BOOST;
            statName = "Agile-ity";
            break;
        case 2:
            effectType = EffectType::LUCK_BOOST;
            statName = "Luck";
            break;
        case 3:
            effectType = EffectType::SYNERGY_BOOST;
            statName = "Synergy";
            break;
    }

    TemporaryEffect buffEffect(effectType, boostAmount, turns, name_);
    target->addTemporaryEffect(buffEffect);

    std::cout << target->getName() << "'s " << statName << " increased by " << boostAmount << " for " << turns << " turns!\n";
}

void Patent::applyDebuffEffect(CombatEntity *caster, CombatEntity *target) const {
    // Check if this is a custom named patent with specific modifiers
    if (hasCustomModifiers_) {
        int turns = customDuration_ > 0 ? customDuration_ : (2 + (level_ / 4));

        // Apply each non-zero modifier as separate effect (inverted for debuff)
        if (customHustleModifier_ != 0) {
            // If config says +10, debuff means -10 to enemy
            EffectType type = EffectType::HUSTLE_REDUCTION;
            TemporaryEffect effect(type, std::abs(customHustleModifier_), turns, name_);
            target->addTemporaryEffect(effect);
            std::cout << target->getName() << "'s Hustle -" << std::abs(customHustleModifier_)
                      << " for " << turns << " turns!\n";
        }

        if (customAgilityModifier_ != 0) {
            EffectType type = EffectType::AGILITY_REDUCTION;
            TemporaryEffect effect(type, std::abs(customAgilityModifier_), turns, name_);
            target->addTemporaryEffect(effect);
            std::cout << target->getName() << "'s Agility -" << std::abs(customAgilityModifier_)
                      << " for " << turns << " turns!\n";
        }

        if (customNdasModifier_ != 0) {
            EffectType type = EffectType::NDAS_REDUCTION;
            TemporaryEffect effect(type, std::abs(customNdasModifier_), turns, name_);
            target->addTemporaryEffect(effect);
            std::cout << target->getName() << "'s NDAs -" << std::abs(customNdasModifier_)
                      << " for " << turns << " turns!\n";
        }

        if (customLuckModifier_ != 0) {
            EffectType type = EffectType::LUCK_REDUCTION;
            TemporaryEffect effect(type, std::abs(customLuckModifier_), turns, name_);
            target->addTemporaryEffect(effect);
            std::cout << target->getName() << "'s Luck -" << std::abs(customLuckModifier_)
                      << " for " << turns << " turns!\n";
        }

        if (customSynergyModifier_ != 0) {
            EffectType type = EffectType::SYNERGY_REDUCTION;
            TemporaryEffect effect(type, std::abs(customSynergyModifier_), turns, name_);
            target->addTemporaryEffect(effect);
            std::cout << target->getName() << "'s Synergy -" << std::abs(customSynergyModifier_)
                      << " for " << turns << " turns!\n";
        }

        if (customTechnicalDebtModifier_ != 0) {
            std::cout << target->getName() << "'s Technical Debt +" << std::abs(customTechnicalDebtModifier_)
                      << " (narrative effect)\n";
        }

        return;
    }

    // Original debuff logic for non-custom patents
    int statChoice = std::hash<std::string>{}(name_) % 4;
    int reduceAmount = currentPower_ / 8;
    int turns = 2 + (level_ / 4);

    EffectType effectType;
    std::string statName;

    switch (statChoice) {
        case 0:
            effectType = EffectType::HUSTLE_REDUCTION;
            statName = "Hustle";
            break;
        case 1:
            effectType = EffectType::AGILITY_REDUCTION;
            statName = "Agile-ity";
            break;
        case 2:
            effectType = EffectType::NDAS_REDUCTION;
            statName = "NDAs";
            break;
        case 3:
            effectType = EffectType::LUCK_REDUCTION;
            statName = "luck";
            break;
    }

    TemporaryEffect debuffEffect(effectType, reduceAmount, turns, name_);
    target->addTemporaryEffect(debuffEffect);

    std::cout << target->getName() << "'s " << statName << " decreased by " << reduceAmount << " for " << turns << " turns!\n";
}

std::string Patent::getTypeName() const {
    switch (type_) {
        case PatentType::ATTACK: return "Attack";
        case PatentType::AOE: return "AOE";
        case PatentType::HEAL: return "Heal";
        case PatentType::DEFENSE: return "Defense";
        case PatentType::BUFF: return "Buff";
        case PatentType::DEBUFF: return "Debuff";
        default: return "Unknown";
    }
}

std::string Patent::getTargetName() const {
    switch (target_) {
        case PatentTarget::SELF: return "Self";
        case PatentTarget::ALLY: return "Ally";
        case PatentTarget::ENEMY: return "Enemy";
        case PatentTarget::ALL_ALLIES: return "All Allies";
        case PatentTarget::ALL_ENEMIES: return "All Enemies";
        default: return "Unknown";
    }
}

std::string Patent::getName() const { return name_; }

int Patent::getLevel() const { return level_; }

int Patent::getCurrentPower() const { return currentPower_; }

int Patent::getSeriesRequired() const { return seriesRequired_; }

PatentType Patent::getType() const { return type_; }

int Patent::getCost() const { return cost_; }

PatentTarget Patent::getTarget() const { return target_; }

std::string intToRoman(int num) {
    static const std::vector<std::pair<int, std::string> > roman = {{1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"}, {90, "XC"}, {50, "L"}, {40, "XL"}, {10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"}};

    std::string result;
    for (const auto &[value, symbol]: roman) {
        while (num >= value) {
            result += symbol;
            num -= value;
        }
    }
    return result;
}

bool isPrime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; ++i) if (n % i == 0) return false;
    return true;
}

std::vector<std::string> getPrimeRomanNumerals() {
    std::vector<std::string> romanPrimes;
    romanPrimes.reserve(1000);

    int count = 0;
    int num = 2;
    while (count < 1000) {
        if (isPrime(num)) {
            romanPrimes.push_back(intToRoman(num));
            ++count;
        }
        ++num;
    }
    return romanPrimes;
}

std::string Patent::getFullName() const {
    // Generate Roman numerals for the first 1000 prime numbers
    static std::vector<std::string> romanPrimes = getPrimeRomanNumerals();

    // Check if the level is a prime number and within the range of generated
    // primes
    if (isPrime(level_) && level_ <= 3999) {
        return name_ + " (" + romanPrimes[level_ - 1] + ")";
    }

    // Roman numerals for levels I through X
    static const std::vector<std::string> standardRomanNumerals = {" I", " II", " III", " IV", " V", " VI", " VII", " VIII", " IX", " X"};

    if (level_ >= 1 && level_ <= 3999) {
        return name_ + standardRomanNumerals[level_ - 1];
    }

    // Fallback
    return name_ + " I";
}

void Patent::loadPatentNames(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open patent names file: " << filename << std::endl;
        return;
    }

    patentNames.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] != '#') {
            patentNames.push_back(line);
        }
    }

    file.close();
    std::cout << "Loaded " << patentNames.size() << " patent names from " << filename << std::endl;
}

PatentType Patent::getRandomPatentType() {
    int choice = rand() % 6; // Now 6 types
    return static_cast<PatentType>(choice);
}

std::string Patent::generatePatentName(PatentType type) {
    if (patentNames.empty()) {
        return "Default Patent";
    }

    int nameIndex = rand() % patentNames.size();
    std::string baseName = patentNames[nameIndex];

    int suffixIndex = rand() % patentTypeSuffixes.size();
    std::string suffix = patentTypeSuffixes[suffixIndex];

    return baseName + " " + suffix;
}

int Patent::calculateBasePower(PatentType type) {
    switch (type) {
        case PatentType::ATTACK: return 15 + (rand() % 20);
        case PatentType::HEAL: return 12 + (rand() % 15);
        case PatentType::DEFENSE: return 10 + (rand() % 12);
        case PatentType::BUFF: return 8 + (rand() % 10);
        case PatentType::DEBUFF: return 8 + (rand() % 10);
        default: return 10;
    }
}

void Patent::generateRandomPatent(Character *character) {
    if (patentNames.empty()) {
        std::cerr << "No patent names loaded!" << std::endl;
        return;
    }
    std::mt19937 rng;
    // 20% chance for named patent, 80% for random
    std::uniform_int_distribution<> typeDist(0, 99);
    bool useNamed = PatentLibrary && (typeDist(rng) < 20);

    if (useNamed) {
        const PatentDefinition* def = PatentLibrary->getRandomNamedPatent();
        if (def) {
            Patent patent = createFromNamedDef(*def);
            characterPatents.insert({def->name, patent});

            std::cout << "Acquired named " << patent.getTypeName() << " patent: "
                      << def->name << " (Power: " << def->basePower
                      << ", Cost: " << def->cost << " hype energy)\n";
            return;
        }
    }

    // Fallback to random generation (existing code)
    PatentType randomType = getRandomPatentType();
    std::string patentName = generatePatentName(randomType);
    int basePower = calculateBasePower(randomType);

    // Series requirement based on character's current series and prime progression
    int characterSeries = character->getSeries();
    int baseRequirement = std::max(1, characterSeries);

    // Set requirement to the next prime number after current series
    int requiredSeries = baseRequirement;
    for (int prime: primes) {
        if (prime > baseRequirement) {
            requiredSeries = prime;
            break;
        }
    }

    Patent newPatent(patentName, requiredSeries, randomType, basePower);

    // Use insert instead of operator[] to avoid default constructor issues
    characterPatents.insert({patentName, newPatent});

    int firstPrimeReq = requiredSeries + primes[0];

    std::cout << "Acquired new " << newPatent.getTypeName() << " patent: " << patentName
              << " (Power: " << basePower << ", Cost: " << newPatent.getCost() << " hype energy)"
              << " (Level I -> II requires Series " << firstPrimeReq << ")\n";
}

void Patent::generateStartingPatents(Character* character) {
    std::cout << "\n Learning starting patents..." << std::endl;

    if (!PatentLibrary || PatentLibrary->getPatentCount() < 3) {
        std::cerr << "ERROR: Need at least 3 named patents for starting loadout!" << std::endl;
        return;
    }

    // Get first 3 patents from named library
    for (int i = 0; i < 3; i++) {
        const PatentDefinition* def = PatentLibrary->getPatentByIndex(i);
        if (!def) continue;

        Patent patent = createFromNamedDef(*def);
        characterPatents.insert({def->name, patent});

        std::cout << "âœ… Learned " << def->name << " (" << patent.getTypeName() << ")" << std::endl;
        std::cout << "   Power: " << def->basePower << " | Cost: " << def->cost << " hype energy" << std::endl;
    }

    std::cout << std::endl;
}

void Patent::initializeRandomPatents(Character *character, int count) {
    static bool initialized = false;
    if (!initialized) {
        srand(time(nullptr));
        initializePrimes(); // Make sure primes are initialized
        initialized = true;
    }
    generateRandomPatent(character);
}

void Patent::levelUpCharacterPatents(Character *character) {
    int characterSeries = character->getSeries();

    // Only attempt to level up patents on prime series numbers
    if (!isPrimeSeries(characterSeries)) {
        return;
    }

    std::cout << "\n--- Prime Series " << characterSeries << " - Patent Level Up Opportunity ---\n";

    bool anyLeveledUp = false;
    for (auto &patentPair: characterPatents) {
        Patent &patent = patentPair.second;
        if (patent.canLevelUp(characterSeries)) {
            patent.levelUp(characterSeries);
            anyLeveledUp = true;
        }
    }

    if (!anyLeveledUp) {
        std::cout << "No patents ready to level up at Series " << characterSeries << "\n";

        // Show what's needed for next levels
        for (auto &patentPair: characterPatents) {
            Patent &patent = patentPair.second;
            if (patent.getLevel() < patent.maxLevel_) {
                int cumulativePrimes = 0;
                for (int i = 0; i < patent.getLevel(); i++) {
                    cumulativePrimes += (i < primes.size()) ? primes[i] : primes.back();
                }
                int nextRequired = patent.getSeriesRequired() + cumulativePrimes;
                std::cout << patent.getName() << " " << patent.getFullName() << " (" << patent.getTypeName() << ") needs Series " << nextRequired << " for next level\n";
            }
        }
    }
    std::cout << "----------------------------------------\n\n";
}

void Patent::printCharacterPatents() {
    std::cout << "Character's Patents:" << std::endl;
    if (characterPatents.empty()) {
        std::cout << "No patents acquired." << std::endl;
        return;
    }

    for (const auto &patentPair: characterPatents) {
        const Patent &patent = patentPair.second;

        if (patent.getLevel() < patent.maxLevel_) {
            // Calculate next requirement
            int cumulativePrimes = 0;
            for (int i = 0; i < patent.getLevel(); i++) {
                cumulativePrimes += (i < primes.size()) ? primes[i] : primes.back();
            }
            int nextRequired = patent.getSeriesRequired() + cumulativePrimes;

            std::cout << "Patent: " << patent.getFullName() << " [" << patent.getTypeName() << " -> " << patent.getTargetName() << "]" << ", Power: " << patent.getCurrentPower() << ", Cost: " << patent.getCost() << " hype energy" << ", Next: Series " << nextRequired << " (Prime " << (std::find(primes.begin(), primes.end(), nextRequired) - primes.begin() + 1) << ")\n";
        } else {
            std::cout << "Patent: " << patent.getFullName() << " [" << patent.getTypeName() << " -> " << patent.getTargetName() << "]" << ", Power: " << patent.getCurrentPower() << ", Cost: " << patent.getCost() << " hype energy" << " [MAX LEVEL]\n";
        }
    }
}

void Patent::usePatent(const std::string &patentName, CombatEntity *caster, CombatEntity *target) {
    auto it = characterPatents.find(patentName);
    if (it != characterPatents.end()) {
        it->second.applyEffect(caster, target);
    } else {
        std::cout << "Patent '" << patentName << "' not found!\n";
    }
}

std::vector<std::string> Patent::getPatentsByType(PatentType type) {
    std::vector<std::string> result;
    for (const auto &patentPair: characterPatents) {
        if (patentPair.second.getType() == type) {
            result.push_back(patentPair.first);
        }
    }
    return result;
}
