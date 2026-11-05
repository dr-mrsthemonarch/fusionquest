// SPG Engine made by drmrsthemonarch
#include "../include/GameMath.h"
#include "../include/Tuning.h"
#include "../include/ConfigStore.h"
#include "../include/ExpandedMonsterTypes.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include "../include/Patent.h"
#include "../include/PatentStrategy.h"

constexpr double SECONDS_PER_YEAR = 365.25 * 24 * 60 * 60;
constexpr double SECONDS_PER_MONTH = SECONDS_PER_YEAR / 12.0;

SpecializedMonsterGenerator::SpecializedStats SpecializedMonsterGenerator::calculateSpecializedStats(int playerSeries, double elapsedSeconds, ExtendedMonsterType type, MonsterDifficulty difficulty) {
    SpecializedStats stats;

    const double diffMult = GameMath::difficultyMult(static_cast<int>(difficulty));
    int kind = -1;
    switch (type) {
        case ExtendedMonsterType::SAFETY_MANAGER: kind = 0; stats.specialMechanic = "SAFETY_AUDIT"; break;
        case ExtendedMonsterType::CONFERENCE: kind = 1; stats.specialMechanic = "NETWORKING_CHECK"; break;
        case ExtendedMonsterType::PAPER_WRITING: kind = 2; stats.specialMechanic = "PEER_REVIEW"; break;
        default: stats.specialMechanic = "NONE"; break;
    }
    const GameMath::EnemyStats e = GameMath::special(kind, playerSeries, diffMult);
    stats.maxHealth = static_cast<int>(e.hp);
    stats.attack = static_cast<int>(e.atk);
    stats.defense = static_cast<int>(e.def);
    stats.speed = static_cast<int>(e.spd);
    stats.moneyReward = static_cast<int>(e.money);

    stats.health = stats.maxHealth;
    return stats;
}

double ReviewCalendarSystem::calculateReviewScheduleTime(int year, int month) const {
    // Boss appears in the middle of each month
    double yearStart = (year - 1) * SECONDS_PER_YEAR;
    double monthOffset = (month - 1) * SECONDS_PER_MONTH;
    double midMonth = SECONDS_PER_MONTH / 2.0;

    return yearStart + monthOffset + midMonth;
}

ScheduledBoss *ReviewCalendarSystem::checkForScheduledReview(double elapsedSeconds) {
    // Find the next unencountered boss that should have appeared by now
    for (auto &scheduledBoss: reviewCalendar_) {
        if (!scheduledBoss.encountered && elapsedSeconds >= scheduledBoss.scheduledTime) {
            // Check if we're within the month window (give 1 month grace period)
            double timeSinceScheduled = elapsedSeconds - scheduledBoss.scheduledTime;
            if (timeSinceScheduled < SECONDS_PER_MONTH) {
                return &scheduledBoss;
            }
        }
    }
    return nullptr;
}

void ReviewCalendarSystem::debugPrintCalendar() const {
    std::cout << "\n=== BOSS CALENDAR DEBUG ===" << std::endl;
    for (size_t i = 0; i < std::min(size_t(24), reviewCalendar_.size()); ++i) {
        const auto &scheduled = reviewCalendar_[i];
        std::cout << "Year " << scheduled.year
                  << " Month " << scheduled.month
                  << ": " << scheduled.bossData.name
                  << " (HP: " << scheduled.bossData.maxHealth
                  << ", ATK: " << scheduled.bossData.attack << ")" << std::endl;
    }
    std::cout << "... (and " << (reviewCalendar_.size() - 24) << " more bosses)" << std::endl;
}

