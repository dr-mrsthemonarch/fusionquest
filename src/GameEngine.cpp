#include "../include/Tuning.h"
#include "../include/GameEngine.h"
#include "../include/StdoutSink.h"
#include "../include/Patent.h"
#include "../include/PatentStrategy.h"
#include <algorithm>
#include <cmath>

namespace {
// global tuner multiplier on special / quest / named equipment stats
void scaleSpecial(Equipment& e) {
    const double m = T("drop.special_mult");
    if (m == 1.0) return;
    e.runway = static_cast<int>(e.runway * m); e.hypeEnergy = static_cast<int>(e.hypeEnergy * m);
    e.hustle = static_cast<int>(e.hustle * m); e.agility = static_cast<int>(e.agility * m);
    e.ndas = static_cast<int>(e.ndas * m); e.luck = static_cast<int>(e.luck * m);
    e.synergy = static_cast<int>(e.synergy * m); e.busFactor = static_cast<float>(e.busFactor * m);
}
}  // namespace

GameEngine::GameEngine(bool debugInstakill)
    : debugInstakill_(debugInstakill), rng_(std::random_device{}()) {
    log_.setMirror(&detailLog_);
    StdoutSink::setLineHandler([this](const std::string& line) { detailLog_.add(line); });
}

bool GameEngine::loadConfig(const std::string& configRoot) {
    StdoutSink silence;
    configRoot_ = configRoot;
    equipGen_ = std::make_unique<EquipmentGenerator>();
    monsterGen_ = std::make_unique<MonsterGenerator>();
    itemGen_ = std::make_unique<ItemGenerator>();
    specialMonsterGen_ = std::make_unique<SpecializedMonsterGenerator>();
    nemesisSystem_ = std::make_unique<NemesisSystem>();
    reviewCalendarSystem_ = std::make_unique<ReviewCalendarSystem>();
    shadowPlayerSystem_ = std::make_unique<ShadowPlayerSystem>();
    namedMonsterLibrary_ = std::make_unique<NamedMonsterLibrary>();
    specialDropsLibrary_ = std::make_unique<SpecialDropsLibrary>();

    bool ok = true;
    ok &= equipGen_->loadWordLists(configRoot + "equipment/");
    ok &= monsterGen_->loadWordLists(configRoot + "mobs/");
    ok &= itemGen_->loadWordLists(configRoot + "items/");
    specialMonsterGen_->loadWordLists(configRoot + "mobs/"); // best-effort, not fatal
    namedMonsterLibrary_->loadFromFile(configRoot + "mobs/named_monsters.txt"); // optional

    Patent::loadPatentNames(configRoot + "items/patent_names.txt");
    Patent::initializePrimes();
    Patent::PatentLibrary = std::make_unique<PatentLibrary>();
    Patent::PatentLibrary->loadFromFile(configRoot + "items/named_patents.txt"); // optional

    log_.add(ok ? "config loaded" : "config load FAILED - check config/ path");
    return ok;
}

void GameEngine::initializeCharacter(const std::string& name, const std::string& employeeNr,
                                      const std::string& position, const std::string& startup) {
    StdoutSink silence;
    // --- full run-state reset (new game, or baseline before SaveGame::load) ---
    currentEncounter_.reset();
    inBattle_ = false; playerTurn_ = true; needsRecovery_ = false;
    waitingForNextBattle_ = false; waitingForBattleStart_ = false; battleStartTime_ = 0.0;
    roundNumber_ = 0; ticksSinceLastAction_ = 0; ticksSinceBattleEnd_ = 0;
    counters_ = Counters{};
    selloff_ = SelloffState{};
    shadowPlayerDefeated_ = false;
    lastNemesisTime_ = (-T("nemesis.interval_days") + T("nemesis.first_delay_days")) * 86400.0;
    lastEquipmentPurchaseSeries_ = 0;
    purchasedThisCycle_.clear();
    nemesisSystem_ = std::make_unique<NemesisSystem>();
    reviewCalendarSystem_ = std::make_unique<ReviewCalendarSystem>();
    shadowPlayerSystem_ = std::make_unique<ShadowPlayerSystem>();
    specialDropsLibrary_ = std::make_unique<SpecialDropsLibrary>();
    namedMonsterLibrary_ = std::make_unique<NamedMonsterLibrary>();
    namedMonsterLibrary_->loadFromFile(configRoot_ + "mobs/named_monsters.txt");
    Patent::characterPatents.clear();
    Character::setInventoryMapDirect({});
    log_.clear();
    detailLog_.clear();

    employeeNr_ = employeeNr;
    position_ = position;
    character_ = std::make_unique<Character>(name, employeeNr, position, startup);
    log_.add("character created: " + name);

    character_->addMoney(static_cast<int>(T("init.money")));
    character_->addFunding(10);

    cachedRequiredFunding_ = character_->getRequiredFunding();
    lastSeries_ = character_->getSeries();

    Equipment laptop = equipGen_->generateSpecificEquipmentForPlayer(EquipmentType::LAPTOP, character_->getSeries());
    Equipment backpack = equipGen_->generateSpecificEquipmentForPlayer(EquipmentType::BACKPACK, character_->getSeries());
    Equipment phone = equipGen_->generateSpecificEquipmentForPlayer(EquipmentType::PHONE, character_->getSeries());
    Equipment bike = equipGen_->generateSpecificEquipmentForPlayer(EquipmentType::BIKE, character_->getSeries());
    Equipment helmet = equipGen_->generateSpecificEquipmentForPlayer(EquipmentType::HELMET, character_->getSeries());

    character_->purchaseEquipment(laptop);   character_->equipItem(laptop);
    character_->purchaseEquipment(backpack); character_->equipItem(backpack);
    character_->purchaseEquipment(phone);    character_->equipItem(phone);
    character_->purchaseEquipment(bike);     character_->equipItem(bike);
    character_->purchaseEquipment(helmet);   character_->equipItem(helmet);

    Patent::generateStartingPatents(character_.get());
    reviewCalendarSystem_->generateReviewCalendar(character_->getSeries());

    Character::setSellProtection([this](const std::string& n) {
        return specialDropsLibrary_ && specialDropsLibrary_->isSpecialEquipment(n);
    });
    questSystem_ = std::make_unique<QuestSystem>();
    questSystem_->loadConfig(configRoot_);
    questSystem_->setCallbacks([this](const std::string& s) { log_.add(s); },
                               [this](bool isMain, const std::string& t) { grantQuestReward(isMain, t); });
    questSystem_->setTimeScale(timeScale_);
    questSystem_->start(0.0);

    initialized_ = true;
    initGeneration_++;
    log_.add("init complete. goal: series 20");
}

