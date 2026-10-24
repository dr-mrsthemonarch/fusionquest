// SPG Engine made by drmrsthemonarch
#include "../include/Tuning.h"
#include "../include/Balance.h"
#include "../include/ConfigStore.h"
#include "../include/MonsterGenerator.h"
#include "../include/PatentStrategy.h"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iostream>
#include "../include/ItemGenerator.h"
#include "../include/Patent.h"


MonsterGenerator::MonsterGenerator() : loadedSuccessfully_(false), rng_(std::time(nullptr)) {
}

namespace MonsterScaling {
    // Linear scaling: base + (level * growth)
    double linearScale(int level, double base, double growthPerLevel) {
        return base + (level * growthPerLevel);
    }

    // Polynomial scaling: base * (1 + level)^exponent
    double polynomialScale(int level, double base, double exponent) {
        return base * std::pow(1.0 + level, exponent);
    }

    // Exponential scaling: base * multiplier^level
    double exponentialScale(int level, double base, double multiplier) {
        return base * std::pow(multiplier, level);
    }

    // Difficulty multipliers
    double getDifficultyMultiplier(MonsterDifficulty difficulty) {
        return Balance::difficultyMult(static_cast<int>(difficulty));
    }

    // Add random variance (±percentage)
    double applyVariance(double value, double variancePercent, std::mt19937 &rng) {
        std::uniform_real_distribution<> dist(1.0 - variancePercent, 1.0 + variancePercent);
        return value * dist(rng);
    }
}

namespace {
int vary(double v, double pct, std::mt19937 &rng) {
    return static_cast<int>(MonsterScaling::applyVariance(static_cast<int>(v), pct, rng));
}
}  // namespace

BossStats MonsterGenerator::calculateBossStats(int playerSeries, MonsterDifficulty difficulty) {
    Balance::EnemyStats e = Balance::boss(playerSeries, MonsterScaling::getDifficultyMultiplier(difficulty));
    BossStats s;
    s.maxHealth = vary(e.hp, T("boss.var_hp"), rng_);
    s.health = s.maxHealth;
    s.attack = vary(e.atk, T("boss.var_atk"), rng_);
    s.defense = vary(e.def, T("boss.var_def"), rng_);
    s.speed = vary(e.spd, T("boss.var_spd"), rng_);
    s.moneyReward = vary(e.money, T("boss.var_money"), rng_);
    return s;
}

MobStats MonsterGenerator::calculateMobStats(int playerSeries, MonsterDifficulty difficulty) {
    Balance::EnemyStats e = Balance::mob(playerSeries, MonsterScaling::getDifficultyMultiplier(difficulty));
    MobStats s;
    s.maxHealth = vary(e.hp, T("mob.var_hp"), rng_);
    s.health = s.maxHealth;
    s.attack = vary(e.atk, T("mob.var_atk"), rng_);
    s.defense = vary(e.def, T("mob.var_def"), rng_);
    s.speed = vary(e.spd, T("mob.var_spd"), rng_);
    s.moneyReward = vary(e.money, T("mob.var_money"), rng_);
    return s;
}

std::string MonsterGenerator::generateMobName() {
    std::uniform_int_distribution<> adjDist(0, adjectives_.size() - 1);
    std::uniform_int_distribution<> mobDist(0, mobNames_.size() - 1);
    std::uniform_int_distribution<> mythDist(0, mythicalNouns_.size() - 1);
    std::uniform_int_distribution<> chanceDist(0, 100);

    std::string name;

    if (!killKeywords_.empty() && std::uniform_real_distribution<>(0.0, 1.0)(rng_) < killBias_) {
        std::uniform_int_distribution<size_t> kd(0, killKeywords_.size() - 1);
        if (chanceDist(rng_) < 70) name += adjectives_[adjDist(rng_)] + " ";
        return name + killKeywords_[kd(rng_)];
    }

    if (chanceDist(rng_) < 70) {
        name += adjectives_[adjDist(rng_)] + " ";
    }
    if (chanceDist(rng_) < 60) {
        name += mobNames_[mobDist(rng_)];
    } else {
        name += mythicalNouns_[mythDist(rng_)];
    }

    return name;
}

