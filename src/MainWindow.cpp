#include "../include/MainWindow.h"
#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QGroupBox>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QListWidget>
#include <QScrollBar>
#include <QSplitter>
#include <QDateTime>
#include <algorithm>
#include "../include/SpecialDropsLibrary.h"
#include "../include/ExpandedMonsterTypes.h"
#include "../include/PatentStrategy.h"
#include <QTCore>
#include "../include/NamedMonsterLibrary.h"
#include "../include/NamedMonster.h"
#include "../include/BattleEncounter.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), gameTimer(new QTimer(this)), totalBattles(0), totalVictories(0), totalDefeats(0), bossesDefeated(0), bankruptcies(0), townVisits(0), roundNumber(0), lastSeries(1), cachedRequiredFunding(0.0), inBattle(false), playerTurn(true), gamePaused(true), gameInitialized(false), needsRecovery(false), lastNemesisTime(-3600.0 * 24 * 365), lastEquipmentPurchaseSeries(0), rng(std::random_device{}()), ticksSinceLastAction(0), runwayTarget(-1), hypeTarget(-1), fundingTarget(-1), enemyHealthTarget(-1),waitingForBattleStart(false),battleStartTime(0.0) {
    setupUI();

    connect(gameTimer, &QTimer::timeout, this, &MainWindow::gameLoop);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startGame);
    connect(pauseButton, &QPushButton::clicked, this, &MainWindow::pauseGame);
    // REMOVED: debug button connection

    setWindowTitle("Startup RPG - ProgressQuest Style");
    resize(1200, 700);
}

MainWindow::~MainWindow() {
}

void MainWindow::adventureLoop() {
    if (!player || !gameInitialized) return;

    // Check victory condition
    if (player->getSeries() >= 20) {
        pauseGame();
        updateBattleLog("🏆 VICTORY! Reached Series 20!");
        updateBattleLog("🎉 ADVENTURE COMPLETE!");

        // Show final statistics
        updateBattleLog("\n═══ FINAL STATISTICS ═══");
        updateBattleLog(QString("Total Battles: %1").arg(totalBattles));
        updateBattleLog(QString("Victories: %1").arg(totalVictories));
        updateBattleLog(QString("Bosses Defeated: %1").arg(bossesDefeated));
        updateBattleLog(QString("Special Items Collected: %1").arg(specialDropsLibrary->getTotalDrops()));
        updateBattleLog(QString("Patents Mastered: %1").arg(Patent::characterPatents.size()));
        return;
    }

    auto now = std::chrono::steady_clock::now();
    double elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - gameStartTime).count();
    player->setElapsedSeconds(elapsedSeconds);

    // Check for series upgrade
    checkSeriesUpgrade();

    // Check for patent level-ups (on prime series numbers)
    Patent::levelUpCharacterPatents(player.get());

    // Check for hype energy recovery
    checkHypeEnergyRecovery();

    // Check for recovery needed
    if (needsRecovery && !inBattle) {
        checkRecoveryNeeded();
        return;
    }

    if (!inBattle && namedMonsterLibrary) {
        auto* namedDef = namedMonsterLibrary->checkForEncounter(elapsedSeconds,
                                                                player->getSeries());
        if (namedDef) {
            std::vector<Monster> namedEncounter =
                namedMonsterLibrary->getMonsterSystem().generateNamedEncounter(
                    *namedDef, player->getSeries()
                );

            currentEncounter = std::make_unique<BattleEncounter>();
            for (auto& enemy : namedEncounter) {
                currentEncounter->addEnemy(std::make_unique<Monster>(std::move(enemy)));
            }

            inBattle = true;
            roundNumber = 1;
            if (currentEncounter && currentEncounter->getTotalCount() > 0) {
                playerTurn = player->getLuck() >= currentEncounter->getEnemy(0)->speed;
            } else {
                playerTurn = true;
            }
            totalBattles++;
            ticksSinceLastAction = 0;

            updateBattleLog(QString("📜 NAMED MONSTER: %1")
                .arg(QString::fromStdString(currentEncounter->getEnemy(0)->name)));

            if (namedDef->minionCount > 0) {
                updateBattleLog(QString("⚠️ With %1 minions!").arg(namedDef->minionCount));
            }

            updateBattleLog("⚠️ Must be defeated to continue!");
            updateBattleDisplay();
            return;
        }
    }

    // Check for special encounters when not in battle
    if (!inBattle) {
        // Check for monthly review (highest priority)
        checkMonthlyReview();
        if (inBattle) return;

        // Check for nemesis encounter
        checkNemesisEncounter();
        if (inBattle) return;

        // Check for shadow player (ultra rare)
        checkShadowPlayer();
        if (inBattle) return;

        // Check if inventory is full
        if (player->isInventoryFull()) {
            checkInventoryFull();
            return;
        }
    }

    // Handle current battle or start new one
    if (!inBattle) {
        // Handle battle delay between encounters
        if (waitingForNextBattle) {
            ticksSinceBattleEnd++;
            if (ticksSinceBattleEnd >= TICKS_BETWEEN_BATTLES) {
                waitingForNextBattle = false;
                ticksSinceBattleEnd = 0;
                updateBattleLog("⏳ Ready for next encounter...");
            } else {
                return; // Still waiting
            }
        }

        // Try special encounters (23% total chance)
        checkSpecialEncounters();

        if (!inBattle) {
            // Generate multi-enemy encounter
            int mobCount = 0;
            int bossCount = 0;
            auto enemies = monsterGen->generateGroupEncounter(
                player->getSeries(), mobCount, bossCount, *itemGen
            );

            currentEncounter = std::make_unique<BattleEncounter>();
            for (auto& enemy : enemies) {
                currentEncounter->addEnemy(std::move(enemy));
            }

            inBattle = true;
            roundNumber = 1;
            playerTurn = player->getLuck() >= currentEncounter->getEnemy(0)->speed;
            totalBattles++;
            ticksSinceLastAction = 0;
        waitingForBattleStart = true;
        battleStartTime = 0;
            QString encounterType;
            if (bossCount > 0) {
                encounterType = QString("👑 BOSS + %1 MINIONS").arg(mobCount);
            } else if (mobCount > 1) {
                encounterType = QString("💾 MOB GROUP (x%1)").arg(mobCount);
            } else {
                encounterType = "💾 MOB";
            }

            updateBattleLog(QString("⚔️ Encounter #%1: %2").arg(totalBattles).arg(encounterType));

            // List all enemies
            auto allEnemies = currentEncounter->getAllEnemies();
            for (size_t i = 0; i < allEnemies.size(); ++i) {
                updateBattleLog(QString("  %1. %2 (Lvl %3, HP: %4)")
                    .arg(i + 1)
                    .arg(QString::fromStdString(allEnemies[i]->name))
                    .arg(allEnemies[i]->level)
                    .arg(allEnemies[i]->maxHealth));
            }

            updateBattleDisplay();
        }
    } else {
        // NEW: Handle battle start delay
        if (waitingForBattleStart) {
            battleStartTime += 0.1; // Each tick is 100ms
            if (battleStartTime >= 2.0) {
                waitingForBattleStart = false;
                updateBattleLog("⚔️ Battle begins!");
            }
            return;
        }

        // Continue battle with timing
        ticksSinceLastAction++;
        if (ticksSinceLastAction >= TICKS_PER_ACTION) {
            handleBattle();
            ticksSinceLastAction = 0;
        }
    }

    // Progress update every 10 battles
    if (totalBattles > 0 && totalBattles % 10 == 0 && !inBattle) {
        static int lastReportedBattle = 0;
        if (lastReportedBattle != totalBattles) {
            updateBattleLog(QString("📈 Progress: Series %1/20 | Battles: %2 | Wins: %3 | Bosses: %4")
                .arg(player->getSeries()).arg(totalBattles).arg(totalVictories).arg(bossesDefeated));
            lastReportedBattle = totalBattles;
        }
    }
}

void MainWindow::animateProgressBar(QProgressBar *bar, int newValue, QPropertyAnimation *animation) {
    if (!bar || !animation) return;

    // Clamp value to valid range
    newValue = std::clamp(newValue, bar->minimum(), bar->maximum());

    // Determine which target we're tracking
    int *targetPtr = nullptr;
    if (animation == runwayAnimation) targetPtr = &runwayTarget;
    else if (animation == hypeAnimation) targetPtr = &hypeTarget;
    else if (animation == fundingAnimation) targetPtr = &fundingTarget;
    else if (animation == enemyHealthAnimation) targetPtr = &enemyHealthTarget;

    // Skip if already animating to this target
    if (targetPtr && *targetPtr == newValue) return;

    // Update target
    if (targetPtr) *targetPtr = newValue;

    // Stop any running animation
    if (animation->state() == QAbstractAnimation::Running) {
        animation->stop();
    }

    animation->setTargetObject(bar);
    animation->setPropertyName("value");
    animation->setStartValue(bar->value());
    animation->setEndValue(newValue);
    animation->setDuration(500); // Unified 500ms
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->start();
}

void MainWindow::addSpecialDropToLibrary(SpecialDropType type, const Equipment &equipment, const std::string &sourceName, int year, int month) {
    if (!specialDropsLibrary) return;

    SpecialDrop drop;
    drop.type = type;
    drop.equipment = equipment;
    drop.name = equipment.name;
    drop.acquiredFrom = sourceName;
    drop.yearAcquired = year;
    drop.monthAcquired = month;
    drop.isEquipped = false;

    // Add lore text based on type
    switch (type) {
        case SpecialDropType::NEMESIS_DROP:
            drop.loreText = "Forged from the essence of a defeated nemesis. Each evolution makes it stronger.";
            break;
        case SpecialDropType::SHADOW_PLAYER_DROP:
            drop.loreText = "A reflection of yourself from another dimension. Impossibly rare.";
            break;
        case SpecialDropType::SAFETY_MANAGER_DROP:
            drop.loreText = "Blessed by the Safety Manager. Reduces risk and protects against technical debt.";
            break;
        case SpecialDropType::CONFERENCE_DROP:
            drop.loreText = "Earned through networking and knowledge sharing at prestigious conferences.";
            break;
        case SpecialDropType::PAPER_WRITING_DROP:
            drop.loreText = "The culmination of rigorous research and peer review. Academic excellence embodied.";
            break;
        case SpecialDropType::MONTHLY_REVIEW_DROP:
            drop.loreText = "A trophy commemorating exceptional monthly performance. Scales with time.";
            break;
    }

    specialDropsLibrary->addSpecialDrop(drop);
}

void MainWindow::animateProgressBarWithDeath(QProgressBar *bar, int newValue, QPropertyAnimation *animation) {
    if (!bar || !animation) return;

    // Stop any running animation
    if (animation->state() == QAbstractAnimation::Running) {
        animation->stop();
    }

    // First animate to 0
    animation->setTargetObject(bar);
    animation->setPropertyName("value");
    animation->setStartValue(bar->value());
    animation->setEndValue(0);
    animation->setDuration(500);
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    // Use a scoped connection that will auto-disconnect
    QMetaObject::Connection *connection = new QMetaObject::Connection();
    *connection = connect(animation, &QPropertyAnimation::finished, this, [=]() {
        // Clean up the connection
        disconnect(*connection);
        delete connection;

        QTimer::singleShot(200, this, [=]() {
            if (bar && animation) {
                animateProgressBar(bar, newValue, animation);
            }
        });
    });

    animation->start();
}