void GameEngine::tick(double elapsedSeconds) {
    StdoutSink silence;
    if (!initialized_) return;
    currentElapsedSeconds_ = elapsedSeconds;
    character_->setElapsedSeconds(elapsedSeconds);

    // selling is a short animated phase: nothing else advances until it finishes
    if (selloff_.active) { stepSelloff(); return; }

    questSystem_->tick(elapsedSeconds);
    if (questSystem_->awaitingAck()) return; // final quest popup open

    if (isGameWon()) return; // caller checks isGameWon() and stops feeding ticks / shows victory screen

    checkSeriesUpgrade();
    // patent level-ups run once per series-up inside Character::addFunding (was every tick -> log spam)
    checkHypeEnergyRecovery();

    if (needsRecovery_ && !inBattle_) { checkRecoveryNeeded(); return; }

    if (!inBattle_ && namedMonsterLibrary_) {
        auto* def = namedMonsterLibrary_->checkForEncounter(elapsedSeconds, character_->getSeries());
        if (def) {
            auto namedEncounter = NamedMonsterSystem{}.generateNamedEncounter(*def, character_->getSeries());
            currentEncounter_ = std::make_unique<BattleEncounter>();
            for (auto& e : namedEncounter) currentEncounter_->addEnemy(std::make_unique<Monster>(std::move(e)));
            inBattle_ = true; roundNumber_ = 1;
            playerTurn_ = currentEncounter_->getTotalCount() > 0
                ? character_->getLuck() >= currentEncounter_->getEnemy(0)->speed : true;
            counters_.totalBattles++; ticksSinceLastAction_ = 0;
            log_.add("NAMED MONSTER: " + currentEncounter_->getEnemy(0)->name);
            return;
        }
    }

    if (!inBattle_) {
        checkMonthlyReview();
        if (inBattle_) return;
        checkNemesisEncounter();
        if (inBattle_) return;
        checkShadowPlayer();
        if (inBattle_) return;
        if (character_->isInventoryFull()) { checkInventoryFull(); return; }
    }

    if (!inBattle_) {
        if (waitingForNextBattle_) {
            ticksSinceBattleEnd_++;
            if (ticksSinceBattleEnd_ >= static_cast<int>(T("time.ticks_between_battles"))) {
                waitingForNextBattle_ = false; ticksSinceBattleEnd_ = 0;
            } else return;
        }

        checkSpecialEncounters();
        if (!inBattle_) {
            int mobCount = 0, bossCount = 0;
            syncBias();
            auto enemies = monsterGen_->generateGroupEncounter(character_->getSeries(), mobCount, bossCount, *itemGen_);
            currentEncounter_ = std::make_unique<BattleEncounter>();
            for (auto& e : enemies) currentEncounter_->addEnemy(std::move(e));

            inBattle_ = true; roundNumber_ = 1;
            playerTurn_ = character_->getLuck() >= currentEncounter_->getEnemy(0)->speed;
            counters_.totalBattles++; ticksSinceLastAction_ = 0;
            waitingForBattleStart_ = true; battleStartTime_ = 0;

            log_.add("encounter #" + std::to_string(counters_.totalBattles) + ": "
                      + std::to_string(mobCount) + " mob(s), " + std::to_string(bossCount) + " boss(es)");
        }
    } else {
        if (waitingForBattleStart_) {
            battleStartTime_ += 0.1;
            if (battleStartTime_ >= T("time.battle_start_delay")) { waitingForBattleStart_ = false; log_.add("battle begins!"); }
            return;
        }
        ticksSinceLastAction_++;
        if (ticksSinceLastAction_ >= static_cast<int>(T("time.ticks_per_action"))) { handleBattle(); ticksSinceLastAction_ = 0; }
    }
}

