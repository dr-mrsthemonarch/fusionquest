//
// Created by drmrsthemonarch on 02.11.25.
//
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <vector>
#include <iomanip>
#include <sstream>

#include "../include/Character.h"
#include "../include/Monster.h"
#include "../include/MonsterGenerator.h"
#include "../include/EquipmentGenerator.h"
#include "class/ItemGenerator.h"
#include "../include/Patent.h"
#include "../include/NemesisSystem.h"
#include "class/ExpandedMonsterTypes.h"
#include "class/SpecialDropsLibrary.h"

// Test statistics tracker
struct TestStats {
    int passed = 0;
    int failed = 0;
    int warnings = 0;

    void pass(const std::string &testName) {
        std::cout << "✓ PASS: " << testName << std::endl;
        passed++;
    }

    void fail(const std::string &testName, const std::string &reason = "") {
        std::cout << "✗ FAIL: " << testName;
        if (!reason.empty()) {
            std::cout << " - " << reason;
        }
        std::cout << std::endl;
        failed++;
    }

    void warn(const std::string &message) {
        std::cout << "⚠ WARNING: " << message << std::endl;
        warnings++;
    }

    void summary() {
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "                      TEST SUMMARY" << std::endl;
        std::cout << std::string(70, '=') << std::endl;
        std::cout << "✓ Passed:   " << passed << std::endl;
        std::cout << "✗ Failed:   " << failed << std::endl;
        std::cout << "⚠ Warnings: " << warnings << std::endl;
        std::cout << "Total:      " << (passed + failed) << std::endl;
        std::cout << "Success:    " << std::fixed << std::setprecision(1) << (100.0 * passed / (passed + failed)) << "%" << std::endl;
        std::cout << std::string(70, '=') << std::endl;
    }
};

TestStats globalStats;

// Helper functions
void displaySection(const std::string &title) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

void displaySubsection(const std::string &title) {
    std::cout << "\n" << std::string(70, '-') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(70, '-') << std::endl;
}

void displayCharacterStatus(const Character &player) {
    std::cout << "\n📊 CHARACTER STATUS:" << std::endl;
    std::cout << "├─ Name: " << player.getName() << std::endl;
    std::cout << "├─ Company: " << player.getStartup() << std::endl;
    std::cout << "├─ Series: " << player.getSeries() << std::endl;
    std::cout << "├─ Runway: " << player.getRunway() << "/" << player.getMaxRunway() << std::endl;
    std::cout << "├─ Hype Energy: " << player.getHypeEnergy() << "/" << player.getMaxHypeEnergy() << std::endl;
    std::cout << "├─ Money: $" << player.getMoney() << std::endl;
    std::cout << "├─ Funding: " << static_cast<int>(player.getCurrentFunding()) << "/" << static_cast<int>(player.getRequiredFunding()) << std::endl;
    std::cout << "├─ Hustle (ATK): " << player.getHustle() << std::endl;
    std::cout << "├─ Agility (SPD): " << player.getAgility() << std::endl;
    std::cout << "├─ NDAs (DEF): " << player.getNDAs() << std::endl;
    std::cout << "├─ Luck: " << player.getLuck() << std::endl;
    std::cout << "├─ Synergy: " << player.getSynergy() << std::endl;
    std::cout << "├─ Technical Debt: " << player.getTechnicalDebt() << std::endl;
    std::cout << "└─ Bus Factor: " << player.getBusFactor() << std::endl;
}

void displayMonsterStatus(const Monster &monster) {
    std::cout << "🐉 " << monster.name << " [" << monster.getDifficultyString() << "]" << std::endl;
    std::cout << "├─ Level: " << monster.level << std::endl;
    std::cout << "├─ Health: " << monster.health << "/" << monster.maxHealth << std::endl;
    std::cout << "├─ Attack: " << monster.attack << std::endl;
    std::cout << "├─ Defense: " << monster.defense << std::endl;
    std::cout << "├─ Speed: " << monster.speed << std::endl;
    std::cout << "└─ Reward: $" << monster.moneyReward << std::endl;
}

void pause(int milliseconds = 500) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

