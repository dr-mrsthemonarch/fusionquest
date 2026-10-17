#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QListWidget>
#include <chrono>
#include <memory>
#include <random>
#include <QPropertyAnimation>
#include <set>
// Include your actual game headers
#include "Character.h"
#include "../include/Monster.h"
#include "MonsterGenerator.h"
#include "EquipmentGenerator.h"
#include "ItemGenerator.h"
#include "Patent.h"
#include "NemesisSystem.h"
#include "ExpandedMonsterTypes.h"
#include "../include/SpecialDropsLibrary.h"
#include "NamedMonsterLibrary.h"
#include "BattleEncounter.h"

struct EnemyFrame {
    QWidget* container;
    QLabel* symbolLabel;
    QLabel* nameLabel;
    QLabel* healthLabel;
    QProgressBar* healthBar;
    QLabel* attackLabel;
    QLabel* defenseLabel;
    QPropertyAnimation* healthAnimation;
    int targetHealth = -1;
    bool isDead = false;  // NEW
    QTimer* fadeTimer = nullptr;  // NEW
};



class MainWindow : public QMainWindow {
    Q_OBJECT public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:
    void gameLoop();

    void checkShadowPlayer();

    void startGame();

    void tryEmergencyHealing();

    void pauseGame();

    void updateDisplay();


private:
    void setupUI();

    void initializeGame();

    void handleSpecialDrop(const Monster &enemy);

    void addSpecialDropToLibrary(SpecialDropType type, const Equipment &equipment, const std::string &sourceName, int year, int month);

    void animateProgressBar(QProgressBar *bar, int newValue, QPropertyAnimation *animation);

    void animateProgressBarWithDeath(QProgressBar *bar, int newValue, QPropertyAnimation *animation);

    void updateSpecialDropsDisplay();

    QString getEnemySymbol(Monster *enemy) const;

    QString getEnemyColor(Monster *enemy) const;

    void updateEnemyFrame(size_t frameIndex, Monster *enemy);

    void clearEnemyFrame(size_t frameIndex);

    void updateBattleDisplay();

    void checkMonthlyReview();

    void adventureLoop();

    void refreshSpecialDropsList();

    void handleBattle();

    void checkNemesisEncounter();

    void checkSpecialEncounters();

    void checkInventoryFull();

    void checkDefensivePatents();

    void checkRecoveryNeeded();

    void displayActiveEffects();

    void checkHypeEnergyRecovery();

    void checkSeriesUpgrade();

    void purchaseNewEquipment();

    void updateCharacterStats();

    void updateEquipmentDisplay();

    void updateInventoryDisplay();

    void updatePatentsDisplay();

    void updateBattleLog(const QString &message);

    void updateCurrentEnemy();

    bool playerAttack();

    void monsterAttack(Monster *enemy);

    void handleVictory();

    void handleDefeat();

    void visitTown();

    void visitTownForRecovery();

    void setupGameplayTab();

    void setupSpecialDropsTab();

    void setupEquipmentTab();

    void setupEnemyFrames();

    // UI Components
    QTimer *gameTimer;

    // Character Stats Group
    QGroupBox *characterStatsGroup;
    QLabel *nameLabel;
    QLabel *seriesLabel;
    QLabel *runwayLabel;
    QLabel *hypeEnergyLabel;
    QLabel *hustleLabel;
    QLabel *agilityLabel;
    QLabel *ndasLabel;
    QLabel *luckLabel;
    QLabel *synergyLabel;
    QLabel *technicalDebtLabel;
    QLabel *busFactorLabel;
    QLabel *moneyLabel;
    QLabel *fundingLabel;
    QProgressBar *fundingProgress;
    QProgressBar *runwayBar;
    QProgressBar *hypeBar;

    // Equipment Group
    QGroupBox *equipmentGroup;
    QLabel *laptopSlot;
    QLabel *phoneSlot;
    QLabel *backpackSlot;
    QLabel *helmetSlot;
    QLabel *bikeSlot;
    QLabel *jacketSlot;
    QLabel *glovesSlot;
    QLabel *headphonesSlot;