void GameEngine::checkSeriesUpgrade() {
    int cur = character_->getSeries();
    if (cur > lastSeries_) {
        cachedRequiredFunding_ = character_->getRequiredFunding();
        lastSeries_ = cur;
        log_.add("SERIES UP! now series " + std::to_string(cur));
        purchaseNewEquipment();
    }
}

void GameEngine::checkHypeEnergyRecovery() {
    int maxHype = character_->getMaxHypeEnergy();
    int cur = character_->getHypeEnergy();
    if (cur < maxHype * T("hype.low_threshold")) {
        int cost = static_cast<int>(character_->getMoney() * T("hype.cost_pct"));
        int recover = static_cast<int>(maxHype * T("hype.recover_frac"));
        if (character_->getMoney() >= cost && cost > 0) {
            character_->spendMoney(cost);
            character_->setHypeEnergy(std::min(maxHype, cur + recover));
            log_.add("hype recovery: spent $" + std::to_string(cost) + " for +" + std::to_string(recover));
        }
    }
}

void GameEngine::checkSpecialEncounters() {
    int chance = std::max(1, static_cast<int>(T("special.spawn_odds")));
    std::uniform_int_distribution<> d(1, chance);

    if (d(rng_) <= 1) {
        auto encounter = specialMonsterGen_->generateSafetyManagerEncounter(character_->getSeries(), currentElapsedSeconds_);
        currentEncounter_ = std::make_unique<BattleEncounter>();
        for (auto& e : encounter) currentEncounter_->addEnemy(std::make_unique<Monster>(std::move(e)));
        log_.add("SAFETY MANAGER + 2 cronies!");
        inBattle_ = true; roundNumber_ = 1;
        playerTurn_ = character_->getAgility() >= currentEncounter_->getEnemy(0)->speed;
        counters_.totalBattles++; ticksSinceLastAction_ = 0;
        return;
    }
    if (d(rng_) <= 1) {
        currentEncounter_ = std::make_unique<BattleEncounter>();
        currentEncounter_->addEnemy(std::make_unique<Monster>(
            specialMonsterGen_->generateConference(character_->getSeries(), currentElapsedSeconds_)));
        log_.add("CONFERENCE: " + currentEncounter_->getEnemy(0)->name);
        inBattle_ = true; roundNumber_ = 1;
        playerTurn_ = character_->getAgility() >= currentEncounter_->getEnemy(0)->speed;
        counters_.totalBattles++; ticksSinceLastAction_ = 0;
        return;
    }
    if (d(rng_) <= 1) {
        currentEncounter_ = std::make_unique<BattleEncounter>();
        currentEncounter_->addEnemy(std::make_unique<Monster>(
            specialMonsterGen_->generatePaperWriting(character_->getSeries(), currentElapsedSeconds_)));
        log_.add("PAPER DEADLINE: " + currentEncounter_->getEnemy(0)->name);
        inBattle_ = true; roundNumber_ = 1;
        playerTurn_ = character_->getAgility() >= currentEncounter_->getEnemy(0)->speed;
        counters_.totalBattles++; ticksSinceLastAction_ = 0;
    }
}

void GameEngine::checkNemesisEncounter() {
    if (nemesisSystem_->shouldNemesisAppear(currentElapsedSeconds_, lastNemesisTime_)) {
        log_.add("NEMESIS ENCOUNTER!");
        currentEncounter_ = std::make_unique<BattleEncounter>();
        currentEncounter_->addEnemy(std::make_unique<Monster>(
            nemesisSystem_->generateNemesis(character_->getSeries(), currentElapsedSeconds_, character_->getName())));
        log_.add(currentEncounter_->getEnemy(0)->name + " appears!");
        inBattle_ = true; roundNumber_ = 1;
        playerTurn_ = character_->getLuck() >= currentEncounter_->getEnemy(0)->speed;
        counters_.totalBattles++; ticksSinceLastAction_ = 0;
        lastNemesisTime_ = currentElapsedSeconds_;
    }
}

