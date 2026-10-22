#ifndef QUESTGENERATOR_H
#define QUESTGENERATOR_H

#include <random>
#include <string>
#include <vector>
#include "Quest.h"

// all tunables, loaded from config/quests/balance.txt
struct QuestBalance {
    double totalSeconds = 20.0 * 365 * 86400;
    double usableFraction = 0.95;
    double growth = 0.15;          // per-quest-index weight growth (later quests get more time)
    double mainShare = 0.6;        // of a main quest budget, share spent on main devs (rest = hustles)
    double mainQuestCount = 20;    // normal mains; final = count+1
    double secPerKill = 40, secPerCollect = 60, secPerBoss = 900, secPerTown = 3600;
    double killBias = 0.15, collectBias = 0.15;
    double switchChance = 0.35, newHustleWeight = 1.0, maxOpenHustles = 3;
    double rewardMainPct = 25, rewardHustlePct = 10;
    double finalWarningDays = 182.5, finalForceDays = 1.0;
};

class QuestGenerator {
public:
    QuestGenerator();
    bool loadConfig(const std::string& root); // root = "config/"
    const QuestBalance& balance() const { return bal_; }
    const std::string& preamble() const { return preamble_; }

    double mainBudget(int index, double elapsed) const;    // seconds
    double hustleBudget(int index, double elapsed) const;  // seconds per hustle
    int hustlesRequired(int index) const;

    Quest generateMain(int index, double elapsed);
    Quest generateHustle(int mainIndex, double budgetSeconds);

private:
    struct MainDef { std::string title, story; };
    QuestBalance bal_;
    std::string preamble_;
    std::vector<MainDef> mainDefs_;
    std::vector<std::string> hustleNames_, devNames_, sprintNames_, killKw_, collectKw_;
    std::mt19937 rng_{std::random_device{}()};

    Quest build(int index, double budgetSeconds);
    std::string pick(const std::vector<std::string>& v);
    GoalType rollType();
    double secPer(GoalType t) const;
};

#endif
