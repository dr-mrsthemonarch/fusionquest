#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <memory>
#include <set>
#include <random>
#include <string>

#include "Character.h"
#include "Monster.h"
#include "MonsterGenerator.h"
#include "EquipmentGenerator.h"
#include "ItemGenerator.h"
#include "NemesisSystem.h"
#include "ExpandedMonsterTypes.h" // SpecializedMonsterGenerator, ReviewCalendarSystem, ShadowPlayerSystem
#include "SpecialDropsLibrary.h"
#include "NamedMonsterLibrary.h"
#include "NamedMonster.h"
#include "BattleEncounter.h"
#include "BattleLog.h"
#include "StdoutSink.h"
#include "QuestSystem.h"

// headless. no Qt/ftxui deps. UI polls state each render frame.
// clock: caller feeds elapsedSeconds (from ftxui CountdownComponent), engine
// stores no wall-clock of its own except lastNemesisTime (elapsed-seconds space).
class GameEngine {
public:
    explicit GameEngine(bool debugInstakill = false);
    ~GameEngine() { StdoutSink::setLineHandler(nullptr); }

    // setup
    bool loadConfig(const std::string& configRoot = "config/");
    void initializeCharacter(const std::string& name, const std::string& employeeNr,
                              const std::string& position, const std::string& startup);

    // main update, call every render frame with current run elapsed seconds
    void tick(double elapsedSeconds);

    bool isInitialized() const { return initialized_; }

    // bumps on every initializeCharacter() (new game / load baseline). main loop watches this
    // to restart the master clock.
    int getInitGeneration() const { return initGeneration_; }

    // identity fields Character doesn't expose
    const std::string& getEmployeeNr() const { return employeeNr_; }
    const std::string& getPosition() const { return position_; }
    void setIdentityDirect(const std::string& emp, const std::string& pos) { employeeNr_ = emp; position_ = pos; }

    void addLog(const std::string& line) { log_.add(line); }

    // 0..1 progress of current activity (battle damage dealt / start delay / rest between battles)
    float getActivityProgress() const;
    // 0..1 progress to next series (funding round). uses cached required funding (no rng jitter)
    float getFundingFraction() const;
    bool isSelling() const { return selloff_.active; }
    // call after SaveGame::load: resync cached funding/series trackers
    void onStateRestored();
    // only real win = clock reaching zero (20y). series can pass 20.
    bool isGameWon() const { return currentElapsedSeconds_ >= 20.0 * 365 * 86400; }

    // quests
    QuestSystem* getQuestSystem() { return questSystem_.get(); }
    const QuestSystem* getQuestSystem() const { return questSystem_.get(); }
    bool consumeFinalWarning() { return questSystem_->consumeFinalWarning(); }
    void acknowledgeFinal() { questSystem_->acknowledgeFinal(); }

    // accessors for UI (read-only)
    Character* getCharacter() { return character_.get(); }
    const Character* getCharacter() const { return character_.get(); }
    BattleEncounter* getCurrentEncounter() { return currentEncounter_.get(); }
    bool isInBattle() const { return inBattle_; }
    bool isWaitingForBattleStart() const { return waitingForBattleStart_; }
    int getRoundNumber() const { return roundNumber_; }
    const BattleLog& getLog() const { return log_; }
    // everything: summary lines + captured stdout details (patent effects, level-ups, sales...)
    const BattleLog& getDetailLog() const { return detailLog_; }
    SpecialDropsLibrary& getSpecialDropsLibrary() { return *specialDropsLibrary_; }
    const SpecialDropsLibrary& getSpecialDropsLibrary() const { return *specialDropsLibrary_; }

    struct Counters {
        int totalBattles = 0, totalVictories = 0, totalDefeats = 0;
        int bossesDefeated = 0, bankruptcies = 0, townVisits = 0;
    };
    const Counters& getCounters() const { return counters_; }

    // shadow player: ShadowPlayerSystem doesn't track defeated itself, engine does
    bool isShadowPlayerDefeated() const { return shadowPlayerDefeated_; }
    void setShadowPlayerDefeated(bool v) { shadowPlayerDefeated_ = v; }