BossData ReviewCalendarSystem::generateMonthlyReviewData(int year, int month, int playerSeries) {
    BossData data;

    // Random difficulty
    std::uniform_int_distribution<> diffDist(0, 1000);
    int roll = diffDist(rng_);
    MonsterDifficulty difficulty;
    if (roll < 400) difficulty = MonsterDifficulty::EASY;
    else if (roll < 700) difficulty = MonsterDifficulty::MEDIUM;
    else if (roll < 990) difficulty = MonsterDifficulty::HARD;
    else difficulty = MonsterDifficulty::LEGENDARY;

    // Generate name
    std::vector<std::string> monthNames = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    data.name = "Performance Review: " + monthNames[month - 1] + " Year " + std::to_string(year);
    data.type = MonsterType::REVIEW;
    data.difficulty = difficulty;

    // Calculate stats (same as before)
    int estimatedPlayerSeries = playerSeries + (year - 1);
    double monthProgression = month / 12.0;

    double diffMult = GameMath::difficultyMult(static_cast<int>(difficulty));

    const GameMath::EnemyStats rb = GameMath::review(year, month, diffMult);
    data.level = GameMath::reviewLevel(playerSeries, year);
    data.maxHealth = static_cast<int>(rb.hp);
    data.attack = static_cast<int>(rb.atk);
    data.defense = static_cast<int>(rb.def);
    data.speed = static_cast<int>(rb.spd);
    data.moneyReward = static_cast<int>(rb.money);
    data.maxHypeEnergy = GameMath::reviewHype(estimatedPlayerSeries);
    data.itemReward = "EQUIPMENT";

    return data;
}

Monster ShadowPlayerSystem::generateShadowPlayer(const Character &player, double elapsedSeconds) {
    Monster shadow;

    shadow.type = MonsterType::BOSS;
    shadow.difficulty = MonsterDifficulty::LEGENDARY;

    std::string reversedName = reverseString(player.getName());
    std::string reversedCompany = reverseString(player.getStartup());

    shadow.name = "Shadow: " + reversedName + " of " + reversedCompany;
    shadow.level = player.getSeries();

    // EXACT REPLICA of player stats (slightly buffed)
    const GameMath::ShadowStats ss = GameMath::shadow(player.getMaxRunway(), player.getMaxHypeEnergy(), player.getHustle(),
                                                      player.getNDAs(), player.getAgility());
    shadow.maxHealth = ss.hp;
    shadow.maxHypeEnergy = ss.hype;
    shadow.health = shadow.maxHealth;
    shadow.hypeEnergy = shadow.maxHypeEnergy;
    shadow.attack = ss.atk;
    shadow.defense = ss.def;
    shadow.speed = ss.spd;

    // Shadow uses MIRROR strategy (copies player behavior intelligently)
    shadow.setStrategy(std::make_unique<MirrorStrategy>());

    // Shadow player COPIES ALL of the player's patents!
    std::cout << "Shadow Player mirrors your patents:\n";
    for (const auto& patentPair : Patent::characterPatents) {
        const Patent& playerPatent = patentPair.second;

        // Create shadow version with slightly more power
        std::string shadowName = "Shadow " + playerPatent.getName();
        int shadowPower = GameMath::shadowPatentPower(playerPatent.getCurrentPower());
        int shadowCost = playerPatent.getCost();

        auto shadowPatent = std::make_shared<Patent>(
            shadowName,
            0,
            playerPatent.getType(),
            shadowPower,
            shadowCost,
            playerPatent.getTarget()
        );
        shadow.addPatent(shadowPatent);

        std::cout << "  - " << shadowName << " ("
                  << playerPatent.getTypeName() << ", Power: " << shadowPower << ")\n";
    }

    shadow.moneyReward = GameMath::shadowReward(player.getMoney());
    shadow.itemReward = "SHADOW_EQUIPMENT";

    return shadow;
}

void ReviewCalendarSystem::generateReviewCalendar(int startingPlayerSeries) {
    std::cout << "\n📅 Generating Performance reviews for 20 years..." << std::endl;

    for (int year = 1; year <= 20; ++year) {
        for (int month = 1; month <= 12; ++month) {
            ScheduledBoss scheduled;
            scheduled.year = year;
            scheduled.month = month;
            scheduled.scheduledTime = calculateReviewScheduleTime(year, month);
            scheduled.bossData = generateMonthlyReviewData(year, month, startingPlayerSeries);
            scheduled.encountered = false;
            scheduled.defeated = false;

            reviewCalendar_.push_back(std::move(scheduled));  // Use move
        }
    }

    std::cout << "✅ Generated 240 monthly performance reviews" << std::endl;
}