void GameEngine::checkMonthlyReview() {
    ScheduledBoss* sched = reviewCalendarSystem_->checkForScheduledReview(currentElapsedSeconds_);
    if (sched) {
        log_.add("MONTHLY REVIEW: " + sched->bossData.name);
        currentEncounter_ = std::make_unique<BattleEncounter>();
        Monster reviewBoss;
        const BossData& d = sched->bossData;
        reviewBoss.name = d.name; reviewBoss.level = d.level;
        reviewBoss.maxHealth = d.maxHealth; reviewBoss.health = d.maxHealth;
        reviewBoss.attack = d.attack; reviewBoss.defense = d.defense; reviewBoss.speed = d.speed;
        reviewBoss.moneyReward = d.moneyReward; reviewBoss.itemReward = d.itemReward;
        reviewBoss.type = d.type; reviewBoss.difficulty = d.difficulty;
        reviewBoss.maxHypeEnergy = d.maxHypeEnergy; reviewBoss.hypeEnergy = d.maxHypeEnergy;

        switch (d.difficulty) {
            case MonsterDifficulty::EASY:
            case MonsterDifficulty::MEDIUM: reviewBoss.setStrategy(std::make_unique<BalancedStrategy>()); break;
            case MonsterDifficulty::HARD:
            case MonsterDifficulty::LEGENDARY: reviewBoss.setStrategy(std::make_unique<TacticalStrategy>()); break;
        }
        currentEncounter_->addEnemy(std::make_unique<Monster>(std::move(reviewBoss)));
        sched->encountered = true;

        inBattle_ = true; roundNumber_ = 1;
        playerTurn_ = character_->getLuck() >= currentEncounter_->getEnemy(0)->speed;
        counters_.totalBattles++; ticksSinceLastAction_ = 0;
    }
}

void GameEngine::checkShadowPlayer() {
    if (shadowPlayerDefeated_) return;
    if (shadowPlayerSystem_->shouldShadowPlayerAppear(currentElapsedSeconds_, shadowPlayerDefeated_)) {
        log_.add("SHADOW LINK APPEARS");
        currentEncounter_ = std::make_unique<BattleEncounter>();
        currentEncounter_->addEnemy(std::make_unique<Monster>(
            shadowPlayerSystem_->generateShadowPlayer(*character_, currentElapsedSeconds_)));
        log_.add(currentEncounter_->getEnemy(0)->name);
        inBattle_ = true; roundNumber_ = 1;
        playerTurn_ = character_->getLuck() >= currentEncounter_->getEnemy(0)->speed;
        counters_.totalBattles++; ticksSinceLastAction_ = 0;
    }
}

void GameEngine::checkInventoryFull() {
    if (character_->isInventoryFull()) { log_.add("inventory full, visiting town..."); visitTown(); }
}

void GameEngine::checkRecoveryNeeded() {
    if (character_->getRunway() == 1 && needsRecovery_) { visitTownForRecovery(); needsRecovery_ = false; }
}

void GameEngine::checkDefensivePatents() {
    std::uniform_int_distribution<> d(0, 99);
    if (d(rng_) < T("patent.defensive_pct")) {
        auto defense = Patent::getPatentsByType(PatentType::DEFENSE);
        for (const auto& name : defense) {
            auto it = Patent::characterPatents.find(name);
            if (it != Patent::characterPatents.end() && character_->getHypeEnergy() >= it->second.getCost()) {
                Patent::usePatent(name, character_.get(), character_.get());
                log_.add(it->second.getFullName() + " activated defensively!");
                break;
            }
        }
    }
}

void GameEngine::handleBattle() {
    if (!character_ || !currentEncounter_ || !inBattle_) return;
    if (!currentEncounter_->hasAliveEnemies()) return;

    if (playerTurn_) {
        character_->processTurnEffects();
        if (!character_->isAlive()) { handleDefeat(); return; }
        if (playerAttack()) { handleVictory(); return; }
    } else {
        for (auto* enemy : currentEncounter_->getAliveEnemies()) {
            enemy->processTurnEffects();
            if (!enemy->isAlive()) { log_.add(enemy->name + " died from DoT!"); continue; }
            monsterAttack(enemy);
            if (!character_->isAlive()) { handleDefeat(); return; }
        }
        if (!currentEncounter_->hasAliveEnemies()) { handleVictory(); return; }

        if (character_->getRunway() < character_->getMaxRunway() * T("emerg.heal_frac")) tryEmergencyHealing();
        if (character_->getRunway() < T("emerg.sale_runway") && character_->getInventoryCount() > 0) {
            beginSelloff(true, false); // runway restored when the last item is sold
        }
    }

    playerTurn_ = !playerTurn_;
    if (playerTurn_) roundNumber_++;
}