void MainWindow::checkMonthlyReview() {
    if (!reviewCalendarSystem) return;

    auto now = std::chrono::steady_clock::now();
    double elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
        now - gameStartTime).count();

    ScheduledBoss* scheduledReview = reviewCalendarSystem->checkForScheduledReview(elapsedSeconds);

    if (scheduledReview && !inBattle) {
        updateBattleLog("📅 MONTHLY PERFORMANCE REVIEW!");
        updateBattleLog(QString("📊 %1")
            .arg(QString::fromStdString(scheduledReview->bossData.name)));

        currentEncounter = std::make_unique<BattleEncounter>();
        Monster reviewBoss;
        const BossData& data = scheduledReview->bossData;

        reviewBoss.name = data.name;
        reviewBoss.level = data.level;
        reviewBoss.maxHealth = data.maxHealth;
        reviewBoss.health = data.maxHealth;
        reviewBoss.attack = data.attack;
        reviewBoss.defense = data.defense;
        reviewBoss.speed = data.speed;
        reviewBoss.moneyReward = data.moneyReward;
        reviewBoss.itemReward = data.itemReward;
        reviewBoss.type = data.type;
        reviewBoss.difficulty = data.difficulty;
        reviewBoss.maxHypeEnergy = data.maxHypeEnergy;
        reviewBoss.hypeEnergy = data.maxHypeEnergy;

        switch (data.difficulty) {
            case MonsterDifficulty::EASY:
            case MonsterDifficulty::MEDIUM:
                reviewBoss.setStrategy(std::make_unique<BalancedStrategy>());
                break;
            case MonsterDifficulty::HARD:
            case MonsterDifficulty::LEGENDARY:
                reviewBoss.setStrategy(std::make_unique<TacticalStrategy>());
                break;
        }

        currentEncounter->addEnemy(std::make_unique<Monster>(std::move(reviewBoss)));
        scheduledReview->encountered = true;

        inBattle = true;
        roundNumber = 1;
        playerTurn = player->getLuck() >= currentEncounter->getEnemy(0)->speed;
        totalBattles++;
        ticksSinceLastAction = 0;

        updateBattleDisplay();
    }
}

void MainWindow::checkShadowPlayer() {
    if (shadowPlayerDefeated || !shadowPlayerSystem) return;

    auto now = std::chrono::steady_clock::now();
    double elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
        now - gameStartTime).count();

    if (shadowPlayerSystem->shouldShadowPlayerAppear(elapsedSeconds, shadowPlayerDefeated)) {
        updateBattleLog("SHADOW LINK APPEARS");

        currentEncounter = std::make_unique<BattleEncounter>();
        currentEncounter->addEnemy(std::make_unique<Monster>(
            shadowPlayerSystem->generateShadowPlayer(*player, elapsedSeconds)
        ));

        updateBattleLog(QString("👤 %1")
            .arg(QString::fromStdString(currentEncounter->getEnemy(0)->name)));
        updateBattleLog("Level: " + QString::number(currentEncounter->getEnemy(0)->level) +
                       " | HP: " + QString::number(currentEncounter->getEnemy(0)->health));

        inBattle = true;
        roundNumber = 1;
        playerTurn = player->getLuck() >= currentEncounter->getEnemy(0)->speed;
        totalBattles++;
        ticksSinceLastAction = 0;

        updateBattleDisplay();
    }
}

void MainWindow::checkSeriesUpgrade() {
    int currentSeries = player->getSeries();
    if (currentSeries > lastSeries) {
        cachedRequiredFunding = player->getRequiredFunding();
        lastSeries = currentSeries;

        updateBattleLog(QString("📈 SERIES UP! Now Series %1").arg(currentSeries));
        purchaseNewEquipment();
    }
}

void MainWindow::checkHypeEnergyRecovery() {
    if (!player) return;

    int maxHype = player->getMaxHypeEnergy();
    int currentHype = player->getHypeEnergy();

    // If below 10% of max hype energy
    if (currentHype < (maxHype * 0.1)) {
        // Cost to recover 50% of max hype: 20% of current money
        int recoveryCost = static_cast<int>(player->getMoney() * 0.2);
        int hypeToRecover = maxHype / 2;

        if (player->getMoney() >= recoveryCost && recoveryCost > 0) {
            player->spendMoney(recoveryCost);
            player->setHypeEnergy(std::min(maxHype, currentHype + hypeToRecover));

            updateBattleLog(QString("⚡ Hype Energy Recovery: Spent $%1 to recover %2 energy").arg(recoveryCost).arg(hypeToRecover));
        }
    }
}

void MainWindow::checkSpecialEncounters() {
    int chance = 1000;
    auto now = std::chrono::steady_clock::now();
    double elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
        now - gameStartTime).count();

    std::uniform_int_distribution<> safetyChance(1, chance);
    if (safetyChance(rng) <= 1) {
        // Safety Manager with 2 cronies
        std::vector<Monster> encounter = specialMonsterGen->generateSafetyManagerEncounter(
            player->getSeries(), elapsedSeconds
        );

        currentEncounter = std::make_unique<BattleEncounter>();
        for (auto& enemy : encounter) {
            currentEncounter->addEnemy(std::make_unique<Monster>(std::move(enemy)));
        }

        updateBattleLog("🛡️ SAFETY MANAGER ENCOUNTER!");
        updateBattleLog("⚠️ Safety Manager + 2 Safety Cronies appear!");

        inBattle = true;
        roundNumber = 1;
        playerTurn = player->getAgility() >= currentEncounter->getEnemy(0)->speed;
        totalBattles++;
        ticksSinceLastAction = 0;

        updateBattleDisplay();
        return;
    }

    std::uniform_int_distribution<> confChance(1, chance);
    if (confChance(rng) <= 1) {
        currentEncounter = std::make_unique<BattleEncounter>();
        currentEncounter->addEnemy(std::make_unique<Monster>(
            specialMonsterGen->generateConference(player->getSeries(), elapsedSeconds)
        ));

        updateBattleLog("🎤 CONFERENCE ENCOUNTER!");
        updateBattleLog(QString("📊 %1 begins!")
            .arg(QString::fromStdString(currentEncounter->getEnemy(0)->name)));

        inBattle = true;
        roundNumber = 1;
        playerTurn = player->getAgility() >= currentEncounter->getEnemy(0)->speed;
        totalBattles++;
        ticksSinceLastAction = 0;

        updateBattleDisplay();
        return;
    }

    std::uniform_int_distribution<> paperChance(1, chance);
    if (paperChance(rng) <= 1) {
        currentEncounter = std::make_unique<BattleEncounter>();
        currentEncounter->addEnemy(std::make_unique<Monster>(
            specialMonsterGen->generatePaperWriting(player->getSeries(), elapsedSeconds)
        ));

        updateBattleLog("📄 PAPER DEADLINE ENCOUNTER!");
        updateBattleLog(QString("✏️ %1 looms!")
            .arg(QString::fromStdString(currentEncounter->getEnemy(0)->name)));

        inBattle = true;
        roundNumber = 1;
        playerTurn = player->getAgility() >= currentEncounter->getEnemy(0)->speed;
        totalBattles++;
        ticksSinceLastAction = 0;

        updateBattleDisplay();
    }
}

void MainWindow::checkNemesisEncounter() {
    auto now = std::chrono::steady_clock::now();
    double elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
        now - gameStartTime).count();

    const double SECONDS_PER_YEAR = 3600.0 * 24 * 365;

    if (nemesisSystem->shouldNemesisAppear(elapsedSeconds, lastNemesisTime)) {
        updateBattleLog("🔥🔥🔥 NEMESIS ENCOUNTER! 🔥🔥🔥");
        updateBattleLog("A legendary foe blocks your path!");

        if (nemesisSystem->getNemesisPowerLevel() > 0) {
            updateBattleLog(QString("The Nemesis returns, more powerful! Power Level: +%1")
                .arg(nemesisSystem->getNemesisPowerLevel()));
        }

        currentEncounter = std::make_unique<BattleEncounter>();
        currentEncounter->addEnemy(std::make_unique<Monster>(
            nemesisSystem->generateNemesis(player->getSeries(), elapsedSeconds,
                                          player->getName())
        ));

        updateBattleLog(QString("⚡ %1 appears!")
            .arg(QString::fromStdString(currentEncounter->getEnemy(0)->name)));
        updateBattleLog(QString("Level: %1 | HP: %2")
            .arg(currentEncounter->getEnemy(0)->level)
            .arg(currentEncounter->getEnemy(0)->health));
        updateBattleLog("This is an overwhelming force...");

        inBattle = true;
        roundNumber = 1;
        playerTurn = player->getLuck() >= currentEncounter->getEnemy(0)->speed;
        totalBattles++;
        ticksSinceLastAction = 0;

        lastNemesisTime = elapsedSeconds;

        updateBattleDisplay();
    }
}

void MainWindow::checkInventoryFull() {
    if (player->isInventoryFull()) {
        updateBattleLog("🎒 Inventory full! Heading to town...");
        visitTown();
    }
}

void MainWindow::checkDefensivePatents() {
    // Small chance to auto-trigger defensive patents when hit
    std::uniform_int_distribution<> triggerChance(0, 99);
    if (triggerChance(rng) < 15) {
        // 15% chance
        auto defensePatents = Patent::getPatentsByType(PatentType::DEFENSE);
        for (const auto &patentName: defensePatents) {
            auto it = Patent::characterPatents.find(patentName);
            if (it != Patent::characterPatents.end()) {
                const Patent &patent = it->second;
                if (player->getHypeEnergy() >= patent.getCost()) {
                    Patent::usePatent(patentName, player.get(), player.get());
                    updateBattleLog(QString("🛡️ %1 activated defensively!").arg(QString::fromStdString(patent.getFullName())));
                    break;
                }
            }
        }
    }
}

void MainWindow::checkRecoveryNeeded() {
    if (player->getRunway() == 1 && needsRecovery) {
        visitTownForRecovery();
        needsRecovery = false;
    }
}

void MainWindow::displayActiveEffects() {
    // Display player effects
    if (player->getEffectsManager().getEffectCount() > 0) {
        updateBattleLog("🌟 Player Effects:");
        std::string summary = player->getEffectsManager().getEffectsSummary();
        QStringList lines = QString::fromStdString(summary).split('\n');
        for (const QString &line: lines) {
            if (!line.isEmpty()) {
                updateBattleLog("  " + line);
            }
        }
    }

    // Display enemy effects
    if (currentEnemy && currentEnemy->getEffectsManager().getEffectCount() > 0) {
        updateBattleLog("🌟 Enemy Effects:");
        std::string summary = currentEnemy->getEffectsManager().getEffectsSummary();
        QStringList lines = QString::fromStdString(summary).split('\n');
        for (const QString &line: lines) {
            if (!line.isEmpty()) {
                updateBattleLog("  " + line);
            }
        }
    }
}

void MainWindow::gameLoop() {
    if (gamePaused || !gameInitialized) return;

    // Update game time display
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - gameStartTime).count();
    int hours = elapsed / 3600;
    int minutes = (elapsed % 3600) / 60;
    int seconds = elapsed % 60;
    gameTimeLabel->setText(QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));

    adventureLoop();
    updateDisplay();
}