    // Inventory Group
    QGroupBox *inventoryGroup;
    QListWidget *inventoryList;
    QLabel *inventoryCountLabel;

    // Patents Group
    QGroupBox *patentsGroup;
    QListWidget *patentsList;

    // Battle Group
    QGroupBox *battleGroup;
    QLabel *enemyNameLabel;
    QLabel *enemyTypeLabel;
    QLabel *enemyLevelLabel;
    QLabel *enemyHealthLabel;
    QLabel *enemyAttackLabel;
    QLabel *enemyDefenseLabel;
    QProgressBar *enemyHealthBar;
    QPropertyAnimation *enemyHealthAnimation;
    QPropertyAnimation *runwayAnimation;
    QPropertyAnimation *hypeAnimation;
    QPropertyAnimation *fundingAnimation;
    QLabel *roundNumberLabel;


    // Battle Log
    QTextEdit *battleLog;

    // Statistics Group
    QGroupBox *statsGroup;
    QLabel *totalBattlesLabel;
    QLabel *victoriesLabel;
    QLabel *defeatsLabel;
    QLabel *bossesDefeatedLabel;
    QLabel *bankruptciesLabel;
    QLabel *townVisitsLabel;
    QLabel *gameTimeLabel;

    // Control buttons
    QPushButton *startButton;
    QPushButton *pauseButton;
    QCheckBox *debugDefeatCheckbox;

    // Game state
    std::unique_ptr<Character> player;
    std::unique_ptr<MonsterGenerator> monsterGen;
    std::unique_ptr<EquipmentGenerator> equipGen;
    std::unique_ptr<ItemGenerator> itemGen;
    QLabel* waitingForBattleLabel;
    std::unique_ptr<NemesisSystem> nemesisSystem;
    std::unique_ptr<SpecializedMonsterGenerator> specialMonsterGen;
    std::unique_ptr<ReviewCalendarSystem> reviewCalendarSystem;
    std::unique_ptr<ShadowPlayerSystem> shadowPlayerSystem;
    std::unique_ptr<SpecialDropsLibrary> specialDropsLibrary;
    std::unique_ptr<Monster> currentEnemy;
    std::unique_ptr<BattleEncounter> currentEncounter;
    std::unique_ptr<NamedMonsterLibrary> namedMonsterLibrary;

    std::chrono::steady_clock::time_point gameStartTime;
    std::mt19937 rng;

    // Game statistics
    int totalBattles;
    int totalVictories;
    int totalDefeats;
    int bossesDefeated;
    int bankruptcies;
    int townVisits;
    int roundNumber;
    int lastSeries;
    double cachedRequiredFunding;
    bool inBattle;
    bool playerTurn;
    bool gamePaused;
    bool gameInitialized;
    bool needsRecovery;
    double lastNemesisTime;
    bool shadowPlayerDefeated;
    std::set<EquipmentType> purchasedThisCycle;
    int lastEquipmentPurchaseSeries;
    QTabWidget *mainTabs;
    QWidget *gameplayTab;
    QWidget *specialDropsTab;
    QListWidget *specialDropsList;
    QTextEdit *specialDropDetails;
    QLabel *specialDropsCountLabel;
    // Equipment tab
    QWidget* equipmentTab;
    QWidget* equipmentCircle;
    std::vector<QLabel*> equipmentSlots;
    std::vector<QLabel*> equipmentStats;

    // Battle timing
    double battleStartTime;
    bool waitingForBattleStart;
    int ticksSinceLastAction;
    const int TICKS_PER_ACTION = 10;
    static constexpr int TICKS_BETWEEN_BATTLES = 25; // 2 second delay
    int ticksSinceBattleEnd = 0;
    bool waitingForNextBattle = false;
    int runwayTarget;
    int hypeTarget;
    int fundingTarget;
    int enemyHealthTarget;
    std::vector<EnemyFrame> enemyFrames_;
    QGridLayout* enemyGridLayout_;
};

#endif // MAINWINDOW_H