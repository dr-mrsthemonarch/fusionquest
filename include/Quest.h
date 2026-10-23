#ifndef QUEST_H
#define QUEST_H

#include <string>
#include <vector>

enum class GoalType { KILL, COLLECT, BOSS, TOWN };

struct Goal {
    GoalType type = GoalType::KILL;
    std::string keyword;       // substring match on monster / item name (case-insens)
    long long target = 1;
    long long progress = 0;
    bool done() const { return progress >= target; }
    float fraction() const;
    std::string describe() const; // "Defeat Dragon 3/10"
};

struct Sprint {
    std::string name;
    std::vector<Goal> goals;
    bool done() const;
    float fraction() const;
};

// sprints are generated lazily: only shown[0] (current) and shown[1] (preview) exist.
struct Dev {
    std::string name;
    long long sprintCount = 0;  // total sprints in this dev
    long long current = 0;      // sprints finished
    int tier = 1;               // goal size scale (quest index)
    std::vector<Sprint> shown;
    bool done() const { return current >= sprintCount; }
    float fraction() const;
};

// main quest or side hustle. dev = progress counter of the quest.
struct Quest {
    std::string title, story;
    int index = 0;
    int hustlesRequired = 0;   // main only: hustles to finish before main can finish
    std::vector<Dev> devs;
    size_t current = 0;
    bool done() const { return current >= devs.size(); }
};

struct QuestState {
    Quest main;
    bool mainActive = false;
    int mainIndex = 0;          // 1..20 normal, 21 final
    int hustlesDone = 0;        // hustles finished during current main
    double hustleBudget = 0.0;  // seconds budget per new hustle
    std::vector<Quest> hustles; // open hustles
    int track = 0;              // 0 = main, i+1 = hustles[i]
    int completedQuests = 0;
    bool finalPending = false;
    bool finalStarted = false;
};

#endif