Monster SpecializedMonsterGenerator::generateSafetyManager(int playerSeries, double elapsedSeconds) {
    Monster safety;
    safety.type = MonsterType::MOB;
    safety.difficulty = MonsterDifficulty::LEGENDARY;

    if (!safetyWords_.empty()) {
        std::uniform_int_distribution<> dist(0, safetyWords_.size() - 1);
        safety.name = "🛡️ Safety Manager: " + safetyWords_[dist(rng_)];
    } else {
        safety.name = "🛡️ Safety Manager";
    }

    safety.level = playerSeries;
    auto stats = calculateSpecializedStats(playerSeries, elapsedSeconds,
                                          ExtendedMonsterType::SAFETY_MANAGER,
                                          MonsterDifficulty::LEGENDARY);

    safety.maxHealth = stats.maxHealth;
    safety.health = stats.health;
    safety.attack = stats.attack;
    safety.defense = stats.defense;
    safety.speed = stats.speed;
    safety.moneyReward = stats.moneyReward;
    safety.itemReward = "Safety Certificate";
    safety.itemValue = GameMath::specialItemValue(0, playerSeries);

    // Safety Managers have high hype energy
    safety.maxHypeEnergy = GameMath::safetyHype(playerSeries);
    safety.hypeEnergy = safety.maxHypeEnergy;

    // Use DEBUFF strategy - they weaken the player!
    safety.setStrategy(std::make_unique<DebuffStrategy>());

    // Generate safety-themed debuff patents
    generateSafetyManagerPatents(safety, playerSeries);

    return safety;
}

void SpecializedMonsterGenerator::generateSafetyManagerPatents(Monster& safety, int playerSeries) {
    // Safety Managers specialize in debuffs and defense
    std::vector<std::pair<std::string, PatentType>> safetyPatents = {
        {"Regulatory Compliance Check", PatentType::DEBUFF},    // Reduces player hustle
        {"Safety Audit", PatentType::DEBUFF},                   // Reduces player agility
        {"Risk Assessment", PatentType::DEBUFF},                // Reduces player NDAs
        {"Compliance Shield", PatentType::DEFENSE},             // Self defense
        {"Policy Enforcement", PatentType::DEBUFF}              // Reduces multiple stats
    };

    // Give 3-4 patents
    const int patentCount = GameMath::safetyPatentCount(playerSeries, static_cast<int>(safetyPatents.size()));

    for (int i = 0; i < patentCount; ++i) {
        const GameMath::PatentSpec ps = GameMath::safetyPatent(playerSeries, safetyPatents[i].second == PatentType::DEBUFF);
        int basePower = ps.power, cost = ps.cost;

        PatentTarget target = (safetyPatents[i].second == PatentType::DEBUFF)
            ? PatentTarget::ENEMY
            : PatentTarget::SELF;

        auto patent = std::make_shared<Patent>(
            safetyPatents[i].first,
            0,
            safetyPatents[i].second,
            basePower,
            cost,
            target
        );
        safety.addPatent(patent);
    }
}