std::string MonsterGenerator::generateBossName() {
    std::uniform_int_distribution<> titleDist(0, titles_.size() - 1);
    std::uniform_int_distribution<> adjDist(0, adjectives_.size() - 1);
    std::uniform_int_distribution<> bossDist(0, bossNames_.size() - 1);
    std::uniform_int_distribution<> chanceDist(0, 100);

    std::string name;

    if (chanceDist(rng_) < 60) {
        name += titles_[titleDist(rng_)] + " ";
    }

    if (chanceDist(rng_) < 80) {
        name += adjectives_[adjDist(rng_)] + " ";
    }

    name += bossNames_[bossDist(rng_)];

    return name;
}

std::vector<Monster> MonsterGenerator::generateEncounter(int playerSeries, int &mobCount, int &bossCount, ItemGenerator &itemGen) {
    std::vector<Monster> encounter;

    std::uniform_int_distribution<> mobCountDist(1, 4);
    mobCount = mobCountDist(rng_);

    std::uniform_int_distribution<> bossChanceDist(0, 100);
    int bossChance = 20 + (playerSeries * 5);
    bossCount = (bossChanceDist(rng_) < bossChance) ? 1 : 0;

    for (int i = 0; i < mobCount; ++i) {
        encounter.push_back(generateMob(playerSeries, itemGen));
    }

    if (bossCount > 0) {
        encounter.push_back(generateBoss(playerSeries));
    }

    return encounter;
}

Monster MonsterGenerator::generateBoss(int playerSeries) {
    Monster boss;
    boss.type = MonsterType::BOSS;
    boss.name = generateBossName();
    boss.difficulty = getRandomDifficulty();
    boss.level = playerSeries;

    // Use new scaling calculation
    BossStats stats = calculateBossStats(playerSeries, boss.difficulty);
    boss.maxHealth = stats.maxHealth;
    boss.health = stats.health;
    boss.attack = stats.attack;
    boss.defense = stats.defense;
    boss.speed = stats.speed;
    boss.moneyReward = stats.moneyReward;

    // Bosses get hype energy and patents
    boss.maxHypeEnergy = static_cast<int>(T("monster.boss_hype_base") + playerSeries * T("monster.boss_hype_per"));
    boss.hypeEnergy = boss.maxHypeEnergy;

    // Assign strategy based on difficulty
    switch (boss.difficulty) {
        case MonsterDifficulty::EASY:
        case MonsterDifficulty::MEDIUM:
            boss.setStrategy(std::make_unique<BalancedStrategy>());
            break;
        case MonsterDifficulty::HARD:
            boss.setStrategy(std::make_unique<TacticalStrategy>());
            break;
        case MonsterDifficulty::LEGENDARY:
            boss.setStrategy(std::make_unique<TacticalStrategy>());
            break;
    }

    // Give bosses diverse patents based on difficulty
    int patentCount = 1;
    switch (boss.difficulty) {
        case MonsterDifficulty::MEDIUM: patentCount = 2;
            break;
        case MonsterDifficulty::HARD: patentCount = 3;
            break;
        case MonsterDifficulty::LEGENDARY: patentCount = 4;
            break;
        default: patentCount = 1;
    }

    // Generate diverse patent loadout
    generateBossPatents(boss, patentCount, playerSeries);

    boss.itemReward = "EQUIPMENT";
    return boss;
}