void MainWindow::handleSpecialDrop(const Monster &enemy) {
    auto now = std::chrono::steady_clock::now();
    double elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - gameStartTime).count();

    const double SECONDS_PER_YEAR = 365.25 * 24 * 60 * 60;
    const double SECONDS_PER_MONTH = SECONDS_PER_YEAR / 12.0;

    int currentYear = static_cast<int>(elapsedSeconds / SECONDS_PER_YEAR) + 1;
    int currentMonth = static_cast<int>((elapsedSeconds - ((currentYear - 1) * SECONDS_PER_YEAR)) / SECONDS_PER_MONTH) + 1;
    currentMonth = std::clamp(currentMonth, 1, 12);

    Equipment specialEquipment;
    SpecialDropType dropType;

    // Check enemy type and generate appropriate special equipment
    if (enemy.type == MonsterType::NEMESIS) {
        dropType = SpecialDropType::NEMESIS_DROP;
        int powerLevel = nemesisSystem->getNemesisPowerLevel() - 1; // -1 because we already incremented
        specialEquipment = specialDropsLibrary->generateNemesisEquipment(player->getSeries(), powerLevel, enemy.name);

        updateBattleLog("⚡ LEGENDARY DROP: " + QString::fromStdString(specialEquipment.name));
    } else if (enemy.name.find("Shadow:") != std::string::npos) {
        dropType = SpecialDropType::SHADOW_PLAYER_DROP;
        specialEquipment = specialDropsLibrary->generateShadowPlayerEquipment(player->getSeries(), player->getName());

        updateBattleLog("👤 ULTRA RARE DROP: " + QString::fromStdString(specialEquipment.name));
    } else if (enemy.name.find("Safety Manager") != std::string::npos) {
        dropType = SpecialDropType::SAFETY_MANAGER_DROP;
        specialEquipment = specialDropsLibrary->generateSafetyManagerEquipment(player->getSeries());

        updateBattleLog("🛡️ SPECIAL DROP: " + QString::fromStdString(specialEquipment.name));
    } else if (enemy.name.find("Conference") != std::string::npos) {
        dropType = SpecialDropType::CONFERENCE_DROP;
        specialEquipment = specialDropsLibrary->generateConferenceEquipment(player->getSeries());

        updateBattleLog("🎤 SPECIAL DROP: " + QString::fromStdString(specialEquipment.name));
    } else if (enemy.name.find("Paper") != std::string::npos) {
        dropType = SpecialDropType::PAPER_WRITING_DROP;
        specialEquipment = specialDropsLibrary->generatePaperWritingEquipment(player->getSeries());

        updateBattleLog("📄 SPECIAL DROP: " + QString::fromStdString(specialEquipment.name));
    } else if (enemy.type == MonsterType::REVIEW) {
        dropType = SpecialDropType::MONTHLY_REVIEW_DROP;
        specialEquipment = specialDropsLibrary->generateMonthlyReviewEquipment(currentYear, currentMonth, player->getSeries());

        updateBattleLog("📅 MONTHLY BOSS DROP: " + QString::fromStdString(specialEquipment.name));
    } else {
        return; // Not a special drop
    }

    // Add to special drops library
    addSpecialDropToLibrary(dropType, specialEquipment, enemy.name, currentYear, currentMonth);

    // Add to player's equipment inventory
    player->addEquipmentToInventory(specialEquipment);

    updateBattleLog("✨ Item added to Special Drops Library (UNSELLABLE)");
    updateBattleLog("💡 View in Special Drops tab!");

    // Update special drops display
    refreshSpecialDropsList(); // Refresh the list with new items
    updateSpecialDropsDisplay(); // Update the count and details
}

void MainWindow::handleBattle() {

    if (!player || !currentEncounter || !inBattle) return;
    if (!currentEncounter->hasAliveEnemies()) return;

    if (playerTurn) {
        // Process player's turn effects
        player->processTurnEffects();

        if (!player->isAlive()) {
            handleDefeat();
            return;
        }

        // Player attacks lowest HP enemy
        if (playerAttack()) {
            handleVictory();
            return;
        }
    } else {
        // All alive enemies attack
        auto aliveEnemies = currentEncounter->getAliveEnemies();

        for (auto* enemy : aliveEnemies) {
            // Process enemy turn effects
            enemy->processTurnEffects();

            // Check if died from DoT
            if (!enemy->isAlive()) {
                updateBattleLog(QString("%1 died from DoT effects!")
                    .arg(QString::fromStdString(enemy->getName())));
                continue;
            }

            // Enemy attacks
            monsterAttack(enemy);

            if (!player->isAlive()) {
                handleDefeat();
                return;
            }
        }

        // Check if all enemies died from DoT
        if (!currentEncounter->hasAliveEnemies()) {
            handleVictory();
            return;
        }

        // Auto-use healing if critical
        if (player->getRunway() < player->getMaxRunway() * 0.3) {
            tryEmergencyHealing();
        }

        if (player->getRunway() < 10 && player->getInventoryCount() > 0) {
            updateBattleLog("🚨 EMERGENCY SALE! Selling inventory...");
            int emergencyFunds = player->sellAllInventory();
            int runwayRecovered = emergencyFunds / 10;
            player->setRunway(std::min(player->getRunway() + runwayRecovered,
                                      player->getMaxRunway()));
            updateBattleLog(QString("💚 Recovered %1 runway").arg(runwayRecovered));
        }
    }

    playerTurn = !playerTurn;
    if (playerTurn) {
        roundNumber++;

        // Display effects every 3 rounds
        if (roundNumber % 3 == 0) {
            displayActiveEffects();
        }
    }

    updateBattleDisplay();
}

void MainWindow::handleVictory() {
    if (!currentEncounter) return;

    auto allEnemies = currentEncounter->getAllEnemies();
    int enemyCount = currentEncounter->getTotalCount();

    updateBattleLog(QString("🎉 VICTORY! Defeated %1 enemies in %2 rounds")
        .arg(enemyCount).arg(roundNumber));

    // Animate all enemy deaths
    for (size_t i = 0; i < enemyFrames_.size() && i < allEnemies.size(); ++i) {
        if (allEnemies[i]->isAlive() || enemyFrames_[i].healthBar->value() > 0) {
            animateProgressBarWithDeath(enemyFrames_[i].healthBar, 0,
                                       enemyFrames_[i].healthAnimation);
        }
    }

    // Calculate total rewards
    int moneyReward = currentEncounter->getTotalMoneyReward();
    int fundingReward = moneyReward / 10;

    updateBattleLog(QString("💰 Earned $%1 | 💎 Funding +%2")
        .arg(moneyReward).arg(fundingReward));

    player->addMoney(moneyReward);

    auto now = std::chrono::steady_clock::now();
    double elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
        now - gameStartTime).count();
    player->setElapsedSeconds(elapsedSeconds);
    player->addFunding(fundingReward);

    // Handle special drops
    bool hasSpecialDrop = false;
    for (auto* enemy : allEnemies) {
        if (enemy->type == MonsterType::NEMESIS ||
            enemy->type == MonsterType::REVIEW ||
            enemy->name.find("Shadow:") != std::string::npos ||
            enemy->name.find("Safety Manager") != std::string::npos) {
            hasSpecialDrop = true;
            handleSpecialDrop(*enemy);
        }
    }

    // Handle named monster victory
    if (namedMonsterLibrary && namedMonsterLibrary->hasBlockingEncounter()) {
        auto* def = namedMonsterLibrary->getBlockingMonster();
        updateBattleLog("📜 NAMED MONSTER DEFEATED!");

        Equipment cert = specialDropsLibrary->generateNamedMonsterCertificate(
            def->name, def->equipmentType, def->equipmentRarity, player->getSeries()
        );

        const double SECONDS_PER_YEAR = 365.25 * 24 * 60 * 60;
        const double SECONDS_PER_MONTH = SECONDS_PER_YEAR / 12.0;
        int currentYear = static_cast<int>(elapsedSeconds / SECONDS_PER_YEAR) + 1;
        int currentMonth = static_cast<int>((elapsedSeconds - ((currentYear - 1) *
            SECONDS_PER_YEAR)) / SECONDS_PER_MONTH) + 1;
        currentMonth = std::clamp(currentMonth, 1, 12);

        addSpecialDropToLibrary(SpecialDropType::NAMED_MONSTER_DROP, cert,
                               def->name, currentYear, currentMonth);
        player->addEquipmentToInventory(cert);
        namedMonsterLibrary->recordDefeat(def->name);

        updateBattleLog(QString("🏆 Received: %1")
            .arg(QString::fromStdString(cert.name)));
    }

    // Regular item drops from mobs
    auto itemRewards = currentEncounter->getAllItemRewards();
    for (const auto& itemName : itemRewards) {
        Monster* dropperMob = nullptr;
        for (auto* enemy : allEnemies) {
            if (enemy->type == MonsterType::MOB && enemy->itemReward == itemName) {
                dropperMob = enemy;
                break;
            }
        }

        if (dropperMob) {
            updateBattleLog(QString("📦 Item: %1")
                .arg(QString::fromStdString(itemName)));
            player->addInventoryItem(itemName, dropperMob->itemValue);
        }
    }

    // Equipment drops from bosses
    for (auto* enemy : allEnemies) {
        if (enemy->type == MonsterType::BOSS && !hasSpecialDrop) {
            Equipment reward = equipGen->generateBossDropEquipment(player->getSeries());
            updateBattleLog(QString("⚔️ Equipment: %1")
                .arg(QString::fromStdString(reward.name)));
            player->addEquipmentToInventory(reward);
            bossesDefeated++;
        }
    }

    totalVictories++;
    QTimer::singleShot(2500, this, [this]() {
        inBattle = false;
        currentEncounter.reset();
        waitingForNextBattle = true;
        updateBattleDisplay();
    });
}

void MainWindow::handleDefeat() {
    QString defeatedBy = "unknown";
    if (currentEncounter && currentEncounter->hasAliveEnemies()) {
        auto aliveEnemies = currentEncounter->getAliveEnemies();
        if (!aliveEnemies.empty()) {
            defeatedBy = QString::fromStdString(aliveEnemies[0]->getName());
            if (aliveEnemies.size() > 1) {
                defeatedBy += QString(" (+%1 others)").arg(aliveEnemies.size() - 1);
            }
        }
    }

    updateBattleLog(QString("💀 BANKRUPTCY! Defeated by %1").arg(defeatedBy));
    updateBattleLog("💸 All assets must be liquidated...");

    // Animate player death to 0
    animateProgressBarWithDeath(runwayBar, 1, runwayAnimation);

    bankruptcies++;
    totalDefeats++;
    player->handleDeath();
    needsRecovery = true;

    updateBattleLog("🥼 Must visit town to recover!");

    inBattle = false;
    currentEncounter.reset();
    updateBattleDisplay();
}