Monster SpecializedMonsterGenerator::generateConference(int playerSeries, double elapsedSeconds) {
    Monster conference;
    conference.type = MonsterType::MOB;

    // Random difficulty
    std::uniform_int_distribution<> diffDist(0, 100);
    int roll = diffDist(rng_);
    if (roll < 50) conference.difficulty = MonsterDifficulty::EASY;
    else if (roll < 80) conference.difficulty = MonsterDifficulty::MEDIUM;
    else if (roll < 95) conference.difficulty = MonsterDifficulty::HARD;
    else conference.difficulty = MonsterDifficulty::LEGENDARY;

    if (!conferenceWords_.empty()) {
        std::uniform_int_distribution<> dist(0, conferenceWords_.size() - 1);
        conference.name = "🎤 Conference: " + conferenceWords_[dist(rng_)];
    } else {
        conference.name = "🎤 Tech Conference";
    }

    conference.level = playerSeries;
    auto stats = calculateSpecializedStats(playerSeries, elapsedSeconds,
                                          ExtendedMonsterType::CONFERENCE,
                                          conference.difficulty);

    conference.maxHealth = stats.maxHealth;
    conference.health = stats.health;
    conference.attack = stats.attack;
    conference.defense = stats.defense;
    conference.speed = stats.speed;
    conference.moneyReward = stats.moneyReward;
    conference.itemReward = "Conference Badge";
    conference.itemValue = GameMath::specialItemValue(1, playerSeries);

    // Conferences have medium-high hype energy
    conference.maxHypeEnergy = GameMath::conferenceHype(playerSeries);
    conference.hypeEnergy = conference.maxHypeEnergy;

    // Conferences use balanced strategy with buffs and debuffs
    conference.setStrategy(std::make_unique<BalancedStrategy>());

    // Generate conference-themed patents (buffs and social debuffs)
    generateConferencePatents(conference, playerSeries);

    return conference;
}

void SpecializedMonsterGenerator::generateConferencePatents(Monster& conference, int playerSeries) {
    // Conferences mix buffs (networking) and debuffs (social pressure)
    std::vector<std::pair<std::string, PatentType>> confPatents = {
        {"Networking Boost", PatentType::BUFF},          // Buff self
        {"Keynote Speech", PatentType::ATTACK},          // Attack
        {"Social Pressure", PatentType::DEBUFF},         // Debuff enemy synergy
        {"Panel Discussion", PatentType::DEBUFF},        // Debuff enemy luck
        {"Coffee Break", PatentType::HEAL}               // Self heal
    };

    // Give 2-3 patents based on difficulty
    const int patentCount = GameMath::specialPatentCount(static_cast<int>(conference.difficulty));

    std::uniform_int_distribution<> selectDist(0, confPatents.size() - 1);
    std::set<int> selectedIndices;

    while (selectedIndices.size() < patentCount && selectedIndices.size() < confPatents.size()) {
        selectedIndices.insert(selectDist(rng_));
    }

    for (int idx : selectedIndices) {
        const auto& [name, type] = confPatents[idx];
        const GameMath::PatentSpec ps = GameMath::conferencePatent(playerSeries);
        int basePower = ps.power, cost = ps.cost;

        PatentTarget target;
        if (type == PatentType::ATTACK || type == PatentType::DEBUFF) {
            target = PatentTarget::ENEMY;
        } else {
            target = PatentTarget::SELF;
        }

        auto patent = std::make_shared<Patent>(name, 0, type, basePower, cost, target);
        conference.addPatent(patent);
    }
}

Monster SpecializedMonsterGenerator::generatePaperWriting(int playerSeries, double elapsedSeconds) {
    Monster paper;
    paper.type = MonsterType::MOB;

    // Random difficulty
    std::uniform_int_distribution<> diffDist(0, 100);
    int roll = diffDist(rng_);
    if (roll < 50) paper.difficulty = MonsterDifficulty::EASY;
    else if (roll < 80) paper.difficulty = MonsterDifficulty::MEDIUM;
    else if (roll < 95) paper.difficulty = MonsterDifficulty::HARD;
    else paper.difficulty = MonsterDifficulty::LEGENDARY;

    if (!paperWords_.empty()) {
        std::uniform_int_distribution<> dist(0, paperWords_.size() - 1);
        paper.name = "📄 Paper Deadline: " + paperWords_[dist(rng_)];
    } else {
        paper.name = "📄 Research Paper";
    }

    paper.level = playerSeries;
    auto stats = calculateSpecializedStats(playerSeries, elapsedSeconds,
                                          ExtendedMonsterType::PAPER_WRITING,
                                          paper.difficulty);

    paper.maxHealth = stats.maxHealth;
    paper.health = stats.health;
    paper.attack = stats.attack;
    paper.defense = stats.defense;
    paper.speed = stats.speed;
    paper.moneyReward = stats.moneyReward;
    paper.itemReward = "Published Paper";
    paper.itemValue = GameMath::specialItemValue(3, playerSeries);  // costs money when sold

    // Papers have medium hype energy
    paper.maxHypeEnergy = GameMath::paperHype(playerSeries);
    paper.hypeEnergy = paper.maxHypeEnergy;

    // Papers use debuff strategy (stress and exhaustion)
    paper.setStrategy(std::make_unique<DebuffStrategy>());

    // Generate paper-themed debuff patents
    generatePaperPatents(paper, playerSeries);

    return paper;
}