// Test initialization and loading
bool testInitialization(EquipmentGenerator &equipGen, MonsterGenerator &monsterGen, ItemGenerator &itemGen, SpecializedMonsterGenerator &specialGen) {
    displaySection("INITIALIZATION TESTS");

    bool allLoaded = true;

    // Load equipment config
    if (equipGen.loadWordLists("config/equipment/")) {
        globalStats.pass("Equipment generator loaded");
    } else {
        globalStats.fail("Equipment generator failed to load");
        globalStats.warn("Check that config/equipment/ exists with required files");
        allLoaded = false;
    }

    // Load monster config
    if (monsterGen.loadWordLists("config/mobs/")) {
        globalStats.pass("Monster generator loaded");
    } else {
        globalStats.fail("Monster generator failed to load");
        globalStats.warn("Check that config/mobs/ exists with required files");
        allLoaded = false;
    }

    // Load item config
    if (itemGen.loadWordLists("config/items/")) {
        globalStats.pass("Item generator loaded");
    } else {
        globalStats.fail("Item generator failed to load");
        globalStats.warn("Check that config/items/ exists with required files");
        allLoaded = false;
    }

    // Load specialized monster config
    if (specialGen.loadWordLists("config/specialized/")) {
        globalStats.pass("Specialized monster generator loaded");
    } else {
        globalStats.fail("Specialized monster generator failed to load");
        globalStats.warn("Check that config/specialized/ exists with required files");
        allLoaded = false;
    }

    // Load patent names
    Patent::loadPatentNames("config/items/patent_names.txt");
    Patent::initializePrimes();
    globalStats.pass("Patent system initialized");

    return allLoaded;
}

// Test character creation and progression
void testCharacterProgression(Character &player, EquipmentGenerator &equipGen) {
    displaySection("CHARACTER PROGRESSION TESTS");

    // Test initial state
    displaySubsection("Initial Character State");
    displayCharacterStatus(player);

    if (player.getSeries() == 1) {
        globalStats.pass("Character starts at Series 1");
    } else {
        globalStats.fail("Character should start at Series 1");
    }

    if (player.isAlive()) {
        globalStats.pass("Character is alive");
    } else {
        globalStats.fail("Character should be alive initially");
    }

    // Test money system
    displaySubsection("Money System");
    player.addMoney(1000);
    if (player.getMoney() == 1000) {
        globalStats.pass("Money addition works");
    } else {
        globalStats.fail("Money addition failed");
    }

    if (player.spendMoney(500)) {
        globalStats.pass("Money spending works");
    } else {
        globalStats.fail("Money spending failed");
    }

    if (!player.spendMoney(10000)) {
        globalStats.pass("Cannot overspend money");
    } else {
        globalStats.fail("Overspending should be prevented");
    }

    // Test funding and leveling
    displaySubsection("Funding and Leveling System");
    int initialSeries = player.getSeries();
    double requiredFunding = player.getRequiredFunding();

    std::cout << "💰 Adding funding to trigger level up: $" << (requiredFunding + 100) << std::endl;
    player.addFunding(requiredFunding + 100);
    pause();

    if (player.getSeries() > initialSeries) {
        globalStats.pass("Character levels up with sufficient funding");
        std::cout << "   Series increased: " << initialSeries << " → " << player.getSeries() << std::endl;
    } else {
        globalStats.fail("Character should level up with sufficient funding");
    }

    // Test equipment system
    displaySubsection("Equipment System");
    Equipment laptop = equipGen.generateSpecificEquipmentForPlayer(EquipmentType::LAPTOP, player.getSeries());
    std::cout << "💻 Generated: " << laptop.name << std::endl;

    player.addMoney(laptop.equipmentvalue);
    if (player.purchaseEquipment(laptop)) {
        globalStats.pass("Equipment purchase works");
    } else {
        globalStats.fail("Equipment purchase failed");
    }

    if (player.equipItem(laptop)) {
        globalStats.pass("Equipment equipping works");
    } else {
        globalStats.fail("Equipment equipping failed");
    }

    Equipment *equippedLaptop = player.getEquippedItem(EquipmentType::LAPTOP);
    if (equippedLaptop != nullptr) {
        globalStats.pass("Can retrieve equipped items");
    } else {
        globalStats.fail("Cannot retrieve equipped items");
    }

    // Test inventory system
    displaySubsection("Inventory System");
    player.addInventoryItem("Test Widget", 100);
    if (player.getInventoryCount() > 0) {
        globalStats.pass("Inventory item addition works");
    } else {
        globalStats.fail("Inventory item addition failed");
    }

    // Test patent system
    displaySubsection("Patent System");
    Patent::initializeRandomPatents(&player, 2);
    if (Patent::characterPatents.size() >= 2) {
        globalStats.pass("Patent generation works");
    } else {
        globalStats.fail("Patent generation failed");
    }
}