bool GameEngine::playerAttack() {
    if (!currentEncounter_) return false;

    if (debugInstakill_) {
        for (auto* e : currentEncounter_->getAllEnemies()) e->takeDamage(9999);
        return !currentEncounter_->hasAliveEnemies();
    }

    bool usedPatent = false;
    float healthPct = static_cast<float>(character_->getRunway()) / character_->getMaxRunway();
    float hypePct = static_cast<float>(character_->getHypeEnergy()) / character_->getMaxHypeEnergy();

    if (character_->canUsePatents() && hypePct > T("patent.min_hype_frac")) {
        std::uniform_int_distribution<> roll(0, 99);
        int threshold = static_cast<int>(T("patent.use_base_pct"));
        if (healthPct < T("patent.low_hp_frac")) threshold = static_cast<int>(T("patent.low_hp_pct"));
        else if (healthPct < T("patent.mid_hp_frac")) threshold = static_cast<int>(T("patent.mid_hp_pct"));
        if (hypePct < T("patent.low_hype_frac")) threshold = static_cast<int>(T("patent.low_hype_pct"));

        if (roll(rng_) < threshold) {
            std::vector<std::pair<std::string, Patent*>> affordable;
            for (auto& p : Patent::characterPatents)
                if (character_->getHypeEnergy() >= p.second.getCost()) affordable.push_back({p.first, &p.second});

            if (!affordable.empty()) {
                std::string chosen;
                if (currentEncounter_->getAliveCount() > 2) {
                    for (auto& [n, p] : affordable) if (p->getType() == PatentType::AOE) { chosen = n; break; }
                }
                if (chosen.empty() && healthPct < T("patent.heal_choice_frac")) {
                    for (const auto& hn : Patent::getPatentsByType(PatentType::HEAL))
                        for (auto& [n, p] : affordable) if (n == hn) { chosen = n; break; }
                }
                if (chosen.empty()) {
                    std::uniform_int_distribution<> pick(0, affordable.size() - 1);
                    chosen = affordable[pick(rng_)].first;
                }

                auto& patent = Patent::characterPatents[chosen];
                if (patent.getType() == PatentType::AOE) {
                    std::vector<CombatEntity*> targets;
                    for (auto* e : currentEncounter_->getAliveEnemies()) targets.push_back(e);
                    patent.applyAOEEffect(character_.get(), targets);
                } else {
                    CombatEntity* target = currentEncounter_->getLowestHPEnemy();
                    if (patent.getTarget() == PatentTarget::SELF) target = character_.get();
                    Patent::usePatent(chosen, character_.get(), target);
                }
                usedPatent = true;
                log_.add("used " + patent.getFullName() + "!");
            }
        }
    }

    if (!usedPatent) {
        Monster* target = currentEncounter_->getLowestHPEnemy();
        if (!target) return false;
        const int varianceRange = static_cast<int>(T("combat.variance"));
        std::uniform_int_distribution<> variance(-varianceRange, varianceRange);
        int dmg = std::max(1, static_cast<int>((character_->getModifiedHustle() - target->getModifiedNDAs()) * T("combat.player_dmg_mult")) + variance(rng_));
        target->takeDamage(dmg);
        log_.add("dealt " + std::to_string(dmg) + " dmg to " + target->getName());
    }

    checkDefensivePatents(); // symmetry w/ original had this on monster side too; harmless here
    return !currentEncounter_->hasAliveEnemies();
}

void GameEngine::monsterAttack(Monster* enemy) {
    if (!enemy) return;
    bool usedPatent = false;
    if (enemy->canUsePatents() && enemy->getStrategy()) {
        usedPatent = enemy->useStrategicPatent(character_.get());
        if (usedPatent) log_.add(enemy->getName() + " used strategic magic!");
    }
    if (!usedPatent) {
        const int varianceRange = static_cast<int>(T("combat.variance"));
        std::uniform_int_distribution<> variance(-varianceRange, varianceRange);
        int dmg = std::max(1, static_cast<int>((enemy->getModifiedHustle() - character_->getModifiedNDAs()) * T("combat.enemy_dmg_mult")) + variance(rng_));
        character_->takeDamage(dmg);
        log_.add(enemy->getName() + " dealt " + std::to_string(dmg) + " dmg");
    }
    checkDefensivePatents();
}

void GameEngine::handleVictory() {
    if (!currentEncounter_) return;
    auto allEnemies = currentEncounter_->getAllEnemies();
    log_.add("VICTORY! defeated " + std::to_string(currentEncounter_->getTotalCount())
              + " in " + std::to_string(roundNumber_) + " rounds");

    int moneyReward = currentEncounter_->getTotalMoneyReward();
    int fundingReward = static_cast<int>(moneyReward * T("funding.per_money"));
    character_->addMoney(moneyReward);
    character_->setElapsedSeconds(currentElapsedSeconds_);
    character_->addFunding(fundingReward);
    log_.add("earned $" + std::to_string(moneyReward) + " | funding +" + std::to_string(fundingReward));
    for (auto* e : allEnemies) questSystem_->onKill(e->name, e->type != MonsterType::MOB);

    bool hasSpecialDrop = false;
    for (auto* e : allEnemies) {
        if (e->type == MonsterType::NEMESIS || e->type == MonsterType::REVIEW ||
            e->name.find("Shadow:") != std::string::npos || e->name.find("Safety Manager") != std::string::npos) {
            hasSpecialDrop = true;
            handleSpecialDrop(*e);
        }
    }

    // named monster killed: record defeat so it stops blocking (this was missing -> same fight forever)
    if (namedMonsterLibrary_ && namedMonsterLibrary_->hasBlockingEncounter()) {
        NamedMonsterDef* def = namedMonsterLibrary_->getBlockingMonster();
        const std::string defName = def->name;
        const EquipmentType defType = def->equipmentType;
        const EquipmentRarity defRarity = def->equipmentRarity;
        bool wasNamed = false;
        for (auto* e : allEnemies) if (e->name == defName) wasNamed = true;
        if (wasNamed) {
            const double SPY = 365.25 * 24 * 60 * 60, SPM = SPY / 12.0;
            int year = static_cast<int>(currentElapsedSeconds_ / SPY) + 1;
            int month = std::clamp(static_cast<int>((currentElapsedSeconds_ - (year - 1) * SPY) / SPM) + 1, 1, 12);
            Equipment cert = specialDropsLibrary_->generateNamedMonsterCertificate(defName, defType, defRarity, character_->getSeries());
            scaleSpecial(cert);
            addSpecialDropToLibrary(SpecialDropType::NAMED_MONSTER_DROP, cert, defName, year, month);
            character_->addEquipmentToInventory(cert);
            namedMonsterLibrary_->recordDefeat(defName);
            hasSpecialDrop = true; // no extra boss equipment drop
            log_.add("NAMED MONSTER DEFEATED: " + defName);
        }
    }

    auto itemRewards = currentEncounter_->getAllItemRewards();
    for (const auto& itemName : itemRewards) {
        Monster* dropper = nullptr;
        for (auto* e : allEnemies) if (e->type == MonsterType::MOB && e->itemReward == itemName) { dropper = e; break; }
        if (dropper) { character_->addInventoryItem(itemName, dropper->itemValue); log_.add("item: " + itemName); questSystem_->onItem(itemName); }
    }

    for (auto* e : allEnemies) {
        if (e->type == MonsterType::BOSS && !hasSpecialDrop) {
            Equipment reward = equipGen_->generateBossDropEquipment(character_->getSeries());
            character_->addEquipmentToInventory(reward);
            counters_.bossesDefeated++;
            log_.add("equipment: " + reward.name);
        }
    }

    counters_.totalVictories++;
    inBattle_ = false;
    currentEncounter_.reset();
    waitingForNextBattle_ = true;
}