Monster MonsterGenerator::generateMob(int playerSeries, ItemGenerator &itemGen) {
    Monster mob;
    mob.type = MonsterType::MOB;
    mob.name = generateMobName();
    mob.difficulty = getRandomDifficulty();
    mob.level = playerSeries;

    // Use scaling
    MobStats stats = calculateMobStats(playerSeries, mob.difficulty);
    mob.maxHealth = stats.maxHealth;
    mob.health = stats.health;
    mob.attack = stats.attack;
    mob.defense = stats.defense;
    mob.speed = stats.speed;
    mob.moneyReward = stats.moneyReward;

    // Mobs get limited hype energy
    mob.maxHypeEnergy = static_cast<int>(T("monster.mob_hype_base") + playerSeries * T("monster.mob_hype_per"));
    mob.hypeEnergy = mob.maxHypeEnergy;

    // Assign simple aggressive strategy to mobs
    mob.setStrategy(std::make_unique<AggressiveStrategy>());

    // Mobs get 1-2 patents based on difficulty
    int patentChance = static_cast<int>(T("monster.mob_patent_easy"));
    int patentCount = 1;
    switch (mob.difficulty) {
        case MonsterDifficulty::MEDIUM:
            patentChance = static_cast<int>(T("monster.mob_patent_medium"));
            break;
        case MonsterDifficulty::HARD:
            patentChance = static_cast<int>(T("monster.mob_patent_hard"));
            patentCount = 2;
            break;
        case MonsterDifficulty::LEGENDARY:
            patentChance = static_cast<int>(T("monster.mob_patent_legendary"));
            patentCount = 2;
            break;
        default:
            break;
    }

    if ((rand() % 100) < patentChance) {
        generateMobPatents(mob, patentCount, playerSeries);
    }

    // Generate dynamic item reward
    GeneratedItem reward = itemGen.generateRandomItem();
    mob.itemReward = reward.name;
    mob.itemValue = reward.value;

    return mob;
}

void MonsterGenerator::generateMonsterPatents(Monster &monster, int count) {
    std::vector<std::string> monsterPatentNames = {"Shadow Strike", "Venom Bite", "Crystal Shield", "Regeneration", "Fury Swipe", "Poison Cloud", "Stone Skin", "Bloodlust", "Thunder Clap", "Frost Breath", "Fireball", "Earthquake"};

    for (int i = 0; i < count; ++i) {
        if (monsterPatentNames.empty()) break;

        int nameIndex = rand() % monsterPatentNames.size();
        std::string patentName = monsterPatentNames[nameIndex];

        // Determine patent type based on name or random
        PatentType type = PatentType::ATTACK; // Default to attack
        if (patentName.find("Shield") != std::string::npos || patentName.find("Skin") != std::string::npos) {
            type = PatentType::DEFENSE;
        } else if (patentName.find("Regeneration") != std::string::npos) {
            type = PatentType::HEAL;
        }

        int basePower = 15 + (monster.level * 2);
        int cost = 8 + (monster.level * 1);

        auto patent = std::make_shared<Patent>(patentName, 0, type, basePower, cost, PatentTarget::SELF);
        monster.addPatent(patent);

        // Remove used name to avoid duplicates
        monsterPatentNames.erase(monsterPatentNames.begin() + nameIndex);
    }
}

MonsterDifficulty MonsterGenerator::getRandomDifficulty() {
    std::uniform_int_distribution<> diffDist(0, 100);
    int roll = diffDist(rng_);

    const double e = T("diff.pct_easy"), md = T("diff.pct_medium"), h = T("diff.pct_hard");
    if (roll < e) return MonsterDifficulty::EASY;
    if (roll < e + md) return MonsterDifficulty::MEDIUM;
    if (roll < e + md + h) return MonsterDifficulty::HARD;
    return MonsterDifficulty::LEGENDARY;
}

bool MonsterGenerator::loadWordList(const std::string &filename, std::vector<std::string> &wordList) {
    ConfigFile file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] != '#') {
            wordList.push_back(line);
        }
    }

    std::cout << "Loaded " << wordList.size() << " entries from " << filename << std::endl;
    return !wordList.empty();
}