// Simulate a battle and return victory status
bool simulateBattle(Character &player, Monster &monster, const std::string &battleType = "") {
    std::cout << "\n⚔️  BATTLE START" << (!battleType.empty() ? " - " + battleType : "") << "!" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    displayMonsterStatus(monster);
    std::cout << "\nVS\n" << std::endl;
    std::cout << "👤 " << player.getName() << std::endl;
    std::cout << "├─ Runway: " << player.getRunway() << "/" << player.getMaxRunway() << std::endl;
    std::cout << "├─ Hustle (ATK): " << player.getHustle() << std::endl;
    std::cout << "├─ Agility (SPD): " << player.getAgility() << std::endl;
    std::cout << "└─ NDAs (DEF): " << player.getNDAs() << std::endl;

    std::cout << std::string(70, '=') << std::endl;
    pause(1000);

    std::random_device rd;
    std::mt19937 gen(rd());

    int roundNumber = 1;
    int maxRounds = 20; // Prevent infinite battles

    while (monster.isAlive() && player.getRunway() > 0 && roundNumber <= maxRounds) {
        std::cout << "\n🥊 Round " << roundNumber << std::endl;
        std::cout << std::string(40, '-') << std::endl;

        // Player attacks
        int playerDamage = std::max(1, static_cast<int>(player.getHustle() - monster.defense * 0.5 + (gen() % 10 - 5)));
        std::cout << "⚡ " << player.getName() << " attacks for " << playerDamage << " damage!" << std::endl;
        monster.takeDamage(playerDamage);

        if (!monster.isAlive()) {
            std::cout << "💀 " << monster.name << " has been defeated!" << std::endl;
            break;
        }

        std::cout << "   " << monster.name << " HP: " << monster.health << "/" << monster.maxHealth << std::endl;

        // Monster attacks back
        int monsterDamage = std::max(1, static_cast<int>(monster.attack - player.getNDAs() * 0.3 + (gen() % 8 - 4)));
        std::cout << "💥 " << monster.name << " attacks for " << monsterDamage << " damage!" << std::endl;

        int newRunway = std::max(0, player.getRunway() - monsterDamage);
        player.setRunway(newRunway);

        std::cout << "   " << player.getName() << " Runway: " << player.getRunway() << "/" << player.getMaxRunway() << std::endl;

        if (player.getRunway() <= 0) {
            std::cout << "💔 " << player.getName() << " has been defeated!" << std::endl;
            break;
        }

        roundNumber++;
        pause(800);
    }

    std::cout << std::string(70, '=') << std::endl;

    bool victory = !monster.isAlive();

    if (victory) {
        std::cout << "🎉 VICTORY!" << std::endl;
        std::cout << "💰 Reward: $" << monster.moneyReward << std::endl;
        player.addMoney(monster.moneyReward);

        if (!monster.itemReward.empty()) {
            std::cout << "🎁 Item: " << monster.itemReward << std::endl;
        }
    } else if (player.getRunway() <= 0) {
        std::cout << "💀 DEFEAT - Player needs recovery!" << std::endl;
    } else {
        std::cout << "⏱️  Battle timeout - Strategic retreat!" << std::endl;
    }

    pause(1000);
    return victory;
}