void GameEngine::handleDefeat() {
    std::string defeatedBy = "unknown";
    if (currentEncounter_ && currentEncounter_->hasAliveEnemies()) {
        auto alive = currentEncounter_->getAliveEnemies();
        if (!alive.empty()) defeatedBy = alive[0]->getName();
    }
    log_.add("BANKRUPTCY! defeated by " + defeatedBy);
    counters_.bankruptcies++;
    counters_.totalDefeats++;
    character_->handleDeath();
    needsRecovery_ = true;
    inBattle_ = false;
    currentEncounter_.reset();
}

void GameEngine::handleSpecialDrop(const Monster& enemy) {
    const double SECONDS_PER_YEAR = 365.25 * 24 * 60 * 60;
    const double SECONDS_PER_MONTH = SECONDS_PER_YEAR / 12.0;
    int year = static_cast<int>(currentElapsedSeconds_ / SECONDS_PER_YEAR) + 1;
    int month = static_cast<int>((currentElapsedSeconds_ - (year - 1) * SECONDS_PER_YEAR) / SECONDS_PER_MONTH) + 1;
    month = std::clamp(month, 1, 12);

    Equipment eq; SpecialDropType type;
    if (enemy.type == MonsterType::NEMESIS) {
        type = SpecialDropType::NEMESIS_DROP;
        nemesisSystem_->recordNemesisDefeat(year, currentElapsedSeconds_, nemesisSystem_->getNemesisType(currentElapsedSeconds_), enemy.name);
        int power = nemesisSystem_->getNemesisPowerLevel() - 1; // 0 on first kill
        eq = specialDropsLibrary_->generateNemesisEquipment(character_->getSeries(), power, enemy.name);
    } else if (enemy.name.find("Shadow:") != std::string::npos) {
        type = SpecialDropType::SHADOW_PLAYER_DROP;
        eq = specialDropsLibrary_->generateShadowPlayerEquipment(character_->getSeries(), character_->getName());
        shadowPlayerDefeated_ = true;
    } else if (enemy.name.find("Safety Manager") != std::string::npos) {
        type = SpecialDropType::SAFETY_MANAGER_DROP;
        eq = specialDropsLibrary_->generateSafetyManagerEquipment(character_->getSeries());
    } else if (enemy.name.find("Conference") != std::string::npos) {
        type = SpecialDropType::CONFERENCE_DROP;
        eq = specialDropsLibrary_->generateConferenceEquipment(character_->getSeries());
    } else if (enemy.name.find("Paper") != std::string::npos) {
        type = SpecialDropType::PAPER_WRITING_DROP;
        eq = specialDropsLibrary_->generatePaperWritingEquipment(character_->getSeries());
    } else if (enemy.type == MonsterType::REVIEW) {
        type = SpecialDropType::MONTHLY_REVIEW_DROP;
        eq = specialDropsLibrary_->generateMonthlyReviewEquipment(year, month, character_->getSeries());
    } else return;

    scaleSpecial(eq);
    addSpecialDropToLibrary(type, eq, enemy.name, year, month);
    character_->addEquipmentToInventory(eq);
    log_.add("SPECIAL DROP: " + eq.name);
}