void MainWindow::initializeGame() {
    if (gameInitialized) return;

    updateBattleLog("🎮 Initializing game systems...");

    // Initialize equipment generator
    equipGen = std::make_unique<EquipmentGenerator>();
    if (!equipGen->loadWordLists("config/equipment/")) {
        updateBattleLog("⚠️ Failed to load equipment config!");
        return;
    }
    updateBattleLog("✅ Equipment system loaded");

    // Initialize monster generator
    monsterGen = std::make_unique<MonsterGenerator>();
    if (!monsterGen->loadWordLists("config/mobs/")) {
        updateBattleLog("⚠️ Failed to load monster config!");
        return;
    }
    updateBattleLog("✅ Monster system loaded");

    // Initialize item generator
    itemGen = std::make_unique<ItemGenerator>();
    if (!itemGen->loadWordLists("config/items/")) {
        updateBattleLog("⚠️ Failed to load item config!");
        return;
    }
    updateBattleLog("✅ Item system loaded");

    // Initialize special monster generator
    specialMonsterGen = std::make_unique<SpecializedMonsterGenerator>();
    if (!specialMonsterGen->loadWordLists("config/mobs/")) {
        updateBattleLog("⚠️ Warning: Special monster config not fully loaded");
    }
    updateBattleLog("✅ Special encounter system loaded");

    // Initialize nemesis system
    nemesisSystem = std::make_unique<NemesisSystem>();
    updateBattleLog("✅ Nemesis system initialized");

    // Initialize review calendar system
    reviewCalendarSystem = std::make_unique<ReviewCalendarSystem>();
    updateBattleLog("✅ Monthly review system initialized");

    // Initialize shadow player system
    shadowPlayerSystem = std::make_unique<ShadowPlayerSystem>();
    updateBattleLog("✅ Shadow player system initialized");

    namedMonsterLibrary = std::make_unique<NamedMonsterLibrary>();
    if (!namedMonsterLibrary->loadFromFile("config/mobs/named_monsters.txt")) {
        updateBattleLog("ℹ️ No named monsters configured");
    } else {
        updateBattleLog("✅ Named monster system loaded");
    }

    // Initialize special drops library
    specialDropsLibrary = std::make_unique<SpecialDropsLibrary>();
    updateBattleLog("✅ Special drops library initialized");

    // Load patent names
    Patent::loadPatentNames("config/items/patent_names.txt");
    updateBattleLog("✅ Patent system loaded");
    Patent::PatentLibrary = std::make_unique<PatentLibrary>();
    if (!Patent::PatentLibrary->loadFromFile("config/items/named_patents.txt")) {
        updateBattleLog("⚠️ No named patents loaded - using random only");
    } else {
        updateBattleLog("✅ Named patent library loaded");
    }
    // Create player character
    player = std::make_unique<Character>("John Bexx", "10", "Rocket Biologist", "Zap Energy");
    updateBattleLog("🆕 Character Created: " + QString::fromStdString(player->getName()));

    player->addMoney(5000);
    updateBattleLog("💰 Starting cash: $5000");

    updateBattleLog("💎 Receiving initial training...");
    player->addFunding(10);

    // Cache initial required funding
    cachedRequiredFunding = player->getRequiredFunding();
    lastSeries = player->getSeries();

    updateBattleLog("🛍️ Purchasing starting equipment...");

    // Purchase starting equipment
    Equipment laptop = equipGen->generateSpecificEquipmentForPlayer(EquipmentType::LAPTOP, player->getSeries());
    Equipment backpack = equipGen->generateSpecificEquipmentForPlayer(EquipmentType::BACKPACK, player->getSeries());
    Equipment phone = equipGen->generateSpecificEquipmentForPlayer(EquipmentType::PHONE, player->getSeries());
    Equipment bike = equipGen->generateSpecificEquipmentForPlayer(EquipmentType::BIKE, player->getSeries());
    Equipment helmet = equipGen->generateSpecificEquipmentForPlayer(EquipmentType::HELMET, player->getSeries());

    player->purchaseEquipment(laptop);
    player->equipItem(laptop);
    player->purchaseEquipment(backpack);
    player->equipItem(backpack);
    player->purchaseEquipment(phone);
    player->equipItem(phone);
    player->purchaseEquipment(bike);
    player->equipItem(bike);
    player->purchaseEquipment(helmet);
    player->equipItem(helmet);

    Patent::generateStartingPatents(player.get());
    updateBattleLog("✅ Starting patents acquired!");

    // Generate review calendar for the entire game
    reviewCalendarSystem->generateReviewCalendar(player->getSeries());
    updateBattleLog("📅 Generated 240 monthly performance reviews");

    gameStartTime = std::chrono::steady_clock::now();
    gameInitialized = true;

    updateBattleLog("✅ Initialization complete!");
    updateBattleLog("🎯 Goal: Reach Series 20");
    updateBattleLog("⚔️ Starting adventure...");

    updateDisplay();
    updateSpecialDropsDisplay();
}

void MainWindow::monsterAttack(Monster* enemy) {
    if (!enemy) return;

    // Strategic patent use
    bool usedPatent = false;
    if (enemy->canUsePatents() && enemy->getStrategy()) {
        usedPatent = enemy->useStrategicPatent(player.get());
        if (usedPatent) {
            updateBattleLog(QString("%1 used strategic magic!")
                .arg(QString::fromStdString(enemy->getName())));
        }
    }

    // Normal attack if no patent
    if (!usedPatent) {
        int baseDamage = enemy->getModifiedHustle();
        int defenseReduction = static_cast<int>(player->getModifiedNDAs() * 1.0);
        std::uniform_int_distribution<> variance(-4, 4);
        int randomVariance = variance(rng);
        int monsterDamage = std::max(1, baseDamage - defenseReduction + randomVariance);

        player->takeDamage(monsterDamage);
        updateBattleLog(QString("💥 %1 dealt %2 damage")
            .arg(QString::fromStdString(enemy->getName()))
            .arg(monsterDamage));
    }

    checkDefensivePatents();
}

void MainWindow::pauseGame() {
    gamePaused = true;
    gameTimer->stop();
    startButton->setEnabled(true);
    pauseButton->setEnabled(false);
    updateBattleLog("⏸️ Game paused");
}

bool MainWindow::playerAttack() {
    if (!currentEncounter) return false;

    if (debugDefeatCheckbox->isChecked()) {
        updateBattleLog("⚡ DEBUG: Instant defeat all enemies!");
        auto allEnemies = currentEncounter->getAllEnemies();
        for (auto* enemy : allEnemies) {
            enemy->takeDamage(9999);
        }
        return !currentEncounter->hasAliveEnemies();
    }

    bool usedPatent = false;
    float healthPercent = static_cast<float>(player->getRunway()) / player->getMaxRunway();
    float hypePercent = static_cast<float>(player->getHypeEnergy()) / player->getMaxHypeEnergy();

    // Try to use patent if conditions right
    if (player->canUsePatents() && hypePercent > 0.2f) {
        std::uniform_int_distribution<> patentChance(0, 99);
        int usePatentRoll = patentChance(rng);

        int patentChanceThreshold = 30;
        if (healthPercent < 0.3f) patentChanceThreshold = 70;
        else if (healthPercent < 0.5f) patentChanceThreshold = 50;
        if (hypePercent < 0.3f) patentChanceThreshold = 15;

        if (usePatentRoll < patentChanceThreshold) {
            std::vector<std::pair<std::string, Patent*>> affordablePatents;
            for (auto& patentPair : Patent::characterPatents) {
                if (player->getHypeEnergy() >= patentPair.second.getCost()) {
                    affordablePatents.push_back({patentPair.first, &patentPair.second});
                }
            }

            if (!affordablePatents.empty()) {
                std::string selectedPatentName;

                // Check for AOE if multiple enemies
                if (currentEncounter->getAliveCount() > 2) {
                    for (const auto& [name, patent] : affordablePatents) {
                        if (patent->getType() == PatentType::AOE) {
                            selectedPatentName = name;
                            updateBattleLog("🌊 Multiple targets - using AOE!");
                            break;
                        }
                    }
                }

                // Smart selection (healing, defense, attack logic)
                if (selectedPatentName.empty()) {
                    // ... existing smart selection logic ...
                    if (healthPercent < 0.35f) {
                        auto healPatents = Patent::getPatentsByType(PatentType::HEAL);
                        for (const auto& healName : healPatents) {
                            for (const auto& [name, patent] : affordablePatents) {
                                if (name == healName) {
                                    selectedPatentName = name;
                                    break;
                                }
                            }
                            if (!selectedPatentName.empty()) break;
                        }
                    }
                }

                if (selectedPatentName.empty()) {
                    std::uniform_int_distribution<> patentSelect(0, affordablePatents.size() - 1);
                    selectedPatentName = affordablePatents[patentSelect(rng)].first;
                }

                // Use selected patent
                auto& selectedPatent = Patent::characterPatents[selectedPatentName];

                if (selectedPatent.getType() == PatentType::AOE) {
                    // AOE attack all enemies
                    std::vector<CombatEntity*> targets;
                    for (auto* enemy : currentEncounter->getAliveEnemies()) {
                        targets.push_back(enemy);
                    }
                    selectedPatent.applyAOEEffect(player.get(), targets);
                } else {
                    // Single target
                    CombatEntity* target = currentEncounter->getLowestHPEnemy();
                    if (selectedPatent.getTarget() == PatentTarget::SELF) {
                        target = player.get();
                    }
                    Patent::usePatent(selectedPatentName, player.get(), target);
                }

                usedPatent = true;
                updateBattleLog(QString("✨ Player used %1!")
                    .arg(QString::fromStdString(selectedPatent.getFullName())));
            }
        }
    }

    // Normal attack if no patent
    if (!usedPatent) {
        Monster* target = currentEncounter->getLowestHPEnemy();
        if (!target) return false;

        int baseDamage = player->getModifiedHustle();
        std::uniform_int_distribution<> variance(-4, 4);
        int randomVariance = variance(rng);
        int defenseReduction = static_cast<int>(target->getModifiedNDAs() * 1.0);
        int playerDamage = std::max(1, baseDamage - defenseReduction + randomVariance);

        target->takeDamage(playerDamage);
        updateBattleLog(QString("⚔️ Player dealt %1 damage to %2")
            .arg(playerDamage)
            .arg(QString::fromStdString(target->getName())));
    }

    return !currentEncounter->hasAliveEnemies();
}

void MainWindow::purchaseNewEquipment() {
    if (player->getSeries() % 8 == 1) {
        purchasedThisCycle.clear();
        lastEquipmentPurchaseSeries = player->getSeries();
    }

    std::vector<EquipmentType> allTypes = {EquipmentType::LAPTOP, EquipmentType::PHONE, EquipmentType::BACKPACK, EquipmentType::HELMET, EquipmentType::BIKE, EquipmentType::JACKET, EquipmentType::GLOVES, EquipmentType::HEADPHONES};

    std::shuffle(allTypes.begin(), allTypes.end(), rng);

    for (const auto &type: allTypes) {
        if (purchasedThisCycle.find(type) != purchasedThisCycle.end()) {
            continue;
        }

        Equipment newEquip = equipGen->generateSpecificEquipmentForPlayer(type, player->getSeries());

        if (player->getMoney() >= newEquip.equipmentvalue) {
            if (player->purchaseEquipment(newEquip)) {
                player->equipItem(newEquip);
                purchasedThisCycle.insert(type);

                updateBattleLog(QString("🛍️ Purchased and equipped: %1 ($%2)").arg(QString::fromStdString(newEquip.name)).arg(newEquip.equipmentvalue));
                break;
            }
        }
    }
}