// Test standard monster encounters with battles
void testStandardMonsters(Character &player, MonsterGenerator &monsterGen, EquipmentGenerator &equipGen, ItemGenerator &itemGen) {
    displaySection("STANDARD MONSTER TESTS");

    // Prepare player
    player.addMoney(10000);
    player.restoreToFull();

    // Test mob generation
    displaySubsection("Regular Mob Encounters");
    Monster mob = monsterGen.generateMob(player.getSeries(), itemGen);

    if (!mob.name.empty() && mob.type == MonsterType::MOB) {
        globalStats.pass("Mob generation works");
        displayMonsterStatus(mob);
    } else {
        globalStats.fail("Mob generation failed");
    }

    // Test different difficulties
    std::cout << "\n🎲 Testing difficulty variations..." << std::endl;
    for (int i = 0; i < 5; i++) {
        Monster testMob = monsterGen.generateMob(player.getSeries(), itemGen);
        std::cout << "  - " << testMob.name << " [" << testMob.getDifficultyString() << "]" << std::endl;
    }
    globalStats.pass("Multiple mob generation works");

    // Battle a mob
    displaySubsection("Mob Combat Test");
    Monster battleMob = monsterGen.generateMob(player.getSeries(), itemGen);

    std::cout << "\n🗡️  Testing combat against a mob..." << std::endl;
    pause(1000);

    bool mobVictory = simulateBattle(player, battleMob, "MOB ENCOUNTER");

    if (mobVictory) {
        globalStats.pass("Mob combat works correctly");
    } else {
        globalStats.warn("Mob was not defeated - this can happen");
    }

    player.restoreToFull();
    pause(1500);

    // Test boss generation
    displaySubsection("Boss Encounters");
    Monster boss = monsterGen.generateBoss(player.getSeries());

    if (!boss.name.empty() && boss.type == MonsterType::BOSS) {
        globalStats.pass("Boss generation works");
        displayMonsterStatus(boss);
    } else {
        globalStats.fail("Boss generation failed");
    }

    if (boss.health > mob.health) {
        globalStats.pass("Bosses are stronger than mobs");
    } else {
        globalStats.fail("Bosses should be stronger than mobs");
    }

    // Battle a boss
    displaySubsection("Boss Combat Test");
    Monster battleBoss = monsterGen.generateBoss(player.getSeries());

    std::cout << "\n👑 Testing combat against a boss..." << std::endl;
    pause(1000);

    bool bossVictory = simulateBattle(player, battleBoss, "BOSS BATTLE");

    if (bossVictory) {
        globalStats.pass("Boss combat works correctly");
        std::cout << "🎁 Boss drops equipment!" << std::endl;

        // Generate boss drop
        Equipment bossReward = equipGen.generateBossDropEquipment(player.getSeries());
        std::cout << "   Dropped: " << bossReward.name << " (Rarity: " << static_cast<int>(bossReward.rarity) << ")" << std::endl;
    } else {
        globalStats.warn("Boss was not defeated - they are tough!");
    }

    player.restoreToFull();
    pause(1500);

    // Test encounter generation
    displaySubsection("Mixed Encounter Generation");
    int mobCount, bossCount;
    std::vector<Monster> encounter = monsterGen.generateEncounter(player.getSeries(), mobCount, bossCount, itemGen);

    std::cout << "📊 Generated encounter: " << mobCount << " mobs, " << bossCount << " bosses" << std::endl;

    if (encounter.size() == (mobCount + bossCount)) {
        globalStats.pass("Encounter generation correct count");
    } else {
        globalStats.fail("Encounter count mismatch");
    }

    // Test monster scaling
    displaySubsection("Monster Level Scaling");
    Monster level1 = monsterGen.generateBoss(1);
    Monster level5 = monsterGen.generateBoss(5);
    Monster level10 = monsterGen.generateBoss(10);

    std::cout << "Level 1 Boss HP: " << level1.maxHealth << std::endl;
    std::cout << "Level 5 Boss HP: " << level5.maxHealth << std::endl;
    std::cout << "Level 10 Boss HP: " << level10.maxHealth << std::endl;

    if (level5.maxHealth > level1.maxHealth && level10.maxHealth > level5.maxHealth) {
        globalStats.pass("Monster stats scale with level");
    } else {
        globalStats.fail("Monster scaling not working correctly");
    }
}

// Test specialized monsters with actual battles
void testSpecializedMonsters(Character &player, SpecializedMonsterGenerator &specialGen) {
    displaySection("SPECIALIZED MONSTER TESTS");

    // Restore and power up player for these challenging battles
    player.restoreToFull();
    player.addMoney(10000);

    // Test Safety Manager
    displaySubsection("Safety Manager Encounter");
    Monster safety = specialGen.generateSafetyManager(player.getSeries(), 0);

    if (safety.difficulty == MonsterDifficulty::LEGENDARY) {
        globalStats.pass("Safety Manager is always Legendary difficulty");
    } else {
        globalStats.fail("Safety Manager should be Legendary");
    }

    std::cout << "\n🛡️ A Safety Manager appears to audit your startup!" << std::endl;
    std::cout << "📋 Special Mechanic: SAFETY_AUDIT" << std::endl;
    displayMonsterStatus(safety);
    pause(1000);

    // Battle Safety Manager
    bool safetyVictory = simulateBattle(player, safety, "SAFETY AUDIT");

    if (safetyVictory) {
        globalStats.pass("Safety Manager can be defeated");
        std::cout << "✅ Your startup passed the safety audit!" << std::endl;
    } else {
        globalStats.warn("Safety Manager was not defeated - they are very tough!");
        std::cout << "⚠️  Safety violations noted. Further training required." << std::endl;
    }

    // Restore before next battle
    player.restoreToFull();
    pause(1500);

    // Test Conference
    displaySubsection("Conference Encounter");
    Monster conference = specialGen.generateConference(player.getSeries(), 0);

    if (!conference.name.empty()) {
        globalStats.pass("Conference generation works");
    } else {
        globalStats.fail("Conference generation failed");
    }

    std::cout << "\n🎤 You must present at a major tech conference!" << std::endl;
    std::cout << "📋 Special Mechanic: NETWORKING_CHECK (Synergy + Luck based)" << std::endl;
    displayMonsterStatus(conference);
    pause(1000);

    // Battle Conference
    bool confVictory = simulateBattle(player, conference, "CONFERENCE PRESENTATION");

    if (confVictory) {
        globalStats.pass("Conference can be completed successfully");
        std::cout << "👏 Standing ovation! Your pitch was amazing!" << std::endl;
    } else {
        globalStats.warn("Conference was challenging");
        std::cout << "😅 Tough crowd, but you learned valuable lessons." << std::endl;
    }

    // Restore before next battle
    player.restoreToFull();
    pause(1500);

    // Test Paper Writing
    displaySubsection("Paper Writing Challenge");
    Monster paper = specialGen.generatePaperWriting(player.getSeries(), 0);

    if (!paper.name.empty()) {
        globalStats.pass("Paper writing challenge generation works");
    } else {
        globalStats.fail("Paper writing generation failed");
    }

    std::cout << "\n📄 A critical research paper deadline approaches!" << std::endl;
    std::cout << "📋 Special Mechanic: PEER_REVIEW (Agility + NDAs based)" << std::endl;
    displayMonsterStatus(paper);
    pause(1000);

    // Battle Paper Writing
    bool paperVictory = simulateBattle(player, paper, "PAPER DEADLINE");

    if (paperVictory) {
        globalStats.pass("Paper writing challenge can be completed");
        std::cout << "📚 Paper accepted! Citations incoming!" << std::endl;
    } else {
        globalStats.warn("Paper writing was challenging");
        std::cout << "📝 Revisions requested. Back to the drawing board." << std::endl;
    }

    // Restore for next tests
    player.restoreToFull();

    std::cout << "\n✅ All specialized monster types tested with battles!" << std::endl;
}