void GameEngine::addSpecialDropToLibrary(SpecialDropType type, const Equipment& equipment,
                                          const std::string& sourceName, int year, int month) {
    SpecialDrop drop;
    drop.type = type; drop.equipment = equipment; drop.name = equipment.name;
    drop.acquiredFrom = sourceName; drop.yearAcquired = year; drop.monthAcquired = month;
    drop.isEquipped = false;

    switch (type) {
        case SpecialDropType::NEMESIS_DROP: drop.loreText = "Forged from the essence of a defeated nemesis."; break;
        case SpecialDropType::SHADOW_PLAYER_DROP: drop.loreText = "A reflection from another dimension."; break;
        case SpecialDropType::SAFETY_MANAGER_DROP: drop.loreText = "Blessed by the Safety Manager."; break;
        case SpecialDropType::CONFERENCE_DROP: drop.loreText = "Earned through networking."; break;
        case SpecialDropType::PAPER_WRITING_DROP: drop.loreText = "Culmination of rigorous research."; break;
        case SpecialDropType::MONTHLY_REVIEW_DROP: drop.loreText = "Trophy for exceptional performance."; break;
        default: break;
    }
    specialDropsLibrary_->addSpecialDrop(drop);
}

void GameEngine::purchaseNewEquipment() {
    if (character_->getSeries() % 8 == 1) { purchasedThisCycle_.clear(); lastEquipmentPurchaseSeries_ = character_->getSeries(); }

    std::vector<EquipmentType> types = {EquipmentType::LAPTOP, EquipmentType::PHONE, EquipmentType::BACKPACK,
        EquipmentType::HELMET, EquipmentType::BIKE, EquipmentType::JACKET, EquipmentType::GLOVES, EquipmentType::HEADPHONES};
    std::shuffle(types.begin(), types.end(), rng_);
    if (purchasedThisCycle_.size() >= types.size()) purchasedThisCycle_.clear(); // all slots bought once: new cycle

    for (auto t : types) {
        if (purchasedThisCycle_.count(t)) continue;
        Equipment eq = equipGen_->generateSpecificEquipmentForPlayer(t, character_->getSeries());
        const Equipment* cur = character_->getEquippedItem(t);
        if (cur && cur->equipmentvalue >= eq.equipmentvalue) continue; // only buy upgrades
        if (character_->getMoney() >= eq.equipmentvalue && character_->purchaseEquipment(eq)) {
            character_->equipItem(eq);
            purchasedThisCycle_.insert(t);
            log_.add("purchased+equipped: " + eq.name + " ($" + std::to_string(eq.equipmentvalue) + ")");
            break;
        }
    }
}

void GameEngine::tryEmergencyHealing() {
    for (const auto& n : Patent::getPatentsByType(PatentType::HEAL)) {
        const Patent& p = Patent::characterPatents[n];
        if (character_->getHypeEnergy() >= p.getCost()) { Patent::usePatent(n, character_.get(), character_.get()); log_.add("emergency heal: " + p.getFullName()); return; }
    }
    for (const auto& n : Patent::getPatentsByType(PatentType::DEFENSE)) {
        const Patent& p = Patent::characterPatents[n];
        if (character_->getHypeEnergy() >= p.getCost()) { Patent::usePatent(n, character_.get(), character_.get()); log_.add("emergency defense: " + p.getFullName()); return; }
    }
    for (const auto& n : Patent::getPatentsByType(PatentType::BUFF)) {
        const Patent& p = Patent::characterPatents[n];
        if (character_->getHypeEnergy() >= p.getCost()) { Patent::usePatent(n, character_.get(), character_.get()); log_.add("emergency buff: " + p.getFullName()); return; }
    }
}

void GameEngine::visitTown() {
    beginSelloff(false, true); // purchase + visit count happen in finishSelloff()
}

void GameEngine::beginSelloff(bool emergency, bool includeEquipment) {
    if (selloff_.active) return;
    selloff_ = SelloffState{};
    selloff_.emergency = emergency;
    selloff_.includeEquipment = includeEquipment;
    // snapshot in the same order the stats page lists them; sell from the bottom up
    for (const auto& [name, val] : character_->getInventoryMap()) selloff_.itemNames.push_back(name);
    selloff_.total = static_cast<int>(selloff_.itemNames.size()) +
                     (includeEquipment ? character_->getSellableEquipmentCount() : 0);
    selloff_.active = true;
    log_.add(emergency ? "EMERGENCY SALE! selling inventory..." : "selling inventory...");
    if (selloff_.total == 0) finishSelloff();
}