void MainWindow::refreshSpecialDropsList() {
    if (!specialDropsLibrary) return;

    // Block signals to prevent selection change events during update
    specialDropsList->blockSignals(true);

    specialDropsList->clear();
    const auto &library = specialDropsLibrary->getLibrary();

    for (const auto &drop: library) {
        QString icon = QString::fromStdString(specialDropsLibrary->getDropTypeIcon(drop.type));
        QString name = QString::fromStdString(drop.name);
        QString equipped = drop.isEquipped ? " ✓" : "";

        specialDropsList->addItem(QString("%1 %2%3").arg(icon).arg(name).arg(equipped));
    }

    specialDropsList->blockSignals(false);
}

void MainWindow::setupUI() {
    // Create central widget with tab widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Control buttons at top
    QHBoxLayout *controlLayout = new QHBoxLayout();
    startButton = new QPushButton("Start Adventure");
    startButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; }");
    pauseButton = new QPushButton("Pause");
    pauseButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; padding: 8px; }");
    pauseButton->setEnabled(false);

    // ADD: Debug instant defeat checkbox
    debugDefeatCheckbox = new QCheckBox("⚡ Instant Defeat (Debug)");
    debugDefeatCheckbox->setStyleSheet("QCheckBox { color: #FF9800; font-weight: bold; padding: 8px; }");
    debugDefeatCheckbox->setChecked(true); // Default to ON

    controlLayout->addWidget(startButton);
    controlLayout->addWidget(pauseButton);
    controlLayout->addWidget(debugDefeatCheckbox); // ADD: Add debug checkbox
    controlLayout->addStretch();
    mainLayout->addLayout(controlLayout);

    // Create tab widget
    mainTabs = new QTabWidget();
    mainTabs->setStyleSheet("QTabWidget::pane { border: 2px solid #555; } QTabBar::tab { padding: 8px 16px; }");

    // Setup gameplay tab (existing UI)
    gameplayTab = new QWidget();
    setupGameplayTab();
    mainTabs->addTab(gameplayTab, "🎮 Gameplay");

    // Setup special drops tab (new)
    specialDropsTab = new QWidget();
    setupSpecialDropsTab();
    mainTabs->addTab(specialDropsTab, "✨ Special Drops Library");

    equipmentTab = new QWidget();
    setupEquipmentTab();
    mainTabs->addTab(equipmentTab, "⚔️ Equipment");

    mainLayout->addWidget(mainTabs);
}

void MainWindow::setupGameplayTab() {
    QHBoxLayout *gameLayout = new QHBoxLayout(gameplayTab);

    // Left panel - Character info
    QVBoxLayout *leftPanel = new QVBoxLayout();

    // Character Stats
    characterStatsGroup = new QGroupBox("Character Status");
    characterStatsGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 2px solid #555; border-radius: 5px; margin-top: 20px; } QGroupBox::title { subcontrol-origin: margin; padding: 0 5px; }");
    QGridLayout *statsLayout = new QGridLayout();

    nameLabel = new QLabel("Name: ---");
    nameLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #FFD700;");
    statsLayout->addWidget(nameLabel, 0, 0, 1, 2);

    seriesLabel = new QLabel("1");
    statsLayout->addWidget(new QLabel("📊 Series:"), 1, 0);
    statsLayout->addWidget(seriesLabel, 1, 1);

    runwayLabel = new QLabel("100/100");
    runwayBar = new QProgressBar();
    runwayBar->setMaximum(100);
    runwayBar->setStyleSheet("QProgressBar::chunk { background-color: #4CAF50; }");
    statsLayout->addWidget(new QLabel("🏃 Runway:"), 2, 0);
    statsLayout->addWidget(runwayLabel, 2, 1);
    statsLayout->addWidget(runwayBar, 3, 0, 1, 2);

    hypeEnergyLabel = new QLabel("100/100");
    hypeBar = new QProgressBar();
    hypeBar->setMaximum(100);
    hypeBar->setStyleSheet("QProgressBar::chunk { background-color: #4CAF50; }");
    statsLayout->addWidget(new QLabel("⚡ Hype Energy:"), 4, 0);
    statsLayout->addWidget(hypeEnergyLabel, 4, 1);
    statsLayout->addWidget(hypeBar, 5, 0, 1, 2);

    moneyLabel = new QLabel("$0");
    statsLayout->addWidget(new QLabel("💰 Money:"), 6, 0);
    statsLayout->addWidget(moneyLabel, 6, 1);

    fundingLabel = new QLabel("0/100");
    fundingProgress = new QProgressBar();
    fundingProgress->setStyleSheet("QProgressBar::chunk { background-color: #9C27B0; }");
    statsLayout->addWidget(new QLabel("💎 Funding:"), 7, 0);
    statsLayout->addWidget(fundingLabel, 7, 1);
    statsLayout->addWidget(fundingProgress, 8, 0, 1, 2);

    statsLayout->addWidget(new QLabel("⚔️ Hustle:"), 9, 0);
    hustleLabel = new QLabel("10");
    statsLayout->addWidget(hustleLabel, 9, 1);

    statsLayout->addWidget(new QLabel("🔄 Agile-ity:"), 10, 0);
    agilityLabel = new QLabel("8");
    statsLayout->addWidget(agilityLabel, 10, 1);

    statsLayout->addWidget(new QLabel("🛡 NDAs:"), 11, 0);
    ndasLabel = new QLabel("5");
    statsLayout->addWidget(ndasLabel, 11, 1);

    statsLayout->addWidget(new QLabel("🍀 Luck:"), 12, 0);
    luckLabel = new QLabel("7");
    statsLayout->addWidget(luckLabel, 12, 1);

    statsLayout->addWidget(new QLabel("🤝 Synergy:"), 13, 0);
    synergyLabel = new QLabel("6");
    statsLayout->addWidget(synergyLabel, 13, 1);

    statsLayout->addWidget(new QLabel("🛠 Tech Debt:"), 14, 0);
    technicalDebtLabel = new QLabel("0");
    statsLayout->addWidget(technicalDebtLabel, 14, 1);

    statsLayout->addWidget(new QLabel("🚌 Bus Factor:"), 15, 0);
    busFactorLabel = new QLabel("1.0");
    statsLayout->addWidget(busFactorLabel, 15, 1);

    characterStatsGroup->setLayout(statsLayout);
    leftPanel->addWidget(characterStatsGroup);

    // Statistics
    statsGroup = new QGroupBox("Game Statistics");
    statsGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 2px solid #555; border-radius: 5px; margin-top: 20px; } QGroupBox::title { subcontrol-origin: margin; padding: 0 5px; }");
    QGridLayout *gameStatsLayout = new QGridLayout();

    gameStatsLayout->addWidget(new QLabel("⚔️ Battles:"), 0, 0);
    totalBattlesLabel = new QLabel("0");
    gameStatsLayout->addWidget(totalBattlesLabel, 0, 1);

    gameStatsLayout->addWidget(new QLabel("🎉 Victories:"), 1, 0);
    victoriesLabel = new QLabel("0");
    gameStatsLayout->addWidget(victoriesLabel, 1, 1);

    gameStatsLayout->addWidget(new QLabel("💀 Defeats:"), 2, 0);
    defeatsLabel = new QLabel("0");
    gameStatsLayout->addWidget(defeatsLabel, 2, 1);

    gameStatsLayout->addWidget(new QLabel("👑 Bosses:"), 3, 0);
    bossesDefeatedLabel = new QLabel("0");
    gameStatsLayout->addWidget(bossesDefeatedLabel, 3, 1);

    gameStatsLayout->addWidget(new QLabel("💸 Bankruptcies:"), 4, 0);
    bankruptciesLabel = new QLabel("0");
    gameStatsLayout->addWidget(bankruptciesLabel, 4, 1);

    gameStatsLayout->addWidget(new QLabel("🏪 Town Visits:"), 5, 0);
    townVisitsLabel = new QLabel("0");
    gameStatsLayout->addWidget(townVisitsLabel, 5, 1);

    gameStatsLayout->addWidget(new QLabel("⏰ Time:"), 6, 0);
    gameTimeLabel = new QLabel("0:00:00");
    gameStatsLayout->addWidget(gameTimeLabel, 6, 1);

    statsGroup->setLayout(gameStatsLayout);
    leftPanel->addWidget(statsGroup);

    leftPanel->addStretch();

    // Middle panel - Equipment and Inventory
    QVBoxLayout *middlePanel = new QVBoxLayout();

    // Equipment
    equipmentGroup = new QGroupBox("Equipped Items");
    equipmentGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 2px solid #555; border-radius: 5px; margin-top: 20px; } QGroupBox::title { subcontrol-origin: margin; padding: 0 5px; }");
    QGridLayout *equipLayout = new QGridLayout();

    equipLayout->addWidget(new QLabel("💻 Laptop:"), 0, 0);
    laptopSlot = new QLabel("Empty");
    laptopSlot->setStyleSheet("color: gray;");
    laptopSlot->setWordWrap(true);
    equipLayout->addWidget(laptopSlot, 0, 1);

    equipLayout->addWidget(new QLabel("📱 Phone:"), 1, 0);
    phoneSlot = new QLabel("Empty");
    phoneSlot->setStyleSheet("color: gray;");
    phoneSlot->setWordWrap(true);
    equipLayout->addWidget(phoneSlot, 1, 1);

    equipLayout->addWidget(new QLabel("🎒 Backpack:"), 2, 0);
    backpackSlot = new QLabel("Empty");
    backpackSlot->setStyleSheet("color: gray;");
    backpackSlot->setWordWrap(true);
    equipLayout->addWidget(backpackSlot, 2, 1);

    equipLayout->addWidget(new QLabel("🪖 Helmet:"), 3, 0);
    helmetSlot = new QLabel("Empty");
    helmetSlot->setStyleSheet("color: gray;");
    helmetSlot->setWordWrap(true);
    equipLayout->addWidget(helmetSlot, 3, 1);

    equipLayout->addWidget(new QLabel("🚲 Bike:"), 4, 0);
    bikeSlot = new QLabel("Empty");
    bikeSlot->setStyleSheet("color: gray;");
    bikeSlot->setWordWrap(true);
    equipLayout->addWidget(bikeSlot, 4, 1);

    equipLayout->addWidget(new QLabel("🧥 Jacket:"), 5, 0);
    jacketSlot = new QLabel("Empty");
    jacketSlot->setStyleSheet("color: gray;");
    jacketSlot->setWordWrap(true);
    equipLayout->addWidget(jacketSlot, 5, 1);

    equipLayout->addWidget(new QLabel("🧤 Gloves:"), 6, 0);
    glovesSlot = new QLabel("Empty");
    glovesSlot->setStyleSheet("color: gray;");
    glovesSlot->setWordWrap(true);
    equipLayout->addWidget(glovesSlot, 6, 1);

    equipLayout->addWidget(new QLabel("🎧 Headphones:"), 7, 0);
    headphonesSlot = new QLabel("Empty");
    headphonesSlot->setStyleSheet("color: gray;");
    headphonesSlot->setWordWrap(true);
    equipLayout->addWidget(headphonesSlot, 7, 1);

    equipmentGroup->setLayout(equipLayout);
    middlePanel->addWidget(equipmentGroup);

    // Inventory
    inventoryGroup = new QGroupBox("Inventory");
    inventoryGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 2px solid #555; border-radius: 5px; margin-top: 20px; } QGroupBox::title { subcontrol-origin: margin; padding: 0 5px; }");
    QVBoxLayout *invLayout = new QVBoxLayout();
    inventoryCountLabel = new QLabel("Items: 0/5");
    inventoryCountLabel->setStyleSheet("font-weight: bold;");
    invLayout->addWidget(inventoryCountLabel);
    inventoryList = new QListWidget();
    inventoryList->setMaximumHeight(130);
    invLayout->addWidget(inventoryList);
    inventoryGroup->setLayout(invLayout);
    middlePanel->addWidget(inventoryGroup);

    // Patents
    patentsGroup = new QGroupBox("Patents");
    patentsGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 2px solid #555; border-radius: 5px; margin-top: 20px } QGroupBox::title { subcontrol-origin: margin; padding: 0 5px; }");
    QVBoxLayout *patentsLayout = new QVBoxLayout();
    patentsList = new QListWidget();
    patentsList->setMaximumHeight(150);
    patentsLayout->addWidget(patentsList);
    patentsGroup->setLayout(patentsLayout);
    middlePanel->addWidget(patentsGroup);
    middlePanel->addStretch();

    // Right panel - Battle and Log
    QVBoxLayout *rightPanel = new QVBoxLayout();
    rightPanel->setSpacing(5);
    rightPanel->setContentsMargins(0, 0, 0, 0);

    // Current Battle
    battleGroup = new QGroupBox("Current Battle");
    battleGroup->setStyleSheet(
        "QGroupBox { "
        "font-weight: bold; "
        "border: 2px solid #555; "
        "border-radius: 5px; "
        "margin-top: 20px; "
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "padding: 0 5px; "
        "}"
    );
    battleGroup->setFixedHeight(300);
    battleGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QVBoxLayout* battleLayout = new QVBoxLayout();
    battleLayout->setContentsMargins(5, 5, 5, 5);
    battleLayout->setSpacing(5);

    // Waiting label
    waitingForBattleLabel = new QLabel("Waiting for battle...");
    waitingForBattleLabel->setStyleSheet("color: gray; font-size: 14px;");
    waitingForBattleLabel->setAlignment(Qt::AlignCenter);

    // Round number display
    roundNumberLabel = new QLabel("Round: 0");
    roundNumberLabel->setStyleSheet("font-weight: bold; color: #FF9800; font-size: 12px;");
    roundNumberLabel->setAlignment(Qt::AlignCenter);

    // Create enemy frames and grid
    setupEnemyFrames();

    // Add everything to battle layout in correct order
    battleLayout->addLayout(enemyGridLayout_);
    battleLayout->addWidget(waitingForBattleLabel);
    battleLayout->addStretch();
    battleLayout->addWidget(roundNumberLabel);

    // Set the layout to battleGroup ONCE
    battleGroup->setLayout(battleLayout);

    // Initialize animations
    runwayAnimation = new QPropertyAnimation(runwayBar, "value", this);
    runwayAnimation->setDuration(500);
    runwayAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    hypeAnimation = new QPropertyAnimation(hypeBar, "value", this);
    hypeAnimation->setDuration(500);
    hypeAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    fundingAnimation = new QPropertyAnimation(fundingProgress, "value", this);
    fundingAnimation->setDuration(500);
    fundingAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // Battle Log
    QGroupBox *logGroup = new QGroupBox("Battle Log");
    logGroup->setStyleSheet(
        "QGroupBox { "
        "font-weight: bold; "
        "border: 2px solid #555; "
        "border-radius: 5px; "
        "margin-top: 20px; "
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "padding: 0 5px; "
        "}"
    );
    logGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    QVBoxLayout *logLayout = new QVBoxLayout();
    battleLog = new QTextEdit();
    battleLog->setReadOnly(true);
    battleLog->setStyleSheet("QTextEdit { background-color: #1E1E1E; color: #E0E0E0; font-family: 'Courier New'; }");

    logLayout->addWidget(battleLog);
    logGroup->setLayout(logLayout);

    // Add to right panel
    rightPanel->addWidget(battleGroup);
    rightPanel->addWidget(logGroup);
    rightPanel->setStretch(0, 0); // battle fixed height
    rightPanel->setStretch(1, 1); // log expands

    // Add panels to game layout
    gameLayout->addLayout(leftPanel, 2);
    gameLayout->addLayout(middlePanel, 2);
    gameLayout->addLayout(rightPanel, 3);
}