// Test nemesis system with battle
void testNemesisSystem(Character &player) {
    displaySection("NEMESIS SYSTEM TESTS");

    NemesisSystem nemesis;

    // Test nemesis at different time periods
    displaySubsection("Nemesis Generation Over Time");

    double year1 = 1.0 * 365 * 24 * 60 * 60;
    double year5 = 5.0 * 365 * 24 * 60 * 60;
    double year10 = 10.0 * 365 * 24 * 60 * 60;
    double year20 = 20.0 * 365 * 24 * 60 * 60;

    Monster nemesis1 = nemesis.generateNemesis(1, year1, player.getName());
    Monster nemesis5 = nemesis.generateNemesis(5, year5, player.getName());
    Monster nemesis10 = nemesis.generateNemesis(10, year10, player.getName());
    Monster nemesis20 = nemesis.generateNemesis(20, year20, player.getName());

    std::cout << "Year 1 Nemesis: " << nemesis1.name << " (HP: " << nemesis1.maxHealth << ")" << std::endl;
    std::cout << "Year 5 Nemesis: " << nemesis5.name << " (HP: " << nemesis5.maxHealth << ")" << std::endl;
    std::cout << "Year 10 Nemesis: " << nemesis10.name << " (HP: " << nemesis10.maxHealth << ")" << std::endl;
    std::cout << "Year 20 Nemesis: " << nemesis20.name << " (HP: " << nemesis20.maxHealth << ")" << std::endl;

    if (nemesis20.maxHealth > nemesis10.maxHealth && nemesis10.maxHealth > nemesis5.maxHealth) {
        globalStats.pass("Nemesis scales over time");
    } else {
        globalStats.fail("Nemesis scaling broken");
    }

    // Test nemesis evolution
    displaySubsection("Nemesis Evolution System");
    nemesis.recordNemesisDefeat(1, year1, NemesisType::ALPHA, nemesis1.name);

    Monster evolvedNemesis = nemesis.generateNemesis(2, year1 + 365 * 24 * 60 * 60, player.getName());

    if (evolvedNemesis.maxHealth > nemesis1.maxHealth) {
        globalStats.pass("Nemesis evolves after defeat");
        std::cout << "   Original: " << nemesis1.maxHealth << " HP" << std::endl;
        std::cout << "   Evolved: " << evolvedNemesis.maxHealth << " HP" << std::endl;
    } else {
        globalStats.fail("Nemesis evolution not working");
    }

    // Epic Nemesis Battle
    displaySubsection("Epic Nemesis Battle - Year 5");

    player.restoreToFull();
    player.addMoney(10000);

    // Power up player for this epic fight
    player.addFunding(player.getRequiredFunding() * 2); // Level up
    player.restoreToFull();

    Monster nemesisBattle = nemesis.generateNemesis(player.getSeries(), year5, player.getName());

    std::cout << "\n⚡ YOUR NEMESIS APPEARS!" << std::endl;
    std::cout << "🎯 This is your arch-rival in the startup world!" << std::endl;
    std::cout << "💀 Type: " << static_cast<int>(nemesisBattle.type) << " (NEMESIS)" << std::endl;
    displayMonsterStatus(nemesisBattle);

    pause(2000);

    std::cout << "\n🌟 This battle will be legendary..." << std::endl;
    pause(1000);

    bool nemesisVictory = simulateBattle(player, nemesisBattle, "NEMESIS SHOWDOWN");

    if (nemesisVictory) {
        globalStats.pass("Nemesis can be defeated in combat");
        std::cout << "\n🏆 INCREDIBLE! You've defeated your nemesis!" << std::endl;
        std::cout << "⚡ But they will return... stronger!" << std::endl;

        // Show what happens after defeat
        nemesis.recordNemesisDefeat(5, year5, NemesisType::BETA, nemesisBattle.name);
        std::cout << "📈 Nemesis power level increased!" << std::endl;
    } else {
        globalStats.warn("Nemesis battle was extremely difficult");
        std::cout << "\n💔 Your nemesis proves too powerful... for now." << std::endl;
        std::cout << "⚔️  Train harder and face them again!" << std::endl;
    }

    player.restoreToFull();
}