void GameEngine::stepSelloff() {
    if (++selloff_.ticks < static_cast<int>(T("time.sell_step_ticks"))) return;
    selloff_.ticks = 0;

    // inventory items first, last one in the list first
    while (!selloff_.itemNames.empty()) {
        std::string name = selloff_.itemNames.back();
        selloff_.itemNames.pop_back();
        if (!character_->getInventoryMap().count(name)) continue; // already gone, try previous
        int n = std::max(1, character_->getInventoryStack(name));
        int value = character_->getInventoryValue(name) * n; // whole stack per step
        character_->removeInventoryItem(name);
        character_->addMoney(value);
        selloff_.earned += value;
        log_.add("sold " + name + (n > 1 ? " x" + std::to_string(n) : "") + " for $" + std::to_string(value));
        return;
    }

    // then unequipped gear (60% of value, matches Character::sellEquipment)
    if (selloff_.includeEquipment) {
        const auto& inv = character_->getEquipmentInventoryVec();
        for (size_t i = inv.size(); i-- > 0;) {
            if (specialDropsLibrary_->isSpecialEquipment(inv[i].name)) continue; // unsellable
            std::string name = inv[i].name;
            int price = static_cast<int>(inv[i].equipmentvalue * T("econ.sell_ratio"));
            character_->sellEquipmentByIndex(i);
            selloff_.earned += price;
            log_.add("sold " + name + " for $" + std::to_string(price));
            return;
        }
    }

    finishSelloff();
}

void GameEngine::finishSelloff() {
    selloff_.active = false;
    if (selloff_.emergency) {
        int recovered = static_cast<int>(selloff_.earned / std::max(1.0, T("emerg.runway_money_div")));
        character_->setRunway(std::min(character_->getRunway() + recovered, character_->getMaxRunway()));
        log_.add("emergency sale done: +" + std::to_string(recovered) + " runway");
    } else {
        purchaseNewEquipment();
        log_.add("town: earned $" + std::to_string(selloff_.earned));
        questSystem_->onTownVisit();
        counters_.townVisits++;
    }
    selloff_ = SelloffState{};
}

void GameEngine::visitTownForRecovery() {
    character_->restoreToFull();
    log_.add("emergency recovery: full restore");
    counters_.townVisits++;
}

float GameEngine::getActivityProgress() const {
    if (!initialized_) return 0.0f;
    if (selloff_.active) {
        if (selloff_.total <= 0) return 1.0f;
        int remaining = static_cast<int>(selloff_.itemNames.size()) +
                        (selloff_.includeEquipment ? character_->getSellableEquipmentCount() : 0);
        return 1.0f - std::clamp(static_cast<float>(remaining) / selloff_.total, 0.0f, 1.0f);
    }
    if (!inBattle_) {
        if (waitingForNextBattle_)
            return std::min(1.0f, static_cast<float>(ticksSinceBattleEnd_) / static_cast<int>(T("time.ticks_between_battles")));
        return 0.0f;
    }
    if (waitingForBattleStart_) return std::min(1.0f, static_cast<float>(battleStartTime_ / std::max(0.1, T("time.battle_start_delay"))));
    if (!currentEncounter_) return 0.0f;

    double hp = 0, maxHp = 0;
    for (auto* e : currentEncounter_->getAllEnemies()) {
        hp += std::max(0, e->health);
        maxHp += std::max(1, e->maxHealth);
    }
    return maxHp > 0 ? static_cast<float>(1.0 - hp / maxHp) : 0.0f;
}

float GameEngine::getFundingFraction() const {
    if (!character_ || cachedRequiredFunding_ <= 0) return 0.0f;
    return static_cast<float>(std::clamp(character_->getCurrentFunding() / cachedRequiredFunding_, 0.0, 1.0));
}

void GameEngine::onStateRestored() {
    cachedRequiredFunding_ = character_->getRequiredFunding();
    lastSeries_ = character_->getSeries();
    purchasedThisCycle_.clear();
}

void GameEngine::syncBias() {
    const auto& b = questSystem_->balance();
    monsterGen_->setKillBias(questSystem_->killKeywords(), b.killBias);
    itemGen_->setCollectBias(questSystem_->collectKeywords(), b.collectBias);
}

void GameEngine::grantQuestReward(bool isMain, const std::string& title) {
    const auto& b = questSystem_->balance();
    character_->setElapsedSeconds(currentElapsedSeconds_);
    double funding = character_->getRequiredFunding() * (isMain ? b.rewardMainPct : b.rewardHustlePct) / 100.0;
    character_->addFunding(funding);

    std::uniform_int_distribution<> d(0, 99);
    bool hi = d(rng_) < 30;
    EquipmentRarity r = isMain ? (hi ? EquipmentRarity::LEGENDARY : EquipmentRarity::EPIC)
                               : (hi ? EquipmentRarity::EPIC : EquipmentRarity::RARE);
    Equipment eq = specialDropsLibrary_->generateQuestEquipment(character_->getSeries(), r, title);
    scaleSpecial(eq);

    if (isMain) {
        const double SPY = 365.25 * 24 * 60 * 60;
        const double SPM = SPY / 12.0;
        int year = static_cast<int>(currentElapsedSeconds_ / SPY) + 1;
        int month = std::clamp(static_cast<int>((currentElapsedSeconds_ - (year - 1) * SPY) / SPM) + 1, 1, 12);
        addSpecialDropToLibrary(SpecialDropType::QUEST_DROP, eq, title, year, month);
    }
    character_->addEquipmentToInventory(eq);
    log_.add("quest reward: " + eq.name + " | funding +" + std::to_string(static_cast<int>(funding)));
}