void SpecializedMonsterGenerator::generatePaperPatents(Monster& paper, int playerSeries) {
    // Papers specialize in debuffs (stress, exhaustion) and some defense
    std::vector<std::pair<std::string, PatentType>> paperPatents = {
        {"Peer Review Critique", PatentType::DEBUFF},     // Reduces confidence/synergy
        {"Deadline Pressure", PatentType::DEBUFF},        // Reduces agility
        {"Citation Needed", PatentType::DEBUFF},          // Reduces hustle
        {"Revision Request", PatentType::DEBUFF},         // Multiple stat reduction
        {"Writer's Block", PatentType::DEFENSE},          // Self defense
        {"Literature Review", PatentType::ATTACK}         // Attack
    };

    // Give 2-3 patents
    const int patentCount = GameMath::specialPatentCount(static_cast<int>(paper.difficulty));

    std::uniform_int_distribution<> selectDist(0, paperPatents.size() - 1);
    std::set<int> selectedIndices;

    while (selectedIndices.size() < patentCount && selectedIndices.size() < paperPatents.size()) {
        selectedIndices.insert(selectDist(rng_));
    }

    for (int idx : selectedIndices) {
        const auto& [name, type] = paperPatents[idx];
        const GameMath::PatentSpec ps = GameMath::paperPatent(playerSeries, type == PatentType::DEBUFF);
        int basePower = ps.power, cost = ps.cost;


        PatentTarget target = (type == PatentType::ATTACK || type == PatentType::DEBUFF)
            ? PatentTarget::ENEMY
            : PatentTarget::SELF;

        auto patent = std::make_shared<Patent>(name, 0, type, basePower, cost, target);
        paper.addPatent(patent);
    }
}

bool SpecializedMonsterGenerator::loadWordList(const std::string &filename, std::vector<std::string> &wordList) {
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

    return !wordList.empty();
}

bool SpecializedMonsterGenerator::loadWordLists(const std::string &configPath) {
    bool allLoaded = true;

    allLoaded &= loadWordList(configPath + "safety_managers.txt", safetyWords_);
    allLoaded &= loadWordList(configPath + "conferences.txt", conferenceWords_);
    allLoaded &= loadWordList(configPath + "papers.txt", paperWords_);

    return allLoaded;
}