void MainWindow::setupSpecialDropsTab() {
    QHBoxLayout *dropLayout = new QHBoxLayout(specialDropsTab);

    // Left side - List of special drops
    QVBoxLayout *leftPanel = new QVBoxLayout();

    specialDropsCountLabel = new QLabel("Special Items: 0");
    specialDropsCountLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #FFD700;");
    leftPanel->addWidget(specialDropsCountLabel);

    specialDropsList = new QListWidget();
    specialDropsList->setStyleSheet("QListWidget { font-size: 12px; }");

    // Connect to the display update function
    connect(specialDropsList, &QListWidget::currentRowChanged, this, &MainWindow::updateSpecialDropsDisplay);

    leftPanel->addWidget(specialDropsList);

    dropLayout->addLayout(leftPanel, 1);

    // Right side - Details of selected drop
    QVBoxLayout *rightPanel = new QVBoxLayout();

    QLabel *detailsTitle = new QLabel("Item Details");
    detailsTitle->setStyleSheet("font-weight: bold; font-size: 14px; color: #4CAF50;");
    rightPanel->addWidget(detailsTitle);

    specialDropDetails = new QTextEdit();
    specialDropDetails->setReadOnly(true);
    specialDropDetails->setStyleSheet("QTextEdit { background-color: #1E1E1E; color: #E0E0E0; font-family: 'Courier New'; font-size: 10px; }");
    rightPanel->addWidget(specialDropDetails);

    dropLayout->addLayout(rightPanel, 2);

    // Initialize the display
    refreshSpecialDropsList();
    updateSpecialDropsDisplay();
}

