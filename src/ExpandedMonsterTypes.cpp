// SPG Engine made by drmrsthemonarch
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

    double diffMult = 1.0;
    switch (difficulty) {
        case MonsterDifficulty::EASY: diffMult = 0.75;
            break;
        case MonsterDifficulty::MEDIUM: diffMult = 1.0;
            break;
        case MonsterDifficulty::HARD: diffMult = 1.35;
            break;
        case MonsterDifficulty::LEGENDARY: diffMult = 1.8;
            break;
    }

    double baseHealth = 50.0 + (playerSeries * 22.0);
    double baseAttack = 8.0 + (playerSeries * 3.5);
    double baseDefense = 5.0 + (playerSeries * 3.0);
    double baseSpeed = 5.0 + (playerSeries * 2.5);
    double baseMoney = 30.0 + (playerSeries * 15.0);

    switch (type) {
        case ExtendedMonsterType::SAFETY_MANAGER:
            stats.maxHealth = static_cast<int>(baseHealth * 1.8);
            stats.attack = static_cast<int>(baseAttack * 0.5);
            stats.defense = static_cast<int>(baseDefense * 10);
            stats.speed = static_cast<int>(baseSpeed * 1);
            stats.moneyReward = static_cast<int>(baseMoney * 2);
            stats.specialMechanic = "SAFETY_AUDIT";
            break;

        case ExtendedMonsterType::CONFERENCE:
            stats.maxHealth = static_cast<int>(baseHealth * 0.9 * diffMult);
            stats.attack = static_cast<int>(baseAttack * 0.8 * diffMult);
            stats.defense = static_cast<int>(baseDefense * 0.9 * diffMult);
            stats.speed = static_cast<int>(baseSpeed * 1.2 * diffMult);
            stats.moneyReward = static_cast<int>(baseMoney * 1.5 * diffMult);
            stats.specialMechanic = "NETWORKING_CHECK";
            break;

        case ExtendedMonsterType::PAPER_WRITING:
            stats.maxHealth = static_cast<int>(baseHealth * 110 * diffMult);
            stats.attack = static_cast<int>(baseAttack * 0.7 * diffMult);
            stats.defense = static_cast<int>(baseDefense * 1.2 * diffMult);
            stats.speed = static_cast<int>(baseSpeed * 0.8 * diffMult);
            stats.moneyReward = static_cast<int>(baseMoney * 1.3 * diffMult);
            stats.specialMechanic = "PEER_REVIEW";
            break;

        default:
            stats.maxHealth = static_cast<int>(baseHealth);
            stats.attack = static_cast<int>(baseAttack);
            stats.defense = static_cast<int>(baseDefense);
            stats.speed = static_cast<int>(baseSpeed);
            stats.moneyReward = static_cast<int>(baseMoney);
            stats.specialMechanic = "NONE";
            break;
    }

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

    double diffMult = 1.0;
    switch (difficulty) {
        case MonsterDifficulty::EASY: diffMult = 0.75; break;
        case MonsterDifficulty::MEDIUM: diffMult = 1.0; break;
        case MonsterDifficulty::HARD: diffMult = 1.35; break;
        case MonsterDifficulty::LEGENDARY: diffMult = 1.8; break;
    }

    double yearScale = 1.0 + (year * 0.5);
    double monthScale = 1.0 + (monthProgression * 0.2);

    data.level = estimatedPlayerSeries + year;
    data.maxHealth = static_cast<int>(150 * yearScale * monthScale * diffMult);
    data.attack = static_cast<int>(20 * yearScale * monthScale * diffMult);
    data.defense = static_cast<int>(15 * yearScale * monthScale * diffMult);
    data.speed = static_cast<int>(12 * yearScale * monthScale * diffMult);
    data.moneyReward = static_cast<int>(100 * yearScale * monthScale * diffMult);
    data.maxHypeEnergy = 50 + (estimatedPlayerSeries * 10);
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
    shadow.maxHealth = player.getMaxRunway() * 1.1;
    shadow.maxHypeEnergy = player.getMaxHypeEnergy() * 1.2; // More energy
    shadow.health = shadow.maxHealth;
    shadow.hypeEnergy = shadow.maxHypeEnergy;
    shadow.attack = player.getHustle() * 1.1;
    shadow.defense = player.getNDAs() * 1.1;
    shadow.speed = player.getLuck() * 1.05;

    // Shadow uses MIRROR strategy (copies player behavior intelligently)
    shadow.setStrategy(std::make_unique<MirrorStrategy>());

    // Shadow player COPIES ALL of the player's patents!
    std::cout << "Shadow Player mirrors your patents:\n";
    for (const auto& patentPair : Patent::characterPatents) {
        const Patent& playerPatent = patentPair.second;

        // Create shadow version with slightly more power
        std::string shadowName = "Shadow " + playerPatent.getName();
        int shadowPower = static_cast<int>(playerPatent.getCurrentPower() * 1.15); // 15% stronger
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

    shadow.moneyReward = player.getMoney() * 2 + 1000000000; // Billion dollar reward
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

    // Safety Managers have high hype energy
    safety.maxHypeEnergy = 70 + (playerSeries * 12);
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
    int patentCount = 3 + (playerSeries / 5);
    patentCount = std::min(patentCount, static_cast<int>(safetyPatents.size()));

    for (int i = 0; i < patentCount; ++i) {
        int basePower = 25 + (playerSeries * 3);
        int cost = 12 + (playerSeries * 2);

        // Debuffs are very powerful for Safety Managers
        if (safetyPatents[i].second == PatentType::DEBUFF) {
            basePower = static_cast<int>(basePower * 1.4); // 40% stronger debuffs
        }

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

    // Conferences have medium-high hype energy
    conference.maxHypeEnergy = 50 + (playerSeries * 8);
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
    int patentCount = 2;
    if (conference.difficulty == MonsterDifficulty::HARD) patentCount = 3;
    if (conference.difficulty == MonsterDifficulty::LEGENDARY) patentCount = 4;

    std::uniform_int_distribution<> selectDist(0, confPatents.size() - 1);
    std::set<int> selectedIndices;

    while (selectedIndices.size() < patentCount && selectedIndices.size() < confPatents.size()) {
        selectedIndices.insert(selectDist(rng_));
    }

    for (int idx : selectedIndices) {
        const auto& [name, type] = confPatents[idx];
        int basePower = 18 + (playerSeries * 2);
        int cost = 9 + (playerSeries * 1);

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

    // Papers have medium hype energy
    paper.maxHypeEnergy = 45 + (playerSeries * 7);
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
    int patentCount = 2;
    if (paper.difficulty == MonsterDifficulty::HARD) patentCount = 3;
    if (paper.difficulty == MonsterDifficulty::LEGENDARY) patentCount = 4;

    std::uniform_int_distribution<> selectDist(0, paperPatents.size() - 1);
    std::set<int> selectedIndices;

    while (selectedIndices.size() < patentCount && selectedIndices.size() < paperPatents.size()) {
        selectedIndices.insert(selectDist(rng_));
    }

    for (int idx : selectedIndices) {
        const auto& [name, type] = paperPatents[idx];
        int basePower = 20 + (playerSeries * 2);
        int cost = 10 + (playerSeries * 1);

        // Debuffs are strong for papers (academic stress is real!)
        if (type == PatentType::DEBUFF) {
            basePower = static_cast<int>(basePower * 1.3);
        }

        PatentTarget target = (type == PatentType::ATTACK || type == PatentType::DEBUFF)
            ? PatentTarget::ENEMY
            : PatentTarget::SELF;

        auto patent = std::make_shared<Patent>(name, 0, type, basePower, cost, target);
        paper.addPatent(patent);
    }
}

bool SpecializedMonsterGenerator::loadWordList(const std::string &filename, std::vector<std::string> &wordList) {
    std::ifstream file(filename);
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
    if (elapsedSeconds < SHADOW_APPEARANCE_TIME) {
        return false;
    }

    // Only appears once per game
    if (alreadyAppeared) {
        return false;
    }

    // 1 in 1 billion chance each check
    std::uniform_int_distribution<long long> dist(1, SHADOW_CHANCE);
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

    crony.maxHealth = static_cast<int>(stats.maxHealth * 0.4);
    crony.health = crony.maxHealth;
    crony.attack = static_cast<int>(stats.attack * 0.4);
    crony.defense = static_cast<int>(stats.defense * 0.6); // Higher defense
    crony.speed = static_cast<int>(stats.speed * 0.4);
    crony.moneyReward = static_cast<int>(stats.moneyReward * 0.3);
    crony.itemReward = "Safety Memo";

    // Limited hype energy
    crony.maxHypeEnergy = 30 + (playerSeries * 5);
    crony.hypeEnergy = crony.maxHypeEnergy;

    // Simple aggressive strategy
    crony.setStrategy(std::make_unique<AggressiveStrategy>());

    // 1 weak debuff patent
    std::vector<std::pair<std::string, PatentType>> cronyPatents = {
        {"Safety Citation", PatentType::DEBUFF}
    };

    int basePower = 15 + (playerSeries * 2);
    int cost = 8 + playerSeries;

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