std::string ShadowPlayerSystem::reverseString(const std::string &str) {
    std::string reversed = str;
    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

ReviewCalendarSystem::ReviewCalendarSystem() : rng_(std::random_device{}()) {
    reviewCalendar_.reserve(240); // 12 bosses × 20 years
}

void ReviewCalendarSystem::recordReviewEncounter(int year, int month, bool defeated) {
    for (auto &scheduledBoss: reviewCalendar_) {
        if (scheduledBoss.year == year && scheduledBoss.month == month) {
            scheduledBoss.encountered = true;
            scheduledBoss.defeated = defeated;
            break;
        }
    }
}

ShadowPlayerSystem::ShadowPlayerSystem() : rng_(std::random_device{}()) {
}

bool ShadowPlayerSystem::shouldShadowPlayerAppear(double elapsedSeconds, bool alreadyAppeared) {
    // Only appears after 10 years
    if (elapsedSeconds < T("shadow.appear_years") * 365.25 * 24 * 3600) {
        return false;
    }

    // Only appears once per game
    if (alreadyAppeared) {
        return false;
    }

    // 1 in 1 billion chance each check
    std::uniform_int_distribution<long long> dist(1, std::max(1LL, static_cast<long long>(T("shadow.odds"))));
    return dist(rng_) == 1;
}

SpecializedMonsterGenerator::SpecializedMonsterGenerator() : rng_(std::random_device{}()) {
}

Monster SpecializedMonsterGenerator::generateSafetyCrony(int playerSeries, int index) {
    Monster crony;
    crony.type = MonsterType::MOB;
    crony.difficulty = MonsterDifficulty::MEDIUM;
    crony.name = "🛡️ Safety Crony #" + std::to_string(index);
    crony.level = playerSeries;

    // 40% of Safety Manager power
    auto stats = calculateSpecializedStats(playerSeries, 0,
                                          ExtendedMonsterType::SAFETY_MANAGER,
                                          MonsterDifficulty::MEDIUM);

    crony.maxHealth = static_cast<int>(stats.maxHealth * GameMath::cronyMult(0));
    crony.health = crony.maxHealth;
    crony.attack = static_cast<int>(stats.attack * GameMath::cronyMult(0));
    crony.defense = static_cast<int>(stats.defense * GameMath::cronyMult(1)); // Higher defense
    crony.speed = static_cast<int>(stats.speed * GameMath::cronyMult(0));
    crony.moneyReward = static_cast<int>(stats.moneyReward * GameMath::cronyMult(2));
    crony.itemReward = "Safety Memo";
    crony.itemValue = GameMath::specialItemValue(2, playerSeries);

    // Limited hype energy
    crony.maxHypeEnergy = GameMath::cronyHype(playerSeries);
    crony.hypeEnergy = crony.maxHypeEnergy;

    // Simple aggressive strategy
    crony.setStrategy(std::make_unique<AggressiveStrategy>());

    // 1 weak debuff patent
    std::vector<std::pair<std::string, PatentType>> cronyPatents = {
        {"Safety Citation", PatentType::DEBUFF}
    };

    const GameMath::PatentSpec cps = GameMath::cronyPatent(playerSeries);
    int basePower = cps.power, cost = cps.cost;

    auto patent = std::make_shared<Patent>(
        cronyPatents[0].first, 0, cronyPatents[0].second,
        basePower, cost, PatentTarget::ENEMY
    );
    crony.addPatent(patent);

    return crony;
}

std::vector<Monster> SpecializedMonsterGenerator::generateSafetyManagerEncounter(
    int playerSeries, double elapsedSeconds
) {
    std::vector<Monster> encounter;

    // Generate main Safety Manager
    encounter.push_back(generateSafetyManager(playerSeries, elapsedSeconds));

    // Add 2 Safety Cronies
    encounter.push_back(generateSafetyCrony(playerSeries, 1));
    encounter.push_back(generateSafetyCrony(playerSeries, 2));

    return encounter;
}

std::vector<ReviewCalendarSystem::ReviewFlag> ReviewCalendarSystem::exportFlags() const {
    std::vector<ReviewFlag> flags;
    flags.reserve(reviewCalendar_.size());
    for (const auto& sb : reviewCalendar_) {
        flags.push_back({sb.year, sb.month, sb.encountered, sb.defeated});
    }
    return flags;
}

void ReviewCalendarSystem::restoreFlags(const std::vector<ReviewFlag>& flags) {
    for (const auto& fl : flags) {
        for (auto& sb : reviewCalendar_) {
            if (sb.year == fl.year && sb.month == fl.month) {
                sb.encountered = fl.encountered;
                sb.defeated = fl.defeated;
                break;
            }
        }
    }
}