bool MonsterGenerator::loadWordLists(const std::string &configPath) {
    bool allLoaded = true;

    allLoaded &= loadWordList(configPath + "bosses.txt", bossNames_);
    allLoaded &= loadWordList(configPath + "mobs.txt", mobNames_);
    allLoaded &= loadWordList(configPath + "titles.txt", titles_);
    allLoaded &= loadWordList(configPath + "adjectives.txt", adjectives_);
    allLoaded &= loadWordList(configPath + "mythical_nouns.txt", mythicalNouns_);

    loadedSuccessfully_ = allLoaded;
    return allLoaded;
}

void MonsterGenerator::generateBossPatents(Monster &boss, int count, int playerSeries) {
    std::vector<std::pair<std::string, PatentType> > patentPool = {
        // Attack patents
        {"Devastating Strike", PatentType::ATTACK}, {"Power Surge", PatentType::ATTACK}, {"Critical Hit", PatentType::ATTACK},

        // Debuff patents (NEW - bosses use these against player!)
        {"Weakness Curse", PatentType::DEBUFF}, {"Slow Field", PatentType::DEBUFF}, {"Armor Break", PatentType::DEBUFF}, {"Confusion Wave", PatentType::DEBUFF},

        // Buff patents
        {"Battle Fury", PatentType::BUFF}, {"Iron Will", PatentType::BUFF},

        // Defense patents
        {"Fortify", PatentType::DEFENSE}, {"Barrier Shield", PatentType::DEFENSE},

        // Heal patents
        {"Regeneration", PatentType::HEAL}, {"Life Drain", PatentType::HEAL}
    };

    // Shuffle to get variety
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(patentPool.begin(), patentPool.end(), g);

    // Ensure diverse patent types
    std::vector<std::pair<std::string, PatentType> > selected;
    std::set<PatentType> typesUsed;

    for (const auto &[name, type]: patentPool) {
        if (selected.size() >= count) break;

        // Try to avoid too many of the same type
        if (typesUsed.count(type) < 2) {
            selected.push_back({name, type});
            typesUsed.insert(type);
        }
    }

    // If we need more, add any type - FIXED VERSION
    for (const auto &patentEntry: patentPool) {
        if (selected.size() >= count) break;
        if (std::find_if(selected.begin(), selected.end(), [&patentEntry](const auto &p) { return p.first == patentEntry.first; }) == selected.end()) {
            selected.push_back(patentEntry);
        }
    }

    // Create patents
    for (const auto &[name, type]: selected) {
        int basePower = static_cast<int>(T("monster.boss_patent_pow") + playerSeries * T("monster.boss_patent_pow_per"));
        int cost = static_cast<int>(T("monster.boss_patent_cost") + playerSeries * T("monster.boss_patent_cost_per"));

        // Debuffs are powerful for bosses
        if (type == PatentType::DEBUFF) {
            basePower = static_cast<int>(basePower * T("monster.debuff_mult"));
        }

        PatentTarget target = (type == PatentType::ATTACK || type == PatentType::DEBUFF) ? PatentTarget::ENEMY : PatentTarget::SELF;

        auto patent = std::make_shared<Patent>(name, 0, type, basePower, cost, target);
        boss.addPatent(patent);
    }
}

void MonsterGenerator::generateMobPatents(Monster &mob, int count, int playerSeries) {
    // Mobs get simpler patents, mostly attacks
    std::vector<std::pair<std::string, PatentType> > mobPatents = {{"Quick Strike", PatentType::ATTACK}, {"Fury Swipe", PatentType::ATTACK}, {"Poison Bite", PatentType::ATTACK}, {"Minor Heal", PatentType::HEAL}};
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(mobPatents.begin(), mobPatents.end(), g);

    for (int i = 0; i < std::min(count, static_cast<int>(mobPatents.size())); ++i) {
        int basePower = static_cast<int>(T("monster.mob_patent_pow") + playerSeries * T("monster.mob_patent_pow_per"));
        int cost = static_cast<int>(T("monster.mob_patent_cost") + playerSeries * T("monster.mob_patent_cost_per"));

        PatentTarget target = (mobPatents[i].second == PatentType::ATTACK) ? PatentTarget::ENEMY : PatentTarget::SELF;

        auto patent = std::make_shared<Patent>(mobPatents[i].first, 0, mobPatents[i].second, basePower, cost, target);
        mob.addPatent(patent);
    }
}

