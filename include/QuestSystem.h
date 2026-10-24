#ifndef QUESTSYSTEM_H
#define QUESTSYSTEM_H

#include <functional>
#include <random>
#include <string>
#include <vector>
#include "Quest.h"
#include "QuestGenerator.h"

// quest tree + track switching. engine feeds events, system advances sprint->dev->quest.
// only the ACTIVE track's CURRENT sprint counts events.
class QuestSystem {
public:
    using LogFn = std::function<void(const std::string&)>;
    using RewardFn = std::function<void(bool isMain, const std::string& title)>;

    struct Gauge { std::string title; float frac = 0.0f; };

    bool loadConfig(const std::string& root);
    void setCallbacks(LogFn log, RewardFn reward) { logFn_ = std::move(log); rewardFn_ = std::move(reward); }
    void start(double elapsed);         // new game: quest 1
    void tick(double elapsed);          // final-quest clock rules, track upkeep

    // events
    void onKill(const std::string& name, bool isBoss);
    void onItem(const std::string& name);
    void onTownVisit();

    // bias feed (current sprint goals)
    std::vector<std::string> killKeywords();
    std::vector<std::string> collectKeywords();
    const QuestBalance& balance() const { return gen_.balance(); }
    void setTimeScale(double s) { gen_.setTimeScale(s); }
    QuestGenerator::QuestPreview preview(int index, double elapsed) { return gen_.preview(index, elapsed); }

    // UI
    const std::string& preamble() const { return gen_.preamble(); }
    Gauge mainGauge() const;
    Gauge hustleGauge() const;
    Gauge devGauge() const;
    const Quest* activeQuest() const;
    const Dev* activeDev() const;
    int activeCount() const;
    int maxActive() const { return 1 + static_cast<int>(gen_.balance().maxOpenHustles); }
    int completedCount() const { return state_.completedQuests; }

    // final quest popup
    bool consumeFinalWarning() { bool w = warn_; warn_ = false; return w; }
    bool awaitingAck() const { return awaitAck_; }
    void acknowledgeFinal() { awaitAck_ = false; }

    // save/load
    QuestState& state() { return state_; }
    const QuestState& state() const { return state_; }
    void restored() { awaitAck_ = false; warn_ = false; }

private:
    QuestGenerator gen_;
    QuestState state_;
    std::mt19937 rng_{std::random_device{}()};
    LogFn logFn_;
    RewardFn rewardFn_;
    double elapsed_ = 0.0;
    bool awaitAck_ = false, warn_ = false;

    void log(const std::string& s) { if (logFn_) logFn_(s); }
    Quest* active();
    std::vector<Goal*> activeGoals();
    void beginMain(int index);
    void startFinal();
    void afterProgress();
    void finishActive();
    void completeMain();
    void chooseTrack(bool excludeCurrent);
};

#endif
