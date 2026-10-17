// ExpandedMonsterTypes.h
#ifndef EXPANDEDMONSTERTYPES_H
#define EXPANDEDMONSTERTYPES_H

#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include "Monster.h"
#include "Character.h"

// Extended Monster Types
enum class ExtendedMonsterType {
    REGULAR_MOB,        // Standard combat
    SAFETY_MANAGER,     // Always legendary, regular mob
    CONFERENCE,         // Synergy + Luck based
    PAPER_WRITING,      // Agility + NDAs based
    BOSS,               // Monthly scheduled boss
    SHADOW_PLAYER,      // 1 in 1 billion chance, exact player replica
    NEMESIS             // Recurring yearly nemesis
};

// Boss Calendar Entry (12 bosses per year, one per month)
struct BossData {
    std::string name;
    int level;
    int maxHealth;
    int attack;
    int defense;
    int speed;
    int moneyReward;
    std::string itemReward;
    MonsterType type;
    MonsterDifficulty difficulty;
    int maxHypeEnergy;
};

struct ScheduledBoss {
    int year;
    int month;
    double scheduledTime;
    BossData bossData;  // CHANGED from: Monster boss
    bool encountered;
    bool defeated;
};

// ============================================================================
// SHADOW PLAYER SYSTEM
// ============================================================================

class ShadowPlayerSystem {
public:
    ShadowPlayerSystem();

    // Check if shadow player should appear (1 in 1 billion chance, only after 10 years)
    bool shouldShadowPlayerAppear(double elapsedSeconds, bool alreadyAppeared);

    // Generate exact replica of player as a monster
    Monster generateShadowPlayer(const Character& player, double elapsedSeconds);

    // Reverse a string (for name and company)
    static std::string reverseString(const std::string& str);

private:
    std::mt19937 rng_;
    static constexpr double SHADOW_APPEARANCE_TIME = 10.0 * 365.25 * 24 * 3600; // 10 years
    static constexpr long long SHADOW_CHANCE = 1000000000; // 1 in 1 billion
};

class ReviewCalendarSystem {
public:
    ReviewCalendarSystem();

    void generateReviewCalendar(int startingPlayerSeries);

    ScheduledBoss* checkForScheduledReview(double elapsedSeconds);

    void recordReviewEncounter(int year, int month, bool defeated);

    // Get all scheduled bosses for debugging
    std::vector<ScheduledBoss> getAllBosses() const { return reviewCalendar_; }

    // DEBUG: Show boss calendar
    void debugPrintCalendar() const;
    struct ReviewFlag { int year; int month; bool encountered; bool defeated; };
    std::vector<ReviewFlag> exportFlags() const;
    // restoreFlags: call AFTER generateReviewCalendar() has rebuilt the 240 entries
    // (post-load, clock reset to elapsed=0). matches by year+month, sets encountered/defeated.
    void restoreFlags(const std::vector<ReviewFlag>& flags);

private:
    std::vector<ScheduledBoss> reviewCalendar_;
    std::mt19937 rng_;

    BossData generateMonthlyReviewData(int year, int month, int playerSeries);
    double calculateReviewScheduleTime(int year, int month) const;
};

class SpecializedMonsterGenerator {
public:
    SpecializedMonsterGenerator();
    std::vector<Monster> generateSafetyManagerEncounter(int playerSeries, double elapsedSeconds);
    Monster generateSafetyCrony(int playerSeries, int index);
    Monster generateSafetyManager(int playerSeries, double elapsedSeconds);
    Monster generateConference(int playerSeries, double elapsedSeconds);
    Monster generatePaperWriting(int playerSeries, double elapsedSeconds);

    void generateSafetyManagerPatents(Monster &monster, int playerSeries);

    void generateConferencePatents(Monster &monster, int playerSeries);

    void generatePaperPatents(Monster &monster, int playerSeries);

    bool loadWordLists(const std::string& configPath);

private:
    std::mt19937 rng_;
    std::vector<std::string> safetyWords_;
    std::vector<std::string> conferenceWords_;
    std::vector<std::string> paperWords_;

    bool loadWordList(const std::string& filename, std::vector<std::string>& wordList);

    // Scaling helpers
    struct SpecializedStats {
        int health;
        int maxHealth;
        int maxHypenergy;
        int attack;
        int defense;
        int speed;
        int moneyReward;
        std::string specialMechanic;
    };

    SpecializedStats calculateSpecializedStats(int playerSeries, double elapsedSeconds,
                                                ExtendedMonsterType type, MonsterDifficulty difficulty);
};

#endif // EXPANDEDMONSTERTYPES_H