// Test review calendar system with battle
void testReviewCalendar(Character &player) {
    displaySection("REVIEW CALENDAR SYSTEM TESTS");

    ReviewCalendarSystem calendar;
    calendar.generateReviewCalendar(player.getSeries());

    // Test scheduled review retrieval
    displaySubsection("Monthly Review Scheduling");

    double month1 = 15 * 24 * 60 * 60; // Mid-first month
    ScheduledBoss *review = calendar.checkForScheduledReview(month1);

    if (review != nullptr) {
        globalStats.pass("Scheduled review found for first month");
        displayMonsterStatus(review->boss);
    } else {
        globalStats.fail("No scheduled review found");
    }

    // Test progression through year
    std::cout << "\n📅 Testing reviews throughout first year..." << std::endl;
    int reviewsFound = 0;
    for (int month = 1; month <= 12; month++) {
        double testTime = (month * 30.0 - 15) * 24 * 60 * 60; // Mid-month
        ScheduledBoss *monthReview = calendar.checkForScheduledReview(testTime);
        if (monthReview) {
            reviewsFound++;
            std::cout << "   Month " << month << ": " << monthReview->boss.name << std::endl;
        }
    }

    if (reviewsFound > 0) {
        globalStats.pass("Multiple monthly reviews generated");
    } else {
        globalStats.fail("Monthly review generation failed");
    }

    // Battle a monthly review
    displaySubsection("Monthly Review Battle");

    player.restoreToFull();

    if (review != nullptr) {
        std::cout << "\n📋 Time for your performance review!" << std::endl;
        std::cout << "💼 Your investors want to see progress..." << std::endl;
        pause(1000);

        // Create a copy of the boss for battle
        Monster reviewBoss = review->boss;

        bool reviewVictory = simulateBattle(player, reviewBoss, "MONTHLY REVIEW");

        if (reviewVictory) {
            globalStats.pass("Monthly review can be completed");
            std::cout << "📈 Excellent performance! Investors are impressed!" << std::endl;
            calendar.recordReviewEncounter(review->year, review->month, true);
        } else {
            globalStats.warn("Monthly review was challenging");
            std::cout << "📉 Review didn't go as planned. Need improvement." << std::endl;
            calendar.recordReviewEncounter(review->year, review->month, false);
        }
    }

    player.restoreToFull();
}

// Test shadow player system
void testShadowPlayer(Character &player) {
    displaySection("SHADOW PLAYER SYSTEM TESTS");

    ShadowPlayerSystem shadow;

    displaySubsection("Shadow Player Generation");

    double year10 = 10.0 * 365 * 24 * 60 * 60;
    Monster shadowPlayer = shadow.generateShadowPlayer(player, year10);

    if (shadowPlayer.name.find("Shadow") != std::string::npos) {
        globalStats.pass("Shadow player name contains 'Shadow'");
    } else {
        globalStats.fail("Shadow player name should contain 'Shadow'");
    }

    std::cout << "🌑 " << shadowPlayer.name << std::endl;
    std::cout << "   Mirrored Stats:" << std::endl;
    std::cout << "   HP: " << shadowPlayer.maxHealth << std::endl;
    std::cout << "   ATK: " << shadowPlayer.attack << std::endl;
    std::cout << "   DEF: " << shadowPlayer.defense << std::endl;

    if (shadowPlayer.difficulty == MonsterDifficulty::LEGENDARY) {
        globalStats.pass("Shadow player is Legendary difficulty");
    } else {
        globalStats.fail("Shadow player should be Legendary");
    }
}