std::vector<std::unique_ptr<Monster> > MonsterGenerator::generateGroupEncounter(int playerSeries, int &mobCount, int &bossCount, ItemGenerator &itemGen) {
    std::vector<std::unique_ptr<Monster> > encounter;
    std::uniform_int_distribution<> encounterType(1, 100);
    int roll = encounterType(rng_);

    if (roll <= T("enc.single_pct")) {
        // 60% - Single enemy
        mobCount = 1;
        bossCount = 0;
        encounter.push_back(std::make_unique<Monster>(generateMob(playerSeries, itemGen)));
    } else if (roll <= T("enc.single_pct") + T("enc.small_pct")) {
        // 25% - Small group (2-3 mobs, 0.7x power each)
        std::uniform_int_distribution<> groupSize(2, 3);
        mobCount = groupSize(rng_);
        bossCount = 0;

        for (int i = 0; i < mobCount; ++i) {
            Monster mob = generateMob(playerSeries, itemGen);
            // Scale down for group
            mob.maxHealth = static_cast<int>(mob.maxHealth * T("enc.small_scale"));
            mob.health = mob.maxHealth;
            mob.attack = static_cast<int>(mob.attack * T("enc.small_scale"));
            mob.defense = static_cast<int>(mob.defense * T("enc.small_scale"));
            mob.moneyReward = static_cast<int>(mob.moneyReward * T("enc.small_scale"));
            encounter.push_back(std::make_unique<Monster>(std::move(mob)));
        }
    } else if (roll <= T("enc.single_pct") + T("enc.small_pct") + T("enc.large_pct")) {
        // 10% - Large group (4 mobs, 0.6x power each)
        mobCount = 4;
        bossCount = 0;

        for (int i = 0; i < mobCount; ++i) {
            Monster mob = generateMob(playerSeries, itemGen);
            mob.maxHealth = static_cast<int>(mob.maxHealth * T("enc.large_scale"));
            mob.health = mob.maxHealth;
            mob.attack = static_cast<int>(mob.attack * T("enc.large_scale"));
            mob.defense = static_cast<int>(mob.defense * T("enc.large_scale"));
            mob.moneyReward = static_cast<int>(mob.moneyReward * T("enc.large_scale"));
            encounter.push_back(std::make_unique<Monster>(std::move(mob)));
        }
    } else {
        // 5% - Boss with minions
        bossCount = 1;
        std::uniform_int_distribution<> minionCount(2, 3);
        mobCount = minionCount(rng_);

        // Add boss first
        encounter.push_back(std::make_unique<Monster>(generateBoss(playerSeries)));

        // Add scaled minions
        for (int i = 0; i < mobCount; ++i) {
encounter.push_back(generateBossMinion(playerSeries, itemGen, static_cast<float>(T("enc.minion_scale"))));
        }
    }

    return encounter;
}

std::unique_ptr<Monster> MonsterGenerator::generateBossMinion(int playerSeries, ItemGenerator &itemGen, float powerScale) {
    Monster minion = generateMob(playerSeries, itemGen);

    // Scale down significantly
    minion.maxHealth = static_cast<int>(minion.maxHealth * powerScale);
    minion.health = minion.maxHealth;
    minion.attack = static_cast<int>(minion.attack * powerScale);
    minion.defense = static_cast<int>(minion.defense * powerScale);
    minion.speed = static_cast<int>(minion.speed * powerScale);
    minion.moneyReward = static_cast<int>(minion.moneyReward * powerScale);

    // Mark as minion in name
    minion.name = "Minion: " + minion.name;

return std::make_unique<Monster>(std::move(minion));;
}
