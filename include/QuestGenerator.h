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
    double timeScale = 1.0;        // debug clock: game seconds per real second
    double goalScale = 1.0;        // multiplies every goal target
    double maxSprints = 50000;     // cap per quest
};

class QuestGenerator {
public:
    QuestGenerator();
    bool loadConfig(const std::string& root); // root = "config/"
    const QuestBalance& balance() const { return bal_; }
    void setTimeScale(double s) { bal_.timeScale = s > 1e-9 ? s : 1e-9; }
    const std::string& preamble() const { return preamble_; }

    double mainBudget(int index, double elapsed) const;    // seconds
    double hustleBudget(int index, double elapsed) const;  // seconds per hustle
    int hustlesRequired(int index) const;

    Quest generateMain(int index, double elapsed);
    Quest generateHustle(int mainIndex, double budgetSeconds);

    void refresh();                            // reload factors from Tuning (call every tick)
    struct QuestPreview { double budget = 0, hustleBudget = 0, avgSprintSeconds = 0; int devs = 0, hustlesRequired = 0; long long sprints = 0; };
    QuestPreview preview(int index, double elapsed);   // for the tuner

    Sprint makeSprint(int tier);               // small goals, scale with tier
    void refill(Dev& d);                       // keep current + preview sprint generated
    double estimateSeconds(const Sprint& s) const;

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
