#include "../include/Balance.h"
#include "../include/Tuning.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace Balance {

double difficultyMult(int d) {
    switch (d) {
        case 0: return T("diff.easy");
        case 1: return T("diff.medium");
        case 2: return T("diff.hard");
        case 3: return T("diff.legendary");
    }
    return 1.0;
}

EnemyStats mob(int s, double diff) {
    EnemyStats e;
    e.hp = (T("mob.hp_base") + s * T("mob.hp_per")) * diff;
    e.atk = (T("mob.atk_base") + s * T("mob.atk_per")) * diff;
    e.def = (T("mob.def_base") + s * T("mob.def_per")) * diff;
    e.spd = (T("mob.spd_base") + s * T("mob.spd_per")) * diff;
    e.money = (T("mob.money_base") + s * T("mob.money_per")) * diff;
    return e;
}

EnemyStats boss(int s, double diff) {
    EnemyStats e;
    e.hp = T("boss.hp_base") * std::pow(1.0 + s, T("boss.hp_exp")) * diff * T("boss.hp_mult");
    e.atk = (T("boss.atk_base") + s * T("boss.atk_per")) * diff * T("boss.atk_mult");
    e.def = (T("boss.def_base") + s * T("boss.def_per")) * diff * T("boss.def_mult");
    e.spd = (T("boss.spd_base") + s * T("boss.spd_per")) * diff * T("boss.spd_mult");
    e.money = (T("boss.money_base") + s * T("boss.money_per")) * diff * T("boss.money_mult");
    return e;
}

EnemyStats nemesis(int s, double elapsed, int type, int power) {
    const double tp = std::clamp(elapsed / kTotalGameSeconds, 0.0, 1.0);
    const double pressure = 1.0 / (1.0 + std::exp(-T("nemesis.time_k") * (tp - T("nemesis.time_c"))));
    const double scaling = 1.0 + pressure * T("nemesis.time_max");
    const char* tk[] = {"nemesis.type0", "nemesis.type1", "nemesis.type2", "nemesis.type3"};
    const double tm = T(tk[std::clamp(type, 0, 3)]);
    const double pm = std::pow(T("nemesis.power"), power);
    const double f = scaling * tm * pm;
    EnemyStats e;
    e.hp = (T("nemesis.hp_base") + s * T("nemesis.hp_per")) * f * T("nemesis.hp_mult");
    e.atk = (T("nemesis.atk_base") + s * T("nemesis.atk_per")) * f * T("nemesis.atk_mult");
    e.def = (T("nemesis.def_base") + s * T("nemesis.def_per")) * f * T("nemesis.def_mult");
    e.spd = (T("nemesis.spd_base") + s * T("nemesis.spd_per")) * f * T("nemesis.spd_mult");
    e.money = (T("nemesis.money_base") + s * T("nemesis.money_per")) * f * T("nemesis.money_mult");
    return e;
}

PlayerStats levelGain(int s) {
    const double n = s - 1;
    PlayerStats g;
    g.runway = T("gain.runway_base") + n * T("gain.runway_per");
    g.hype = T("gain.hype_base") + n * T("gain.hype_per");
    g.hustle = T("gain.hustle_base") + n * T("gain.hustle_per");
    g.agility = T("gain.agility_base") + n * T("gain.agility_per");
    g.ndas = T("gain.ndas_base") + n * T("gain.ndas_per");
    g.luck = T("gain.luck_base") + n * T("gain.luck_per");
    g.synergy = T("gain.synergy_base") + n * T("gain.synergy_per");
    g.techDebt = T("gain.techdebt_base") - n * T("gain.techdebt_per"); // subtracted from stat
    g.bus = T("gain.bus_base") + n * T("gain.bus_per");
    return g;
}

PlayerStats player(int series) {
    PlayerStats p;
    p.runway = T("player.runway"); p.hype = T("player.hype"); p.hustle = T("player.hustle");
    p.agility = T("player.agility"); p.ndas = T("player.ndas"); p.luck = T("player.luck");
    p.synergy = T("player.synergy"); p.techDebt = T("player.techdebt"); p.bus = T("player.bus");
    for (int s = 1; s <= series; ++s) {  // same per-step truncation as Character::addFunding
        PlayerStats g = levelGain(s);
        p.runway += static_cast<int>(g.runway);
        p.hype += static_cast<int>(g.hype);
        p.hustle += static_cast<int>(g.hustle);
        p.agility = static_cast<int>(p.agility + g.agility);
        p.ndas += static_cast<int>(g.ndas);
        p.luck = static_cast<int>(p.luck + g.luck);
        p.synergy = static_cast<int>(p.synergy + g.synergy);
        p.techDebt -= static_cast<int>(g.techDebt);
        p.bus += g.bus;
    }
    return p;
}

double fundingRequired(int series, double elapsed, double noise) {
    const double p = std::clamp(elapsed / kTotalGameSeconds, 0.0, 1.0);
    double slowdown = 1.0 / (1.0 + std::exp(T("funding.k") * (p - T("funding.c"))));
    slowdown = std::max(slowdown, T("funding.min_slowdown"));
    return T("funding.base") * std::pow(static_cast<double>(series), T("funding.exponent")) / (slowdown * noise);
}

double equipStat(int series, double mult) {
    return (series * T("equip.per_series") + T("equip.base")) * mult;
}

