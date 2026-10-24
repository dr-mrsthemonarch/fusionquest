#ifndef BALANCE_H
#define BALANCE_H

// pure balance formulas. read factors from Tuning (T("key")). used by the game AND the debug tuner,
// so what the tuner shows is what the game computes. no randomness here (callers add variance/noise).
#include <vector>

namespace Balance {

struct EnemyStats { double hp = 0, atk = 0, def = 0, spd = 0, money = 0; };
struct PlayerStats {
    double runway = 0, hype = 0, hustle = 0, agility = 0, ndas = 0, luck = 0, synergy = 0, techDebt = 0, bus = 0;
};

constexpr double kTotalGameSeconds = 20.0 * 365 * 24 * 3600;

double difficultyMult(int difficulty);                    // 0 easy,1 medium,2 hard,3 legendary
EnemyStats mob(int series, double diffMult);
EnemyStats boss(int series, double diffMult);
EnemyStats nemesis(int series, double elapsedSeconds, int type, int power);  // type 0..3

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

}  // namespace Balance

#endif
