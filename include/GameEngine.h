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

// headless. no Qt/ftxui deps. UI polls state each render frame.
// clock: caller feeds elapsedSeconds (from ftxui CountdownComponent), engine
// stores no wall-clock of its own except lastNemesisTime (elapsed-seconds space).
class GameEngine {
public:
    explicit GameEngine(bool debugInstakill = false);

    // setup
    bool loadConfig(const std::string& configRoot = "config/");
    void initializeCharacter(const std::string& name, const std::string& employeeNr,
                              const std::string& position, const std::string& startup);

    // main update, call every render frame with current run elapsed seconds
    void tick(double elapsedSeconds);

    bool isInitialized() const { return initialized_; }
    bool isGameWon() const { return character_ && character_->getSeries() >= 20; }

    // accessors for UI (read-only)
    Character* getCharacter() { return character_.get(); }
    const Character* getCharacter() const { return character_.get(); }
    BattleEncounter* getCurrentEncounter() { return currentEncounter_.get(); }
    bool isInBattle() const { return inBattle_; }
    bool isWaitingForBattleStart() const { return waitingForBattleStart_; }
    int getRoundNumber() const { return roundNumber_; }
    const BattleLog& getLog() const { return log_; }
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