void MainWindow::setupEquipmentTab() {
    QVBoxLayout* mainLayout = new QVBoxLayout(equipmentTab);

    QLabel* title = new QLabel("Equipped Items");
    title->setStyleSheet("font-weight: bold; font-size: 16px; color: #FFD700;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // Central circle container
    equipmentCircle = new QWidget();
    equipmentCircle->setMinimumSize(620, 620);
    equipmentCircle->setStyleSheet("background-color: #1a1a1a; border-radius: 300px;");

    // Position slots in circle
    QPoint center(310, 300);
    int radius = 220;

    EquipmentType types[] = {
        EquipmentType::LAPTOP,    // 0° (top)
        EquipmentType::PHONE,     // 45°
        EquipmentType::BACKPACK,  // 90° (right)
        EquipmentType::HELMET,    // 135°
        EquipmentType::BIKE,      // 180° (bottom)
        EquipmentType::JACKET,    // 225°
        EquipmentType::GLOVES,    // 270° (left)
        EquipmentType::HEADPHONES // 315°
    };

    const char* icons[] = {"💻", "📱", "🎒", "🪖", "🚲", "🧥", "🧤", "🎧"};

    for (int i = 0; i < 8; i++) {
        double angle = (i * 45.0 - 90.0) * M_PI / 180.0; // -90 to start at top
        int x = center.x() + radius * cos(angle) - 60;
        int y = center.y() + radius * sin(angle) - 60;

        // Slot frame
        QFrame* slotFrame = new QFrame(equipmentCircle);
        slotFrame->setGeometry(x, y, 120, 140);
        slotFrame->setStyleSheet(
            "QFrame { "
            "border: 2px solid #555; "
            "border-radius: 10px; "
            "background-color: #2a2a2a; "
            "padding: 1px; "
            "}"
        );

        QVBoxLayout* slotLayout = new QVBoxLayout(slotFrame);
        slotLayout->setSpacing(2);

        // Icon
        QLabel* icon = new QLabel(icons[i]);
        icon->setStyleSheet("font-size: 24px; border: none;");
        icon->setAlignment(Qt::AlignCenter);
        slotLayout->addWidget(icon);

        // Name label
        QLabel* nameLabel = new QLabel("Empty");
        nameLabel->setStyleSheet("font-size: 9px; color: gray; border: none;");
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setWordWrap(true);
        equipmentSlots.push_back(nameLabel);
        slotLayout->addWidget(nameLabel);

        // Stats label
        QLabel* statsLabel = new QLabel("");
        statsLabel->setStyleSheet("font-size: 8px; color: #4CAF50; border: none;");
        statsLabel->setAlignment(Qt::AlignCenter);
        statsLabel->setWordWrap(true);
        equipmentStats.push_back(statsLabel);
        slotLayout->addWidget(statsLabel);
    }

    mainLayout->addWidget(equipmentCircle, 0, Qt::AlignCenter);
    mainLayout->addStretch();
}

void MainWindow::setupEnemyFrames() {
    // Create 2x2 grid for enemies
    enemyGridLayout_ = new QGridLayout();
    enemyGridLayout_->setSpacing(10);
    enemyGridLayout_->setContentsMargins(0, 0, 0, 0);

    // Set grid layout properties to prevent stretching
    enemyGridLayout_->setColumnStretch(0, 0);
    enemyGridLayout_->setColumnStretch(1, 0);
    enemyGridLayout_->setRowStretch(0, 0);
    enemyGridLayout_->setRowStretch(1, 0);

    for (int i = 0; i < 4; ++i) {
        EnemyFrame frame;

        // Container widget with fixed size policy
        frame.container = new QWidget();
        frame.container->setStyleSheet(
            "QWidget { "
            "border: 2px solid #555; "
            "border-radius: 5px; "
            "background-color: #2a2a2a; "
            "padding: 5px; "
            "}"
        );
        frame.container->setMinimumSize(180, 120);
        frame.container->setMaximumWidth(250);  // Increased max width
        frame.container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QVBoxLayout* frameLayout = new QVBoxLayout(frame.container);
        frameLayout->setContentsMargins(5, 5, 5, 5);
        frameLayout->setSpacing(2);

        // Symbol + Name row
        QHBoxLayout* headerLayout = new QHBoxLayout();
        headerLayout->setSpacing(4);
        frame.symbolLabel = new QLabel("👾");
        frame.symbolLabel->setStyleSheet("font-size: 20px; border: none;");
        frame.symbolLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        headerLayout->addWidget(frame.symbolLabel);

        frame.nameLabel = new QLabel("Empty");
        frame.nameLabel->setStyleSheet("font-weight: bold; font-size: 11px; color: gray; border: none;");
        frame.nameLabel->setWordWrap(true);
        frame.nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        headerLayout->addWidget(frame.nameLabel, 1);
        frameLayout->addLayout(headerLayout);

        // Health display
        frame.healthLabel = new QLabel("HP: ---/---");
        frame.healthLabel->setStyleSheet("font-size: 10px; border: none;");
        frame.healthLabel->setAlignment(Qt::AlignCenter);
        frameLayout->addWidget(frame.healthLabel);

        frame.healthBar = new QProgressBar();
        frame.healthBar->setStyleSheet("QProgressBar::chunk { background-color: #F44336; } QProgressBar { border: none; }");
        frame.healthBar->setMaximumHeight(12);
        frame.healthBar->setTextVisible(false);
        frameLayout->addWidget(frame.healthBar);

        // Stats row
        QHBoxLayout* statsLayout = new QHBoxLayout();
        statsLayout->setSpacing(4);
        frame.attackLabel = new QLabel("⚔️ ---");
        frame.attackLabel->setStyleSheet("font-size: 9px; border: none;");
        frame.defenseLabel = new QLabel("🛡️ ---");
        frame.defenseLabel->setStyleSheet("font-size: 9px; border: none;");
        statsLayout->addWidget(frame.attackLabel);
        statsLayout->addWidget(frame.defenseLabel);
        frameLayout->addLayout(statsLayout);

        frameLayout->addStretch(1);

        // Animation
        frame.healthAnimation = new QPropertyAnimation(frame.healthBar, "value", this);
        frame.healthAnimation->setDuration(500);
        frame.healthAnimation->setEasingCurve(QEasingCurve::InOutQuad);

        // Initialize state
        frame.targetHealth = -1;
        frame.isDead = false;
        frame.fadeTimer = nullptr;

        // Add to grid (2x2 layout)
        int row = i / 2;
        int col = i % 2;
        enemyGridLayout_->addWidget(frame.container, row, col, Qt::AlignCenter);

        // Hide initially
        frame.container->hide();

        enemyFrames_.push_back(frame);
    }
}

void MainWindow::startGame() {
    if (!gameInitialized) {
        initializeGame();
    }

    if (!gameInitialized) return; // Failed to initialize

    gamePaused = false;
    gameTimer->start(100); // Update every 100ms
    startButton->setEnabled(false);
    pauseButton->setEnabled(true);
    updateBattleLog("▶️ Game started!");
}

void MainWindow::tryEmergencyHealing() {
    // Priority 1: Healing patents
    auto healingPatents = Patent::getPatentsByType(PatentType::HEAL);
    for (const auto &patentName: healingPatents) {
        const Patent &patent = Patent::characterPatents[patentName];
        if (player->getHypeEnergy() >= patent.getCost()) {
            Patent::usePatent(patentName, player.get(), player.get());
            updateBattleLog(QString("💊 Emergency use of %1!").arg(QString::fromStdString(patent.getFullName())));
            return;
        }
    }

    // Priority 2: Defense patents for protection
    auto defensePatents = Patent::getPatentsByType(PatentType::DEFENSE);
    for (const auto &patentName: defensePatents) {
        const Patent &patent = Patent::characterPatents[patentName];
        if (player->getHypeEnergy() >= patent.getCost()) {
            Patent::usePatent(patentName, player.get(), player.get());
            updateBattleLog(QString("🛡️ Emergency defense with %1!").arg(QString::fromStdString(patent.getFullName())));
            return;
        }
    }

    // Priority 3: Buff patents to increase survivability
    auto buffPatents = Patent::getPatentsByType(PatentType::BUFF);
    for (const auto &patentName: buffPatents) {
        const Patent &patent = Patent::characterPatents[patentName];
        if (player->getHypeEnergy() >= patent.getCost()) {
            Patent::usePatent(patentName, player.get(), player.get());
            updateBattleLog(QString("✨ Emergency buff with %1!").arg(QString::fromStdString(patent.getFullName())));
            return;
        }
    }
}

void MainWindow::visitTown() {
    updateBattleLog("🏪 Visiting town - selling inventory and equipment...");

    int inventoryEarned = player->sellAllInventory();
    int equipmentEarned = player->sellAllEquipment();
    int totalEarned = inventoryEarned + equipmentEarned;
    purchaseNewEquipment();
    updateBattleLog(QString("💰 Total earned: $%1 | Current funds: $%2").arg(totalEarned).arg(player->getMoney()));

    townVisits++;
}

void MainWindow::visitTownForRecovery() {
    updateBattleLog("🏥 Emergency recovery - restoring to full health...");
    player->restoreToFull();
    updateBattleLog(QString("✅ Runway: %1/%2 | Hype: %3/%4").arg(player->getRunway()).arg(player->getMaxRunway()).arg(player->getHypeEnergy()).arg(player->getMaxHypeEnergy()));
    townVisits++;
}

void MainWindow::updateDisplay() {
    updateCharacterStats();
    updateEquipmentDisplay();
    updateInventoryDisplay();
    updatePatentsDisplay();

    totalBattlesLabel->setText(QString::number(totalBattles));
    victoriesLabel->setText(QString::number(totalVictories));
    defeatsLabel->setText(QString::number(totalDefeats));
    bossesDefeatedLabel->setText(QString::number(bossesDefeated));
    bankruptciesLabel->setText(QString::number(bankruptcies));
    townVisitsLabel->setText(QString::number(townVisits));
}

void MainWindow::updateCharacterStats() {
    if (!player) return;

    nameLabel->setText(QString::fromStdString(player->getName()));
    seriesLabel->setText(QString::number(player->getSeries()));

    // Runway
    runwayLabel->setText(QString("%1/%2").arg(player->getRunway()).arg(player->getMaxRunway()));
    runwayBar->setMaximum(player->getMaxRunway());
    animateProgressBar(runwayBar, player->getRunway(), runwayAnimation);

    int runwayPercent = (player->getRunway() * 100) / std::max(1, player->getMaxRunway());
    if (runwayPercent > 60) {
        runwayBar->setStyleSheet("QProgressBar::chunk { background-color: #4CAF50; }");
    } else if (runwayPercent > 30) {
        runwayBar->setStyleSheet("QProgressBar::chunk { background-color: #FF9800; }");
    } else {
        runwayBar->setStyleSheet("QProgressBar::chunk { background-color: #F44336; }");
    }

    // Hype Energy
    hypeEnergyLabel->setText(QString("%1/%2").arg(player->getHypeEnergy()).arg(player->getMaxHypeEnergy()));
    hypeBar->setMaximum(player->getMaxHypeEnergy());
    animateProgressBar(hypeBar, player->getHypeEnergy(), hypeAnimation);

    // Money
    moneyLabel->setText(QString("$%1").arg(player->getMoney()));

    // Funding
    fundingLabel->setText(QString("%1/%2").arg(static_cast<int>(player->getCurrentFunding())).arg(static_cast<int>(cachedRequiredFunding)));
    fundingProgress->setMaximum(static_cast<int>(cachedRequiredFunding));
    animateProgressBar(fundingProgress, static_cast<int>(player->getCurrentFunding()), fundingAnimation);

    // Display modified stats with indication if buffed/debuffed
    auto displayStat = [](QLabel *label, int baseValue, int modifiedValue) {
        if (modifiedValue > baseValue) {
            label->setText(QString("<span style='color: #4CAF50;'>%1 (+%2)</span>").arg(modifiedValue).arg(modifiedValue - baseValue));
        } else if (modifiedValue < baseValue) {
            label->setText(QString("<span style='color: #F44336;'>%1 (-%2)</span>").arg(modifiedValue).arg(baseValue - modifiedValue));
        } else {
            label->setText(QString::number(modifiedValue));
        }
    };

    displayStat(hustleLabel, player->getHustle(), player->getModifiedHustle());
    displayStat(agilityLabel, player->getAgility(), player->getModifiedAgility());
    displayStat(ndasLabel, player->getNDAs(), player->getModifiedNDAs());
    displayStat(luckLabel, player->getLuck(), player->getModifiedLuck());
    displayStat(synergyLabel, player->getSynergy(), player->getModifiedSynergy());

    technicalDebtLabel->setText(QString::number(player->getTechnicalDebt()));
    busFactorLabel->setText(QString::number(player->getBusFactor(), 'f', 1));

    // Show active effects count
    int effectCount = player->getEffectsManager().getEffectCount();
    if (effectCount > 0) {
        seriesLabel->setText(QString("%1 [%2 effects]").arg(player->getSeries()).arg(effectCount));
    }
}

void MainWindow::updateEquipmentDisplay() {
    if (!player) return;

    auto updateSlot = [](QLabel *label, Equipment *equip) {
        if (equip) {
            label->setText(QString::fromStdString(equip->name));
            label->setStyleSheet("color: lightgreen; font-size: 11px;");
            label->setToolTip(QString("Stats: R+%1 H+%2 Hu+%3 A+%4 N+%5 L+%6").arg(equip->runway).arg(equip->hypeEnergy).arg(equip->hustle).arg(equip->agility).arg(equip->ndas).arg(equip->luck));
        } else {
            label->setText("Empty");
            label->setStyleSheet("color: gray; font-size: 11px;");
            label->setToolTip("");
        }
    };

    updateSlot(laptopSlot, player->getEquippedItem(EquipmentType::LAPTOP));
    updateSlot(phoneSlot, player->getEquippedItem(EquipmentType::PHONE));
    updateSlot(backpackSlot, player->getEquippedItem(EquipmentType::BACKPACK));
    updateSlot(helmetSlot, player->getEquippedItem(EquipmentType::HELMET));
    updateSlot(bikeSlot, player->getEquippedItem(EquipmentType::BIKE));
    updateSlot(jacketSlot, player->getEquippedItem(EquipmentType::JACKET));
    updateSlot(glovesSlot, player->getEquippedItem(EquipmentType::GLOVES));
    updateSlot(headphonesSlot, player->getEquippedItem(EquipmentType::HEADPHONES));

    // NEW: Update equipment tab circular display
    EquipmentType types[] = {
        EquipmentType::LAPTOP, EquipmentType::PHONE, EquipmentType::BACKPACK,
        EquipmentType::HELMET, EquipmentType::BIKE, EquipmentType::JACKET,
        EquipmentType::GLOVES, EquipmentType::HEADPHONES
    };

    for (int i = 0; i < 8; i++) {
        Equipment* eq = player->getEquippedItem(types[i]);

        if (eq) {
            equipmentSlots[i]->setText(QString::fromStdString(eq->name));
            equipmentSlots[i]->setStyleSheet("font-size: 9px; color: lightgreen; border: none;");

            QString stats;
            if (eq->runway > 0) stats += QString("R+%1\n").arg(eq->runway);
            if (eq->hypeEnergy > 0) stats += QString("H+%1\n").arg(eq->hypeEnergy);
            if (eq->hustle > 0) stats += QString("Hu+%1\n").arg(eq->hustle);
            if (eq->agility > 0) stats += QString("A+%1\n").arg(eq->agility);
            if (eq->ndas > 0) stats += QString("N+%1\n").arg(eq->ndas);
            if (eq->luck > 0) stats += QString("L+%1\n").arg(eq->luck);
            if (eq->synergy > 0) stats += QString("S+%1\n").arg(eq->synergy);
            if (eq->technicalDebt != 0) stats += QString("TD%1%2\n")
                .arg(eq->technicalDebt > 0 ? "+" : "")
                .arg(eq->technicalDebt);
            if (eq->busFactor > 0) stats += QString("BF+%1\n").arg(eq->busFactor, 0, 'f', 1);
            if (eq->inventoryLimit > 0) stats += QString("Inv:%1").arg(eq->inventoryLimit);

            equipmentStats[i]->setText(stats.trimmed());
        } else {
            equipmentSlots[i]->setText("Empty");
            equipmentSlots[i]->setStyleSheet("font-size: 9px; color: gray; border: none;");
            equipmentStats[i]->setText("");
        }
    }
}

void MainWindow::updateInventoryDisplay() {
    if (!player) return;

    inventoryCountLabel->setText(QString("Items: %1/%2").arg(player->getInventoryCount()).arg(player->getInventoryLimit()));

    inventoryList->clear();
    const auto &inv = player->getInventoryMap();
    for (const auto &item: inv) {
        inventoryList->addItem(QString("%1 ($%2)").arg(QString::fromStdString(item.first)).arg(item.second));
    }
}

void MainWindow::updatePatentsDisplay() {
    patentsList->clear();
    for (const auto &patentPair: Patent::characterPatents) {
        const Patent &patent = patentPair.second;
        patentsList->addItem(QString("%1 | Power: %2").arg(QString::fromStdString(patent.getFullName())).arg(patent.getCurrentPower()));
    }
}

void MainWindow::updateBattleLog(const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    battleLog->append(QString("<span style='color: #888;'>[%1]</span> %2").arg(timestamp).arg(message));

    QScrollBar *scrollBar = battleLog->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::updateSpecialDropsDisplay() {
    if (!specialDropsLibrary) {
        // qDebug() << "Special drops library is null";
        return;
    }

    // Update count
    int totalDrops = specialDropsLibrary->getTotalDrops();
    specialDropsCountLabel->setText(QString("Special Items: %1").arg(totalDrops));

    // Update details if item selected
    int currentRow = specialDropsList->currentRow();
    const auto &library = specialDropsLibrary->getLibrary();

    // qDebug() << "Current row:" << currentRow << "Library size:" << library.size();

    if (currentRow >= 0 && currentRow < library.size()) {
        // qDebug() << "Showing details for item at row" << currentRow;
        const SpecialDrop &drop = library[currentRow];

        QString details;
        details += "╔═══════════════════════════════════════╗\n";
        details += "║         SPECIAL ITEM DETAILS          ║\n";
        details += "╚═══════════════════════════════════════╝\n\n";

        details += QString::fromStdString(specialDropsLibrary->getDropTypeIcon(drop.type)) + " ";
        details += QString::fromStdString(drop.name) + "\n";
        details += "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

        details += "Type: " + QString::fromStdString(specialDropsLibrary->getDropTypeString(drop.type)) + "\n";
        details += "ID: " + QString::number(drop.uniqueID) + "\n";
        details += "Status: " + QString(drop.isEquipped ? "✓ EQUIPPED" : "In Library") + "\n\n";

        details += "Acquired From:\n";
        details += "  " + QString::fromStdString(drop.acquiredFrom) + "\n";
        details += "  Year " + QString::number(drop.yearAcquired) + ", Month " + QString::number(drop.monthAcquired) + "\n\n";

        const Equipment &eq = drop.equipment;
        details += "Equipment Stats:\n";
        details += "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        details += QString("  Class: %1 | Rarity: %2\n").arg(QString::fromStdString(eq.equipmentclass)).arg(static_cast<int>(eq.rarity));
        details += QString("  Runway: +%1\n").arg(eq.runway);
        details += QString("  Hype Energy: +%1\n").arg(eq.hypeEnergy);
        details += QString("  Hustle: +%1\n").arg(eq.hustle);
        details += QString("  Agility: +%1\n").arg(eq.agility);
        details += QString("  NDAs: +%1\n").arg(eq.ndas);
        details += QString("  Luck: +%1\n").arg(eq.luck);
        details += QString("  Synergy: +%1\n").arg(eq.synergy);
        details += QString("  Technical Debt: %1\n").arg(eq.technicalDebt);
        details += QString("  Bus Factor: +%1\n").arg(eq.busFactor, 0, 'f', 2);

        if (eq.type == EquipmentType::BACKPACK && eq.inventoryLimit > 0) {
            details += QString("  Inventory Limit: %1\n").arg(eq.inventoryLimit);
        }

        details += "\n📖 Lore:\n";
        details += "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        details += QString::fromStdString(drop.loreText) + "\n\n";

        details += "⚠️ This item cannot be sold!\n";
        details += "It remains in your collection forever.\n";

        specialDropDetails->setText(details);
    } else {
        // qDebug() << "No item selected or invalid row";
        specialDropDetails->setText("Select an item to view details...");
    }
}

QString MainWindow::getEnemySymbol(Monster* enemy) const {
    if (!enemy) return "💾";

    if (enemy->type == MonsterType::NEMESIS) return "⚡";
    if (enemy->type == MonsterType::BOSS) return "👑";
    if (enemy->type == MonsterType::REVIEW) return "📅";
    if (enemy->name.find("Shadow:") != std::string::npos) return "👤";
    if (enemy->name.find("Safety Manager") != std::string::npos) return "🛡️";
    if (enemy->name.find("Safety Crony") != std::string::npos) return "⚠️";
    if (enemy->name.find("Conference") != std::string::npos) return "🎤";
    if (enemy->name.find("Paper") != std::string::npos) return "📄";
    if (enemy->name.find("Minion:") != std::string::npos) return "👾";

    return "💾"; // Default mob
}

QString MainWindow::getEnemyColor(Monster* enemy) const {
    if (!enemy) return "#FFFFFF";

    // Get color from effects manager
    std::string effectColor = enemy->getEffectsManager().getDisplayColor();
    return QString::fromStdString(effectColor);
}

void MainWindow::updateEnemyFrame(size_t frameIndex, Monster* enemy) {
    if (frameIndex >= enemyFrames_.size() || !enemy) return;

    EnemyFrame& frame = enemyFrames_[frameIndex];

    // Check if enemy just died
    if (!enemy->isAlive() && !frame.isDead) {
        frame.isDead = true;

        // Greyscale dead enemy
        frame.container->setStyleSheet(
            "QWidget { "
            "border: 2px solid #333; "
            "border-radius: 5px; "
            "background-color: #1a1a1a; "
            "padding: 5px; "
            "}"
        );
        frame.nameLabel->setStyleSheet("font-weight: bold; font-size: 11px; color: #666; border: none;");
        frame.symbolLabel->setStyleSheet("font-size: 20px; border: none; color: #666;");

        // Fade out after 2 seconds
        frame.fadeTimer = new QTimer(this);
        frame.fadeTimer->setSingleShot(true);
        connect(frame.fadeTimer, &QTimer::timeout, [this, frameIndex]() {
            clearEnemyFrame(frameIndex);
        });
        frame.fadeTimer->start(2000);

        return;  // Don't update other fields for dead enemy
    }

    // Don't update if already dead
    if (frame.isDead) return;

    frame.container->show();
    frame.container->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Update symbol
    frame.symbolLabel->setText(getEnemySymbol(enemy));

    // Update name with effect count and color
    int effectCount = enemy->getEffectsManager().getEffectCount();
    QString nameText = QString::fromStdString(enemy->name);
    if (effectCount > 0) {
        nameText += QString(" [%1]").arg(effectCount);
    }

    QString color = getEnemyColor(enemy);
    frame.nameLabel->setText(nameText);
    frame.nameLabel->setStyleSheet(
        QString("font-weight: bold; font-size: 12px; color: %1; border: none;").arg(color)
    );

    // Update health
    frame.healthLabel->setText(
        QString("HP: %1/%2").arg(enemy->health).arg(enemy->maxHealth)
    );

    frame.healthBar->setMaximum(enemy->maxHealth);

    // Animate health bar
    if (frame.targetHealth != enemy->health) {
        frame.targetHealth = enemy->health;

        if (frame.healthAnimation->state() == QAbstractAnimation::Running) {
            frame.healthAnimation->stop();
        }

        frame.healthAnimation->setStartValue(frame.healthBar->value());
        frame.healthAnimation->setEndValue(enemy->health);
        frame.healthAnimation->start();
    }

    // Color health bar based on percentage
    int healthPercent = (enemy->health * 100) / std::max(1, enemy->maxHealth);
    QString barColor;
    if (healthPercent > 60) barColor = "#4CAF50";
    else if (healthPercent > 30) barColor = "#FF9800";
    else barColor = "#F44336";

    frame.healthBar->setStyleSheet(
        QString("QProgressBar::chunk { background-color: %1; }").arg(barColor)
    );

    // Update stats with modifiers
    int baseAttack = enemy->attack;
    int modifiedAttack = enemy->getModifiedHustle();
    if (modifiedAttack != baseAttack) {
        frame.attackLabel->setText(
            QString("⚔️ %1 (%2%3)").arg(modifiedAttack)
            .arg(modifiedAttack > baseAttack ? "+" : "")
            .arg(modifiedAttack - baseAttack)
        );
    } else {
        frame.attackLabel->setText(QString("⚔️ %1").arg(modifiedAttack));
    }

    int baseDefense = enemy->defense;
    int modifiedDefense = enemy->getModifiedNDAs();
    if (modifiedDefense != baseDefense) {
        frame.defenseLabel->setText(
            QString("🛡️ %1 (%2%3)").arg(modifiedDefense)
            .arg(modifiedDefense > baseDefense ? "+" : "")
            .arg(modifiedDefense - baseDefense)
        );
    } else {
        frame.defenseLabel->setText(QString("🛡️ %1").arg(modifiedDefense));
    }
}

void MainWindow::clearEnemyFrame(size_t frameIndex) {
    if (frameIndex >= enemyFrames_.size()) return;

    EnemyFrame& frame = enemyFrames_[frameIndex];

    // Clean up timer if exists
    if (frame.fadeTimer) {
        frame.fadeTimer->deleteLater();
        frame.fadeTimer = nullptr;
    }

    frame.container->hide();
    frame.container->setStyleSheet(
        "QWidget { "
        "border: 2px solid #555; "
        "border-radius: 5px; "
        "background-color: #2a2a2a; "
        "padding: 5px; "
        "}"
    );

    frame.symbolLabel->setText("👾");
    frame.symbolLabel->setStyleSheet("font-size: 20px; border: none;");
    frame.nameLabel->setText("Empty");
    frame.nameLabel->setStyleSheet("font-weight: bold; font-size: 12px; color: gray; border: none;");
    frame.healthLabel->setText("HP: ---/---");
    frame.healthBar->setValue(0);
    frame.attackLabel->setText("⚔️ ---");
    frame.defenseLabel->setText("🛡️ ---");
    frame.targetHealth = -1;
    frame.isDead = false;
}

void MainWindow::updateBattleDisplay() {
    if (!currentEncounter || !currentEncounter->hasAliveEnemies()) {
        for (size_t i = 0; i < enemyFrames_.size(); ++i) {
            clearEnemyFrame(i);
        }
        waitingForBattleLabel->show();
        roundNumberLabel->setText("Round: 0");
        return;
    }

    waitingForBattleLabel->hide();

    auto allEnemies = currentEncounter->getAllEnemies();
    for (size_t i = 0; i < enemyFrames_.size(); ++i) {
        if (i < allEnemies.size()) {
            updateEnemyFrame(i, allEnemies[i]);
        } else {
            clearEnemyFrame(i);
        }
    }

    roundNumberLabel->setText(QString("Round: %1").arg(roundNumber));
}