namespace {
struct Foe { double hp, atk, def, money; };
struct Fight { double rounds = 0, dmg = 0, money = 0; };

double turnsFor(double hp, double dmg) { return dmg <= 0 ? 1e6 : std::ceil(hp / dmg); }

Fight evalFight(std::vector<Foe> foes, double hustle, double ndas, double pm, double em) {
    std::sort(foes.begin(), foes.end(), [](const Foe& a, const Foe& b) { return a.hp < b.hp; });
    Fight f;
    double cum = 0;
    for (const auto& e : foes) {
        double pd = std::max(1.0, (hustle - e.def) * pm);
        double ed = std::max(1.0, (e.atk - ndas) * em);
        cum += turnsFor(e.hp, pd);
        f.dmg += ed * std::max(0.0, cum - 1);   // it attacks every round until killed
        f.money += e.money;
    }
    f.rounds = cum;
    return f;
}
Foe scaled(const EnemyStats& e, double k) { return {e.hp * k, e.atk * k, e.def * k, e.money * k}; }
}  // namespace

void modelGear(int s, double& gh, double& gn) {
    const double f = T("proj.gear_fraction");
    gh = equipStat(s, T("equip.laptop_hustle") + T("equip.gloves_each")) * f;
    gn = equipStat(s, T("equip.laptop_ndas") + T("equip.helmet_ndas")) * f;
}

SeriesMetrics seriesMetrics(int s, double elapsed, double gH, double gN) {
    SeriesMetrics m;
    m.series = s;
    m.player = player(s);
    m.hustle = m.player.hustle + gH;
    m.ndas = m.player.ndas + gN;
    m.runway = m.player.runway;
    const double pm = T("combat.player_dmg_mult"), em = T("combat.enemy_dmg_mult");
    const int yr = static_cast<int>(elapsed / (365.0 * 86400.0)) + 1;
    const int ntype = yr <= 5 ? 0 : yr <= 10 ? 1 : yr <= 15 ? 2 : 3;
    m.mob = mob(s, difficultyMult(1));
    m.boss = boss(s, difficultyMult(1));
    m.nemesis = nemesis(s, elapsed, ntype, 0);
    auto hits = [&](const EnemyStats& e, double& mine, double& theirs) {
        mine = turnsFor(e.hp, std::max(1.0, (m.hustle - e.def) * pm));
        theirs = turnsFor(m.runway, std::max(1.0, (e.atk - m.ndas) * em));
    };
    hits(m.mob, m.hitsMob, m.survMob);
    hits(m.boss, m.hitsBoss, m.survBoss);
    hits(m.nemesis, m.hitsNemesis, m.survNemesis);

    // encounter mix x difficulty mix
    const double pe = T("diff.pct_easy"), pmid = T("diff.pct_medium"), ph = T("diff.pct_hard");
    const double plg = std::max(0.0, 100.0 - pe - pmid - ph);
    const double dw[4] = {pe, pmid, ph, plg}, dsum = std::max(1e-9, pe + pmid + ph + plg);
    const double single = T("enc.single_pct"), small = T("enc.small_pct"), large = T("enc.large_pct");
    const double bossP = std::max(0.0, 100.0 - single - small - large), esum = std::max(1e-9, single + small + large + bossP);
    const double sk = T("enc.small_scale"), lk = T("enc.large_scale"), mk = T("enc.minion_scale");

    double rounds = 0, dmg = 0, money = 0;
    for (int d = 0; d < 4; ++d) {
        const double w = dw[d] / dsum;
        if (w <= 0) continue;
        const EnemyStats mb = mob(s, difficultyMult(d)), bs = boss(s, difficultyMult(d));
        auto add = [&](double p, const Fight& f) { rounds += w * p * f.rounds; dmg += w * p * f.dmg; money += w * p * f.money; };
        auto grp = [&](int n, double k) { std::vector<Foe> v(n, scaled(mb, k)); return evalFight(v, m.hustle, m.ndas, pm, em); };
        add(single / esum, grp(1, 1.0));
        Fight g2 = grp(2, sk), g3 = grp(3, sk);
        add(small / esum, Fight{(g2.rounds + g3.rounds) / 2, (g2.dmg + g3.dmg) / 2, (g2.money + g3.money) / 2});
        add(large / esum, grp(4, lk));
        for (int mn = 2; mn <= 3; ++mn) {
            std::vector<Foe> v = {scaled(bs, 1.0)};
            for (int i = 0; i < mn; ++i) v.push_back(scaled(mb, mk));
            Fight f = evalFight(v, m.hustle, m.ndas, pm, em);
            add(bossP / esum / 2, f);
        }
    }
    const double tick = 0.1;
    m.secondsPerEncounter = T("time.battle_start_delay") + T("time.ticks_between_battles") * tick +
                            rounds * 2.0 * T("time.ticks_per_action") * tick;
    m.fundingPerEncounter = money * T("funding.per_money");
    m.dmgPerEncounter = dmg;
    m.encountersPerLife = dmg > 1e-9 ? m.runway / dmg : 1e9;
    const double rate = m.fundingPerEncounter / std::max(1e-9, m.secondsPerEncounter);
    m.secondsToNextSeries = rate > 1e-12 ? fundingRequired(s + 1, elapsed) / rate : 1e12;
    return m;
}

Projection project() {
    Projection p;
    const int kMaxSeries = 400;
    double t = 0;
    int s = 1;
    for (int year = 0; year <= 20; ++year) {
        const double yearStart = year * 365.0 * 86400.0;
        while (t < yearStart && s < kMaxSeries) {  // advance to this year boundary
            double gh, gn; modelGear(s, gh, gn);
            t += seriesMetrics(s, t, gh, gn).secondsToNextSeries;
            if (t <= yearStart) ++s; else { t = yearStart; break; }
        }
        double gh, gn; modelGear(s, gh, gn);
        p.atYear.push_back(seriesMetrics(s, yearStart, gh, gn));
        p.seriesAtYear.push_back(s);
    }
    return p;
}

}  // namespace Balance
