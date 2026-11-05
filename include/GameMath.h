#ifndef GAMEMATH_H
#define GAMEMATH_H

// ALL game math lives here. read factors from Tuning (T("key")). used by the game AND the debug tuner,
// so what the tuner shows is what the game computes. no randomness here (callers pass rolls/noise in).
// to change a curve: edit it in GameMath.cpp only. numbers = Tuning defaults (see Tuning.cpp).
#include <string>
#include <vector>
#include "Equipment.h"

namespace GameMath {

struct EnemyStats { double hp = 0, atk = 0, def = 0, spd = 0, money = 0; };
struct PlayerStats {
    double runway = 0, hype = 0, hustle = 0, agility = 0, ndas = 0, luck = 0, synergy = 0, techDebt = 0, bus = 0;
};

constexpr double kTotalGameSeconds = 20.0 * 365 * 24 * 3600;

double difficultyMult(int difficulty);                    // 0 easy,1 medium,2 hard,3 legendary
EnemyStats mob(int series, double diffMult);
EnemyStats boss(int series, double diffMult);
EnemyStats nemesis(int series, double elapsedSeconds, int type, int power);  // type 0..3
EnemyStats special(int kind, int series, double diffMult);   // 0 safety manager, 1 conference, 2 paper (else base)
EnemyStats review(int year, int month, double diffMult);     // monthly performance review boss

// ---- stat effects (fractions 0..1; all tunable, capped) ----
bool chance(double p);
double critChance(double luck);
double critMult(double luck, double synergy, double bonus = 0.0);
double dodgeChance(double defenderAgility, double attackerSpeed);
double extraActionChance(double agility, double foeSpeed, double bonus = 0.0);
double failChance(double techDebt);                    // techDebt negative = debt
double breakChance(double techDebt, double resist = 0.0);
double patentPotency(double synergy);                  // power multiplier
double hypeDiscount(double synergy, double bonus = 0.0); // cost multiplier
double luckRarityBonus(double luck);                   // subtracted from 1..10000 boss rarity roll
double luckItemMult(double luck, double bonus = 0.0);
void setProjectionClass(int id);                       // tuner only (-1 = none)
std::string projectionClassName();

PlayerStats levelGain(int series);                        // gain applied when reaching `series`
PlayerStats player(int series);                           // base stats at series (0 = start), no gear

double fundingRequired(int series, double elapsedSeconds, double noise = 1.0);
double equipStat(int series, double mult = 1.0);          // mean equipment stat

// ---- projection model (tuner): expected fights + progression, analytic (no RNG) ----
struct SeriesMetrics {
    int series = 0;
    PlayerStats player;                  // base + modeled gear applied to hustle/ndas
    double hustle = 0, ndas = 0, runway = 0;
    EnemyStats mob, boss, nemesis;       // medium difficulty
    double hitsMob = 0, hitsBoss = 0, hitsNemesis = 0;        // player hits to kill one
    double survMob = 0, survBoss = 0, survNemesis = 0;        // enemy hits to kill player
    double secondsPerEncounter = 0, fundingPerEncounter = 0;  // mix-averaged
    double dmgPerEncounter = 0, encountersPerLife = 0;
    double secondsToNextSeries = 0;
};

// gear model from slot multipliers: hustle kit = laptop+gloves, NDAs kit = laptop+helmet
void modelGear(int series, double& gearHustle, double& gearNdas);
SeriesMetrics seriesMetrics(int series, double elapsedSeconds, double gearHustle, double gearNdas);

struct Projection {
    std::vector<SeriesMetrics> atYear;   // index = year 0..20 (state when year starts)
    std::vector<int> seriesAtYear;
};
Projection project();


// =====================================================================
// SHARED HELPERS
// =====================================================================
// every growth curve = base + lin*x (+ quad*x^2). key names = Tuning registry keys.
struct Curve { const char* base; const char* lin; const char* quad = nullptr; };
double eval(const Curve& c, double x);
double dv(const char* key);        // divisor guard: max(0.01, T(key))

// =====================================================================
// COMBAT / ECONOMY
// =====================================================================
int combatVariance();
int playerDamage(double atk, double def, int noise);       // max(1,(atk-def)*mult)+noise
int enemyDamage(double atk, double def, int noise);
int fundingFromMoney(int money);
bool hypeRecoveryNeeded(int cur, int maxHype);
int hypeRecoveryCost(int money);
int hypeRecoveryAmount(int maxHype);
bool emergencyHealNeeded(int runway, int maxRunway);
bool emergencySaleNeeded(int runway);
int emergencyRunwayGain(int earned);
double sellRatio(double synergy);                          // equipment sell price / value
int repairCost(int itemValue, double classMult);
double questRewardFunding(double requiredFunding, bool main);
EquipmentRarity questRewardRarity(bool main, int roll0to99);
int itemBaseRange();
int itemStyleBonus(int nameStyle);                         // 0-99 name style roll

// =====================================================================
// EQUIPMENT
// =====================================================================
double equipVariance();
int backpackNoiseRange();
int backpackSlots(int series, int noise);
double rarityStatMult(EquipmentRarity r);
double rarityValueMult(EquipmentRarity r);
EquipmentRarity bossRarity(int roll1to10000);
double equipValue(const Equipment& e);

// special drops / named / quest gear (stat table, per kind: gear.<kind>.* keys)
struct GearStats {
    int runway = 0, hype = 0, hustle = 0, agility = 0, ndas = 0, luck = 0, synergy = 0, techDebt = 0;
    float bus = 0; int value = 0, inventoryLimit = 0;
};
GearStats gearNemesis(int series, int power);
GearStats gearShadow(int series);
GearStats gearSafety(int series);
GearStats gearConference(int series);
GearStats gearPaper(int series);
GearStats gearReview(int year, int series);
GearStats gearNamed(int series, EquipmentRarity r);
GearStats gearQuest(int series, EquipmentRarity r);

// =====================================================================
// MONSTER POOLS, PATENTS, SPECIAL ENCOUNTERS
// =====================================================================
struct PatentSpec { int power = 0, cost = 0; };
int mobHype(int s);
int bossHype(int s);
int nemesisHype(int s, int power);
int namedHype(int s);
int safetyHype(int s);
int conferenceHype(int s);
int paperHype(int s);
int cronyHype(int s);
int minionHype(int s);
int reviewHype(int estSeries);
int reviewLevel(int startSeries, int year);
int specialItemValue(int kind, int s);                     // 0 safety, 1 conference, 2 crony
PatentSpec mobPatent(int s);
PatentSpec bossPatent(int s, bool debuff);
PatentSpec nemesisPatent(int s, int power, int kind);      // kind 0 other, 1 attack, 2 debuff
PatentSpec safetyPatent(int s, bool debuff);
PatentSpec conferencePatent(int s);
PatentSpec paperPatent(int s, bool debuff);
PatentSpec cronyPatent(int s);
int namedPatentPower(int basePower, int s);
int mobPatentChance(int difficulty);                       // 0 easy .. 3 legendary
int mobPatentCount(int difficulty);
int bossPatentCount(int difficulty);
int nemesisPatentCount(int type, int power);
int safetyPatentCount(int s, int poolSize);
int specialPatentCount(int difficulty);                    // conference / paper
double groupScale(int kind);                               // 0 small, 1 large, 2 boss minion
double cronyMult(int which);                               // 0 hp/atk/spd, 1 def, 2 money
struct ShadowStats { int hp = 0, hype = 0, atk = 0, def = 0, spd = 0; };
ShadowStats shadow(int maxRunway, int maxHype, int hustle, int ndas, int agility);
int shadowPatentPower(int playerPower);
int shadowReward(int playerMoney);
int namedStat(int baseStat, int s);
int namedMoney(int s);
int namedMinionStat(int which, int s);                     // 0 hp 1 atk 2 def 3 spd
int namedMinionMoney(int s);
int namedMinionItem(int s);

// =====================================================================
// PATENT MATH
// =====================================================================
enum class PType { Attack, Aoe, Heal, Defense, Buff, Debuff, Other };
int patentCost(PType t, int basePower);
int patentPowerIncrease(PType t, int level, int prime);
int patentBasePower(PType t, int rnd);
int attackDamage(double power, double potency, double hustle, double targetNdas);
int aoeDamage(double power, double potency, double hustle, double targetNdas);
int healAmount(double power, double potency, double synergy);
bool regenAt(int level);
int regenAmount(double power);
int regenTurns();
int defenseBoost(double power, double potency);
int defenseTurns(int level);
int buffAmount(double power, double potency);
int debuffAmount(double power, double potency);
int effectTurns(int level);                                // buff/debuff duration

// =====================================================================
// QUESTS
// =====================================================================
int questHustlesRequired(int index, int mainCount);
int questDevCount(int index);
double questMainBudget(int index, double elapsed, double total, double usable, double growth, int mainCount, double finalMinimum);
double questHustleBudget(double mainBudget, int hustlesRequired, double mainShare);
struct GoalRange { int lo = 1, hi = 1; };
GoalRange questGoalRange(int goalType, int tier);          // 0 kill 1 collect 2 boss 3 town
long long questSprintTotal(double budget, double avgSeconds, int devCount, double maxSprints);

}  // namespace GameMath

#endif