    // for SaveGame restore (needs class-internal access to sub-systems)
    NemesisSystem* getNemesisSystem() { return nemesisSystem_.get(); }
    const NemesisSystem* getNemesisSystem() const { return nemesisSystem_.get(); }
    ReviewCalendarSystem* getReviewCalendarSystem() { return reviewCalendarSystem_.get(); }
    const ReviewCalendarSystem* getReviewCalendarSystem() const { return reviewCalendarSystem_.get(); }
    Counters& mutableCounters() { return counters_; }
    // rebuild review calendar against fresh (elapsed=0) clock, keep series-based content,
    // then caller reapplies saved ReviewFlag list via getReviewCalendarSystem()->restoreFlags(...)
    void rebuildReviewCalendarForLoad() {
        reviewCalendarSystem_ = std::make_unique<ReviewCalendarSystem>();
        reviewCalendarSystem_->generateReviewCalendar(character_->getSeries());
    }

private:
    // --- state machine (mirrors old MainWindow) ---
    bool initialized_ = false;
    int initGeneration_ = 0;
    std::string employeeNr_, position_, configRoot_ = "config/";
    // funding awarded per $ of monster reward. was moneyReward/10 in Qt build. tune for pacing.
    static constexpr double FUNDING_PER_MONEY = 0.1;
    bool debugInstakill_ = false;

    std::unique_ptr<Character> character_;
    std::unique_ptr<MonsterGenerator> monsterGen_;
    std::unique_ptr<EquipmentGenerator> equipGen_;
    std::unique_ptr<ItemGenerator> itemGen_;
    std::unique_ptr<SpecializedMonsterGenerator> specialMonsterGen_;
    std::unique_ptr<NemesisSystem> nemesisSystem_;
    std::unique_ptr<ReviewCalendarSystem> reviewCalendarSystem_;
    std::unique_ptr<ShadowPlayerSystem> shadowPlayerSystem_;
    std::unique_ptr<NamedMonsterLibrary> namedMonsterLibrary_;
    std::unique_ptr<SpecialDropsLibrary> specialDropsLibrary_;
    std::unique_ptr<BattleEncounter> currentEncounter_;

    BattleLog log_;
    BattleLog detailLog_{2000};
    Counters counters_;
    std::mt19937 rng_;

    bool inBattle_ = false;
    bool playerTurn_ = true;
    bool needsRecovery_ = false;
    bool waitingForNextBattle_ = false;
    bool waitingForBattleStart_ = false;
    double battleStartTime_ = 0.0;
    int roundNumber_ = 0;
    int ticksSinceLastAction_ = 0;
    int ticksSinceBattleEnd_ = 0;
    static constexpr int TICKS_PER_ACTION = 10;      // ~1s at 100ms/tick, tune as needed
    static constexpr int TICKS_BETWEEN_BATTLES = 20;

    int lastSeries_ = 1;
    double cachedRequiredFunding_ = 0.0;
    double lastNemesisTime_ = -3600.0 * 24 * 365;
    int lastEquipmentPurchaseSeries_ = 0;
    std::set<EquipmentType> purchasedThisCycle_;
    bool shadowPlayerDefeated_ = false;

    double currentElapsedSeconds_ = 0.0;

    // --- animated selloff: items leave one at a time, last-in-list first ---
    struct SelloffState {
        bool active = false;
        bool emergency = false;        // true: mid-battle sale, recovers runway from proceeds
        bool includeEquipment = false; // town visit also sells unequipped gear
        int ticks = 0;
        int earned = 0;
        int total = 0;                 // items+gear at start, for progress
        std::vector<std::string> itemNames; // snapshot in display order, sold from the back
    } selloff_;
    static constexpr int SELL_STEP_TICKS = 3; // 100ms ticks between items
    void beginSelloff(bool emergency, bool includeEquipment);
    void stepSelloff();
    void finishSelloff();

    std::unique_ptr<QuestSystem> questSystem_ = std::make_unique<QuestSystem>();
    void syncBias();
    void grantQuestReward(bool isMain, const std::string& title);

    // --- extracted from MainWindow, Qt-free ---
    void checkSeriesUpgrade();
    void checkHypeEnergyRecovery();
    void checkSpecialEncounters();
    void checkNemesisEncounter();
    void checkMonthlyReview();
    void checkShadowPlayer();
    void checkInventoryFull();
    void checkRecoveryNeeded();
    void checkDefensivePatents();

    void handleBattle();
    void handleVictory();
    void handleDefeat();
    bool playerAttack();
    void monsterAttack(Monster* enemy);
    void tryEmergencyHealing();
    void purchaseNewEquipment();
    void visitTown();
    void visitTownForRecovery();
    void handleSpecialDrop(const Monster& enemy);
    void addSpecialDropToLibrary(SpecialDropType type, const Equipment& equipment,
                                  const std::string& sourceName, int year, int month);
};

#endif