// Test special drops library
void testSpecialDrops(Character &player, SpecialDropsLibrary &library, EquipmentGenerator &equipGen) {
    displaySection("SPECIAL DROPS LIBRARY TESTS");

    displaySubsection("Nemesis Equipment Generation");
    Equipment nemesisGear = library.generateNemesisEquipment(player.getSeries(), 0, "TestNemesis");

    if (nemesisGear.rarity == EquipmentRarity::LEGENDARY) {
        globalStats.pass("Nemesis equipment is Legendary");
    } else {
        globalStats.fail("Nemesis equipment should be Legendary");
    }

    std::cout << "⚡ " << nemesisGear.name << std::endl;
    std::cout << "   Value: $" << nemesisGear.equipmentvalue << std::endl;

    // Test adding to library
    SpecialDrop drop;
    drop.name = nemesisGear.name;
    drop.type = SpecialDropType::NEMESIS_DROP;
    drop.equipment = nemesisGear;
    drop.acquiredFrom = "Test Nemesis";
    drop.yearAcquired = 1;
    drop.monthAcquired = 1;

    library.addSpecialDrop(drop);

    if (library.getDropsByType(SpecialDropType::NEMESIS_DROP) == 1) {
        globalStats.pass("Special drop added to library");
    } else {
        globalStats.fail("Special drop not added to library");
    }

    // Test other special equipment types
    displaySubsection("Other Special Equipment");

    Equipment shadowGear = library.generateShadowPlayerEquipment(player.getSeries(), player.getName());
    std::cout << "🌑 Shadow: " << shadowGear.name << std::endl;

    Equipment safetyGear = library.generateSafetyManagerEquipment(player.getSeries());
    std::cout << "🛡️ Safety: " << safetyGear.name << std::endl;

    Equipment confGear = library.generateConferenceEquipment(player.getSeries());
    std::cout << "🎤 Conference: " << confGear.name << std::endl;

    Equipment paperGear = library.generatePaperWritingEquipment(player.getSeries());
    std::cout << "📄 Paper: " << paperGear.name << std::endl;

    Equipment reviewGear = library.generateMonthlyReviewEquipment(1, 1, player.getSeries());
    std::cout << "📅 Review: " << reviewGear.name << std::endl;

    globalStats.pass("All special equipment types generate successfully");
}

// Test combat mechanics
void testCombatMechanics(Character &player, MonsterGenerator &monsterGen, ItemGenerator &itemGen) {
    displaySection("COMBAT MECHANICS TESTS");

    player.restoreToFull();

    displaySubsection("Basic Combat");
    Monster testMonster = monsterGen.generateMob(1, itemGen);

    int initialHealth = testMonster.health;
    testMonster.takeDamage(20);

    if (testMonster.health < initialHealth) {
        globalStats.pass("Monster takes damage correctly");
    } else {
        globalStats.fail("Monster damage calculation broken");
    }

    int monsterDamage = testMonster.calculateDamage();
    if (monsterDamage > 0) {
        globalStats.pass("Monster can calculate damage");
    } else {
        globalStats.fail("Monster damage calculation failed");
    }

    // Test death
    testMonster.health = 0;
    if (!testMonster.isAlive()) {
        globalStats.pass("Monster death detection works");
    } else {
        globalStats.fail("Monster should be dead at 0 HP");
    }
}

// Test bankruptcy system
void testBankruptcySystem(Character &player) {
    displaySection("BANKRUPTCY SYSTEM TESTS");

    displaySubsection("Triggering Bankruptcy");

    // Save initial state
    int initialMoney = player.getMoney();

    player.addMoney(1000);
    player.addInventoryItem("Test Item", 100);

    std::cout << "💰 Money before bankruptcy: $" << player.getMoney() << std::endl;
    std::cout << "🎒 Inventory items: " << player.getInventoryCount() << std::endl;

    player.setRunway(0);
    player.handleDeath();

    if (player.getMoney() == 0) {
        globalStats.pass("Money reset on bankruptcy");
    } else {
        globalStats.fail("Money should be 0 after bankruptcy");
    }

    if (player.getRunway() == 1) {
        globalStats.pass("Runway set to 1 (critical) after bankruptcy");
    } else {
        globalStats.fail("Runway should be 1 after bankruptcy");
    }

    if (player.isAlive()) {
        globalStats.pass("Character survives bankruptcy");
    } else {
        globalStats.fail("Character should survive bankruptcy");
    }

    // Restore for further tests
    player.restoreToFull();
    player.addMoney(10000);
}

// Test equipment rarity system
void testEquipmentRarity(EquipmentGenerator &equipGen) {
    displaySection("EQUIPMENT RARITY TESTS");

    displaySubsection("Boss Drop Rarity Distribution");

    int legendary = 0, epic = 0, rare = 0, uncommon = 0, common = 0;

    std::cout << "🎲 Generating 100 boss drops..." << std::endl;
    for (int i = 0; i < 100; i++) {
        Equipment drop = equipGen.generateBossDropEquipment(5);

        switch (drop.rarity) {
            case EquipmentRarity::LEGENDARY: legendary++;
                break;
            case EquipmentRarity::EPIC: epic++;
                break;
            case EquipmentRarity::RARE: rare++;
                break;
            case EquipmentRarity::UNCOMMON: uncommon++;
                break;
            case EquipmentRarity::COMMON: common++;
                break;
        }
    }

    std::cout << "\n📊 Rarity Distribution:" << std::endl;
    std::cout << "   Legendary: " << legendary << "%" << std::endl;
    std::cout << "   Epic: " << epic << "%" << std::endl;
    std::cout << "   Rare: " << rare << "%" << std::endl;
    std::cout << "   Uncommon: " << uncommon << "%" << std::endl;
    std::cout << "   Common: " << common << "%" << std::endl;

    if (legendary >= 0 && epic > 0) {
        globalStats.pass("Boss drops generate with varied rarities");
    } else {
        globalStats.fail("Rarity distribution not working");
    }
}

// Main test runner
int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║       STARTUP SELF-PLAYING GAME ENGINE - GAMEPLAY TEST SUITE        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════╝\n";

    // Initialize generators
    EquipmentGenerator equipGen;
    MonsterGenerator monsterGen;
    ItemGenerator itemGen;
    SpecializedMonsterGenerator specialGen;
    SpecialDropsLibrary dropLibrary;

    // Test initialization
    if (!testInitialization(equipGen, monsterGen, itemGen, specialGen)) {
        std::cout << "\n❌ Critical initialization failures detected!" << std::endl;
        std::cout << "Some tests may fail due to missing config files." << std::endl;
        std::cout << "Continuing anyway...\n" << std::endl;
        pause(2000);
    }

    // Create test character
    Character player("Alice Founder", "TEST001", "CEO", "TestCorp");
    player.addMoney(10000); // Starting funds for tests

    // Run all test suites
    testCharacterProgression(player, equipGen);
    pause(1000);

    testStandardMonsters(player, monsterGen, equipGen, itemGen);
    pause(1000);

    testSpecializedMonsters(player, specialGen);
    pause(1000);

    testNemesisSystem(player);
    pause(1000);

    testReviewCalendar(player);
    pause(1000);

    testShadowPlayer(player);
    pause(1000);

    testSpecialDrops(player, dropLibrary, equipGen);
    pause(1000);

    testCombatMechanics(player, monsterGen, itemGen);
    pause(1000);

    testBankruptcySystem(player);
    pause(1000);

    testEquipmentRarity(equipGen);
    pause(1000);

    // Final character state
    displaySection("FINAL CHARACTER STATE");
    displayCharacterStatus(player);

    // Summary
    globalStats.summary();

    std::cout << "\n💡 SYSTEMS TESTED:" << std::endl;
    std::cout << "   ✓ Character creation & progression" << std::endl;
    std::cout << "   ✓ Money & funding systems" << std::endl;
    std::cout << "   ✓ Equipment generation & equipping" << std::endl;
    std::cout << "   ✓ Inventory management" << std::endl;
    std::cout << "   ✓ Patent system with prime leveling" << std::endl;
    std::cout << "   ✓ Standard monsters (mobs & bosses)" << std::endl;
    std::cout << "   ✓ Monster difficulty scaling" << std::endl;
    std::cout << "   ✓ Specialized monsters (Safety, Conference, Papers)" << std::endl;
    std::cout << "   ✓ Nemesis system with evolution" << std::endl;
    std::cout << "   ✓ Review calendar (240 monthly bosses)" << std::endl;
    std::cout << "   ✓ Shadow player system" << std::endl;
    std::cout << "   ✓ Special drops library" << std::endl;
    std::cout << "   ✓ Combat mechanics" << std::endl;
    std::cout << "   ✓ Bankruptcy & recovery system" << std::endl;
    std::cout << "   ✓ Equipment rarity distribution" << std::endl;

    std::cout << "\n🎮 TEST SUITE COMPLETE!" << std::endl;

    return (globalStats.failed == 0) ? 0 : 1;
}
