#include "../include/GameMath.h"
#include "../include/Tuning.h"
#include "../include/Archetype.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <string>

namespace GameMath {

namespace {
int g_class = -1;
// growth curves: base + lin*x + quad*x^2. quad keys default 0 (balance knob).
const Curve MOB_HP{"mob.hp_base", "mob.hp_per", "mob.hp_quad"}, MOB_ATK{"mob.atk_base", "mob.atk_per", "mob.atk_quad"},
    MOB_DEF{"mob.def_base", "mob.def_per", "mob.def_quad"}, MOB_SPD{"mob.spd_base", "mob.spd_per", "mob.spd_quad"},
    MOB_MONEY{"mob.money_base", "mob.money_per", "mob.money_quad"};
const Curve BOSS_ATK{"boss.atk_base", "boss.atk_per", "boss.atk_quad"}, BOSS_DEF{"boss.def_base", "boss.def_per", "boss.def_quad"},
    BOSS_SPD{"boss.spd_base", "boss.spd_per", "boss.spd_quad"}, BOSS_MONEY{"boss.money_base", "boss.money_per", "boss.money_quad"};
const Curve NEM_HP{"nemesis.hp_base", "nemesis.hp_per", "nemesis.hp_quad"}, NEM_ATK{"nemesis.atk_base", "nemesis.atk_per", "nemesis.atk_quad"},
    NEM_DEF{"nemesis.def_base", "nemesis.def_per", "nemesis.def_quad"}, NEM_SPD{"nemesis.spd_base", "nemesis.spd_per", "nemesis.spd_quad"},
    NEM_MONEY{"nemesis.money_base", "nemesis.money_per", "nemesis.money_quad"};
const Curve SP_HP{"special.hp_base", "special.hp_per", "special.hp_quad"}, SP_ATK{"special.atk_base", "special.atk_per", "special.atk_quad"},
    SP_DEF{"special.def_base", "special.def_per", "special.def_quad"}, SP_SPD{"special.spd_base", "special.spd_per", "special.spd_quad"},
    SP_MONEY{"special.money_base", "special.money_per", "special.money_quad"};
const Curve G_RUNWAY{"gain.runway_base", "gain.runway_per", "gain.runway_quad"}, G_HYPE{"gain.hype_base", "gain.hype_per", "gain.hype_quad"},
    G_HUSTLE{"gain.hustle_base", "gain.hustle_per", "gain.hustle_quad"}, G_AGI{"gain.agility_base", "gain.agility_per", "gain.agility_quad"},
    G_NDAS{"gain.ndas_base", "gain.ndas_per", "gain.ndas_quad"}, G_LUCK{"gain.luck_base", "gain.luck_per", "gain.luck_quad"},
    G_SYN{"gain.synergy_base", "gain.synergy_per", "gain.synergy_quad"};
const Curve EQUIP{"equip.base", "equip.per_series", "equip.quad"};
}  // namespace
void setProjectionClass(int id) { g_class = id; }
std::string projectionClassName() { return g_class < 0 ? "none" : Archetypes::I().get(g_class).name; }

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
    e.hp = eval(MOB_HP, s) * diff;
    e.atk = eval(MOB_ATK, s) * diff;
    e.def = eval(MOB_DEF, s) * diff;
    e.spd = eval(MOB_SPD, s) * diff;
    e.money = eval(MOB_MONEY, s) * diff;
    return e;
}

EnemyStats boss(int s, double diff) {
    EnemyStats e;
    e.hp = T("boss.hp_base") * std::pow(1.0 + s, T("boss.hp_exp")) * diff * T("boss.hp_mult");
    e.atk = eval(BOSS_ATK, s) * diff * T("boss.atk_mult");
    e.def = eval(BOSS_DEF, s) * diff * T("boss.def_mult");
    e.spd = eval(BOSS_SPD, s) * diff * T("boss.spd_mult");
    e.money = eval(BOSS_MONEY, s) * diff * T("boss.money_mult");
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
    e.hp = eval(NEM_HP, s) * f * T("nemesis.hp_mult");
    e.atk = eval(NEM_ATK, s) * f * T("nemesis.atk_mult");
    e.def = eval(NEM_DEF, s) * f * T("nemesis.def_mult");
    e.spd = eval(NEM_SPD, s) * f * T("nemesis.spd_mult");
    e.money = eval(NEM_MONEY, s) * f * T("nemesis.money_mult");
    return e;
}

EnemyStats special(int kind, int s, double diff) {
    const double hp = eval(SP_HP, s);
    const double atk = eval(SP_ATK, s);
    const double def = eval(SP_DEF, s);
    const double spd = eval(SP_SPD, s);
    const double mon = eval(SP_MONEY, s);
    EnemyStats e;
    switch (kind) {
        case 0:  // safety manager ignores difficulty
            e = {hp * T("special.safety_hp"), atk * T("special.safety_atk"), def * T("special.safety_def"),
                 spd * T("special.safety_spd"), mon * T("special.safety_money")};
            break;
        case 1:
            e = {hp * T("special.conf_hp") * diff, atk * T("special.conf_atk") * diff, def * T("special.conf_def") * diff,
                 spd * T("special.conf_spd") * diff, mon * T("special.conf_money") * diff};
            break;
        case 2:
            e = {hp * T("special.paper_hp") * diff, atk * T("special.paper_atk") * diff, def * T("special.paper_def") * diff,
                 spd * T("special.paper_spd") * diff, mon * T("special.paper_money") * diff};
            break;
        default: e = {hp, atk, def, spd, mon};
    }
    return e;
}

EnemyStats review(int year, int month, double diff) {
    const double k = (1.0 + year * T("review.year_growth")) * (1.0 + (month / 12.0) * T("review.month_growth")) * diff;
    return {T("review.hp") * k, T("review.atk") * k, T("review.def") * k, T("review.spd") * k, T("review.money") * k};
}

PlayerStats levelGain(int s) {
    const double n = s - 1;
    PlayerStats g;
    g.runway = eval(G_RUNWAY, n);
    g.hype = eval(G_HYPE, n);
    g.hustle = eval(G_HUSTLE, n);
    g.agility = eval(G_AGI, n);
    g.ndas = eval(G_NDAS, n);
    g.luck = eval(G_LUCK, n);
    g.synergy = eval(G_SYN, n);
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
    if (g_class >= 0) {
        const ClassDef& c = Archetypes::I().get(g_class);
        const double ms = T("class.mod_scale"), gs = T("class.grow_scale");
        auto ap = [&](double& v, double m, double g) { v += m * ms + g * gs * series; };
        ap(p.runway, c.mods.runway, c.grow.runway); ap(p.hype, c.mods.hype, c.grow.hype);
        ap(p.hustle, c.mods.hustle, c.grow.hustle); ap(p.agility, c.mods.agility, c.grow.agility);
        ap(p.ndas, c.mods.ndas, c.grow.ndas); ap(p.luck, c.mods.luck, c.grow.luck);
        ap(p.synergy, c.mods.synergy, c.grow.synergy);
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
    return eval(EQUIP, series) * mult;
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
    // expected stat effects folded into damage multipliers
    const double cc = critChance(m.player.luck), cm = critMult(m.player.luck, m.player.synergy);
    const double pm = T("combat.player_dmg_mult") * (1.0 + cc * (cm - 1.0)) * (1.0 - failChance(m.player.techDebt));
    const double em = T("combat.enemy_dmg_mult") * (1.0 - dodgeChance(m.player.agility, mob(s, 1.0).spd));
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

bool chance(double p) {
    static std::mt19937 g(std::random_device{}());
    if (p <= 0.0) return false;
    return std::uniform_real_distribution<double>(0.0, 1.0)(g) < p;
}
double critChance(double luck) {
    return std::clamp(T("crit.base") + std::max(0.0, luck) * T("crit.per_luck"), 0.0, T("crit.cap"));
}
double critMult(double luck, double synergy, double bonus) {
    (void)luck;
    return std::min(T("crit.mult_cap"), T("crit.mult_base") + std::max(0.0, synergy) * T("crit.mult_per_syn") + bonus);
}
double dodgeChance(double def, double atk) {
    const double adv = (def - atk) / std::max(1.0, def + atk);
    return std::clamp(adv * T("dodge.edge"), 0.0, T("dodge.cap"));
}
double extraActionChance(double agi, double foe, double bonus) {
    const double lead = agi / std::max(1.0, foe);
    return std::clamp((lead - 1.0) * T("extra.per_lead"), 0.0, T("extra.cap")) + bonus;
}
double failChance(double td) { return std::clamp(std::max(0.0, -td) * T("fail.per_debt"), 0.0, T("fail.cap")); }
double breakChance(double td, double resist) {
    return std::clamp(std::max(0.0, -td) * T("break.per_debt"), 0.0, T("break.cap")) * (1.0 - std::clamp(resist, 0.0, 1.0));
}
double patentPotency(double syn) { return 1.0 + std::min(T("syn.potency_cap") - 1.0, std::max(0.0, syn) * T("syn.potency_per")); }
double hypeDiscount(double syn, double bonus) {
    return std::max(0.2, 1.0 - std::min(T("syn.hype_cap"), std::max(0.0, syn) * T("syn.hype_per")) - bonus);
}
double luckRarityBonus(double luck) { return std::min(T("luck.rarity_cap"), std::max(0.0, luck) * T("luck.rarity_per")); }
double luckItemMult(double luck, double bonus) { return 1.0 + std::min(T("luck.item_cap"), std::max(0.0, luck) * T("luck.item_per")) + bonus; }


// =====================================================================
// SHARED HELPERS
// =====================================================================
double eval(const Curve& c, double x) {
    double v = T(c.base) + T(c.lin) * x;
    if (c.quad) v += T(c.quad) * x * x;
    return v;
}
double dv(const char* key) { return std::max(0.01, T(key)); }

// =====================================================================
// COMBAT / ECONOMY
// =====================================================================
int combatVariance() { return static_cast<int>(T("combat.variance")); }
int playerDamage(double atk, double def, int noise) {
    return std::max(1, static_cast<int>((atk - def) * T("combat.player_dmg_mult")) + noise);
}
int enemyDamage(double atk, double def, int noise) {
    return std::max(1, static_cast<int>((atk - def) * T("combat.enemy_dmg_mult")) + noise);
}
int fundingFromMoney(int money) { return static_cast<int>(money * T("funding.per_money")); }
bool hypeRecoveryNeeded(int cur, int maxHype) { return cur < maxHype * T("hype.low_threshold"); }
int hypeRecoveryCost(int money) { return static_cast<int>(money * T("hype.cost_pct")); }
int hypeRecoveryAmount(int maxHype) { return static_cast<int>(maxHype * T("hype.recover_frac")); }
bool emergencyHealNeeded(int runway, int maxRunway) { return runway < maxRunway * T("emerg.heal_frac"); }
bool emergencySaleNeeded(int runway) { return runway < T("emerg.sale_runway"); }
int emergencyRunwayGain(int earned) { return std::max(0, static_cast<int>(earned / std::max(1.0, T("emerg.runway_money_div")))); }
double sellRatio(double synergy) {
    return T("econ.sell_ratio") + std::min(T("syn.sell_cap"), std::max(0.0, synergy) * T("syn.sell_per"));
}
int repairCost(int itemValue, double classMult) {
    return std::max(1, static_cast<int>(itemValue * T("econ.repair_pct") * classMult));
}
double questRewardFunding(double required, bool main) {
    return required * (main ? T("quest.reward_main_pct") : T("quest.reward_hustle_pct")) / 100.0;
}
EquipmentRarity questRewardRarity(bool main, int roll) {
    const bool hi = roll < static_cast<int>(T("quest.reward_hi_pct"));
    return main ? (hi ? EquipmentRarity::LEGENDARY : EquipmentRarity::EPIC) : (hi ? EquipmentRarity::EPIC : EquipmentRarity::RARE);
}
int itemBaseRange() { return std::max(1, static_cast<int>(T("item.base_range"))); }
int itemStyleBonus(int style) {
    int b = 0;
    if (style >= 50) b += static_cast<int>(T("item.bonus_fancy"));
    if (style >= 80) b += static_cast<int>(T("item.bonus_epic"));
    if (style >= 95) b += static_cast<int>(T("item.bonus_corporate"));
    return b;
}

// =====================================================================
// EQUIPMENT
// =====================================================================
double equipVariance() { return T("equip.variance"); }
int backpackNoiseRange() { return std::max(0, static_cast<int>(T("equip.backpack_var"))); }
int backpackSlots(int series, int noise) {
    const int base = static_cast<int>(T("equip.backpack_base") + series * T("equip.backpack_per"));
    return std::max(static_cast<int>(T("equip.backpack_min")), base + noise);
}
double rarityStatMult(EquipmentRarity r) {
    switch (r) {
        case EquipmentRarity::UNCOMMON: return T("rarity.stat_uncommon");
        case EquipmentRarity::RARE: return T("rarity.stat_rare");
        case EquipmentRarity::EPIC: return T("rarity.stat_epic");
        case EquipmentRarity::LEGENDARY: return T("rarity.stat_legendary");
        default: return 1.0;
    }
}
double rarityValueMult(EquipmentRarity r) {
    switch (r) {
        case EquipmentRarity::UNCOMMON: return T("rarity.val_uncommon");
        case EquipmentRarity::RARE: return T("rarity.val_rare");
        case EquipmentRarity::EPIC: return T("rarity.val_epic");
        case EquipmentRarity::LEGENDARY: return T("rarity.val_legendary");
        default: return 1.0;
    }
}
EquipmentRarity bossRarity(int roll) {
    if (roll <= T("rarity.cum_legendary")) return EquipmentRarity::LEGENDARY;
    if (roll <= T("rarity.cum_epic")) return EquipmentRarity::EPIC;
    if (roll <= T("rarity.cum_rare")) return EquipmentRarity::RARE;
    if (roll <= T("rarity.cum_uncommon")) return EquipmentRarity::UNCOMMON;
    return EquipmentRarity::COMMON;
}
double equipValue(const Equipment& q) {
    double v = 0.0;
    v += q.runway * T("equip.val_runway");
    v += q.hypeEnergy * T("equip.val_hype");
    v += q.hustle * T("equip.val_hustle");
    v += q.agility * T("equip.val_agility");
    v += q.ndas * T("equip.val_ndas");
    v += q.luck * T("equip.val_luck");
    v += q.synergy * T("equip.val_synergy");
    v += q.technicalDebt * T("equip.val_td");
    v += q.busFactor * T("equip.val_bus");
    v += q.inventoryLimit * T("equip.val_inv");
    return std::max(T("equip.val_min"), v * rarityValueMult(q.rarity));
}

namespace {
double gk(const std::string& kind, const std::string& stat) { return T(("gear." + kind + "." + stat).c_str()); }
// stat = (base + per*series) * statMult. keys gear.<kind>.<stat>_base / _per
GearStats gearBuild(const std::string& kind, int s, double sm, double vm, double im) {
    auto st = [&](const char* n) { return (gk(kind, std::string(n) + "_base") + gk(kind, std::string(n) + "_per") * s) * sm; };
    GearStats g;
    g.runway = static_cast<int>(st("runway")); g.hype = static_cast<int>(st("hype"));
    g.hustle = static_cast<int>(st("hustle")); g.agility = static_cast<int>(st("agility"));
    g.ndas = static_cast<int>(st("ndas")); g.luck = static_cast<int>(st("luck"));
    g.synergy = static_cast<int>(st("synergy")); g.techDebt = static_cast<int>(st("td"));
    g.bus = static_cast<float>(gk(kind, "bus") * sm);
    g.value = static_cast<int>(gk(kind, "value") * vm);
    g.inventoryLimit = static_cast<int>(gk(kind, "inv") * im);
    return g;
}
}  // namespace

GearStats gearNemesis(int s, int power) {
    const double m = 1.0 + power * gk("nemesis", "power_mult");
    return gearBuild("nemesis", s, m, m, 1.0);
}
GearStats gearShadow(int s) { return gearBuild("shadow", s, 1, 1, 1); }
GearStats gearSafety(int s) { return gearBuild("safety", s, 1, 1, 1); }
GearStats gearConference(int s) { return gearBuild("conf", s, 1, 1, 1); }
GearStats gearPaper(int s) { return gearBuild("paper", s, 1, 1, 1); }
GearStats gearReview(int year, int s) {
    const double m = 1.0 + year * gk("review", "year_mult");
    GearStats g = gearBuild("review", s, m, m, 1.0);
    g.inventoryLimit = static_cast<int>(gk("review", "inv_base") + gk("review", "inv_per") * year);
    return g;
}
GearStats gearNamed(int s, EquipmentRarity r) { const double m = rarityStatMult(r); return gearBuild("named", s, m, 1.0, m); }
GearStats gearQuest(int s, EquipmentRarity r) { const double m = rarityStatMult(r); return gearBuild("quest", s, m, m, m); }

// =====================================================================
// MONSTER POOLS, PATENTS, SPECIAL ENCOUNTERS
// =====================================================================
namespace {
const Curve H_SAFETY{"hype.safety_base", "hype.safety_per"}, H_CONF{"hype.conf_base", "hype.conf_per"},
    H_PAPER{"hype.paper_base", "hype.paper_per"}, H_CRONY{"hype.crony_base", "hype.crony_per"},
    H_MINION{"hype.minion_base", "hype.minion_per"}, H_REVIEW{"hype.review_base", "hype.review_per"};
const Curve I_SAFETY{"item.safety_base", "item.safety_per"}, I_CONF{"item.conf_base", "item.conf_per"},
    I_CRONY{"item.crony_base", "item.crony_per"};
const Curve P_SAFETY_POW{"pat.safety_pow_base", "pat.safety_pow_per"}, P_SAFETY_COST{"pat.safety_cost_base", "pat.safety_cost_per"},
    P_CONF_POW{"pat.conf_pow_base", "pat.conf_pow_per"}, P_CONF_COST{"pat.conf_cost_base", "pat.conf_cost_per"},
    P_PAPER_POW{"pat.paper_pow_base", "pat.paper_pow_per"}, P_PAPER_COST{"pat.paper_cost_base", "pat.paper_cost_per"},
    P_CRONY_POW{"pat.crony_pow_base", "pat.crony_pow_per"}, P_CRONY_COST{"pat.crony_cost_base", "pat.crony_cost_per"};
const Curve N_MINION_MONEY{"named.minion_money_base", "named.minion_money_per"}, N_MINION_ITEM{"named.minion_item_base", "named.minion_item_per"};
const char* kDiff[4] = {"easy", "medium", "hard", "legendary"};
}  // namespace

int mobHype(int s) { return static_cast<int>(T("monster.mob_hype_base") + s * T("monster.mob_hype_per")); }
int bossHype(int s) { return static_cast<int>(T("monster.boss_hype_base") + s * T("monster.boss_hype_per")); }
int nemesisHype(int s, int power) { return static_cast<int>(T("nemesis.hype_base") + s * T("nemesis.hype_per") + power * T("nemesis.hype_pow")); }
int namedHype(int s) { return static_cast<int>(T("named.hype_base") + s * T("named.hype_per")); }
int safetyHype(int s) { return static_cast<int>(eval(H_SAFETY, s)); }
int conferenceHype(int s) { return static_cast<int>(eval(H_CONF, s)); }
int paperHype(int s) { return static_cast<int>(eval(H_PAPER, s)); }
int cronyHype(int s) { return static_cast<int>(eval(H_CRONY, s)); }
int minionHype(int s) { return static_cast<int>(eval(H_MINION, s)); }
int reviewHype(int est) { return static_cast<int>(eval(H_REVIEW, est)); }
int reviewLevel(int startSeries, int year) { return startSeries + (year - 1) + year; }
int specialItemValue(int kind, int s) {
    switch (kind) {
        case 0: return static_cast<int>(eval(I_SAFETY, s));
        case 1: return static_cast<int>(eval(I_CONF, s));
        case 2: return static_cast<int>(eval(I_CRONY, s));
        default: return -static_cast<int>(T("special.paper_item_base") + s * T("special.paper_item_per"));  // paper: costs money
    }
}
PatentSpec mobPatent(int s) {
    return {static_cast<int>(T("monster.mob_patent_pow") + s * T("monster.mob_patent_pow_per")),
            static_cast<int>(T("monster.mob_patent_cost") + s * T("monster.mob_patent_cost_per"))};
}
PatentSpec bossPatent(int s, bool debuff) {
    PatentSpec p{static_cast<int>(T("monster.boss_patent_pow") + s * T("monster.boss_patent_pow_per")),
                 static_cast<int>(T("monster.boss_patent_cost") + s * T("monster.boss_patent_cost_per"))};
    if (debuff) p.power = static_cast<int>(p.power * T("monster.debuff_mult"));
    return p;
}
PatentSpec nemesisPatent(int s, int power, int kind) {
    PatentSpec p{static_cast<int>(T("nemesis.patent_pow_base") + s * T("nemesis.patent_pow_per") + power * T("nemesis.patent_pow_pow")),
                 static_cast<int>(T("nemesis.patent_cost_base") + s * T("nemesis.patent_cost_per"))};
    if (kind == 2) p.power = static_cast<int>(p.power * T("nemesis.patent_debuff_mult"));
    else if (kind == 1) p.power = static_cast<int>(p.power * T("nemesis.patent_attack_mult"));
    return p;
}
PatentSpec safetyPatent(int s, bool debuff) {
    PatentSpec p{static_cast<int>(eval(P_SAFETY_POW, s)), static_cast<int>(eval(P_SAFETY_COST, s))};
    if (debuff) p.power = static_cast<int>(p.power * T("pat.safety_debuff_mult"));
    return p;
}
PatentSpec conferencePatent(int s) { return {static_cast<int>(eval(P_CONF_POW, s)), static_cast<int>(eval(P_CONF_COST, s))}; }
PatentSpec paperPatent(int s, bool debuff) {
    PatentSpec p{static_cast<int>(eval(P_PAPER_POW, s)), static_cast<int>(eval(P_PAPER_COST, s))};
    if (debuff) p.power = static_cast<int>(p.power * T("pat.paper_debuff_mult"));
    return p;
}
PatentSpec cronyPatent(int s) { return {static_cast<int>(eval(P_CRONY_POW, s)), static_cast<int>(eval(P_CRONY_COST, s))}; }
int namedPatentPower(int base, int s) { return static_cast<int>(base + s * T("named.patent_pow_per")); }
int mobPatentChance(int d) { return static_cast<int>(T((std::string("monster.mob_patent_") + kDiff[std::clamp(d, 0, 3)]).c_str())); }
int mobPatentCount(int d) { return static_cast<int>(T((std::string("monster.mob_count_") + kDiff[std::clamp(d, 0, 3)]).c_str())); }
int bossPatentCount(int d) { return static_cast<int>(T((std::string("monster.boss_count_") + kDiff[std::clamp(d, 0, 3)]).c_str())); }
int nemesisPatentCount(int type, int power) {
    return std::min(static_cast<int>(T("pat.nemesis_count_cap")),
                    static_cast<int>(T("pat.nemesis_count_base")) + type + power / std::max(1, static_cast<int>(T("pat.nemesis_count_power_div"))));
}
int safetyPatentCount(int s, int pool) {
    return std::min(pool, static_cast<int>(T("pat.safety_count_base")) + s / std::max(1, static_cast<int>(T("pat.safety_count_div"))));
}
int specialPatentCount(int d) {
    int n = static_cast<int>(T("pat.conf_count_base"));
    if (d == 2) n += static_cast<int>(T("pat.count_hard"));
    if (d == 3) n += static_cast<int>(T("pat.count_legendary"));
    return n;
}
double groupScale(int kind) { return T(kind == 0 ? "enc.small_scale" : kind == 1 ? "enc.large_scale" : "enc.minion_scale"); }
double cronyMult(int which) { return T(which == 0 ? "crony.mult_main" : which == 1 ? "crony.mult_def" : "crony.mult_money"); }
ShadowStats shadow(int maxRunway, int maxHype, int hustle, int ndas, int agility) {
    ShadowStats s;
    s.hp = static_cast<int>(maxRunway * T("shadow.hp_mult"));
    s.hype = static_cast<int>(maxHype * T("shadow.hype_mult"));
    s.atk = static_cast<int>(hustle * T("shadow.atk_mult"));
    s.def = static_cast<int>(ndas * T("shadow.def_mult"));
    s.spd = static_cast<int>(agility * T("shadow.spd_mult"));
    return s;
}
int shadowPatentPower(int power) { return static_cast<int>(power * T("shadow.patent_mult")); }
int shadowReward(int money) { return static_cast<int>(std::min(T("shadow.reward_flat") + money * T("shadow.reward_money_mult"), 2.0e9)); }
int namedStat(int base, int s) {
    const double growth = base * T("named.growth_per_series");
    return static_cast<int>((base + s * growth) * T("named.power_mult"));
}
int namedMoney(int s) { return static_cast<int>(T("named.money_base") + s * T("named.money_per")); }
int namedMinionStat(int which, int s) {
    static const char* k[4] = {"named.minion_hp", "named.minion_atk", "named.minion_def", "named.minion_spd"};
    return namedStat(static_cast<int>(T(k[std::clamp(which, 0, 3)])), s);
}
int namedMinionMoney(int s) { return static_cast<int>(eval(N_MINION_MONEY, s)); }
int namedMinionItem(int s) { return static_cast<int>(eval(N_MINION_ITEM, s)); }

// =====================================================================
// PATENT MATH
// =====================================================================
int patentCost(PType t, int b) {
    switch (t) {
        case PType::Attack: return static_cast<int>(T("patent.attack_cost_base") + b / dv("patent.attack_cost_div"));
        case PType::Aoe: return static_cast<int>((T("patent.aoe_cost_base") + b / dv("patent.aoe_cost_div")) * T("patent.aoe_cost_mult"));
        case PType::Heal: return static_cast<int>(T("patent.heal_cost_base") + b / dv("patent.heal_cost_div"));
        case PType::Defense: return static_cast<int>(T("patent.defense_cost_base") + b / dv("patent.defense_cost_div"));
        case PType::Buff:
        case PType::Debuff: return static_cast<int>(T("patent.bd_cost_base") + b / dv("patent.bd_cost_div"));
        default: return 10;
    }
}
int patentPowerIncrease(PType t, int lvl, int prime) {
    switch (t) {
        case PType::Attack: return static_cast<int>(T("patent.attack_pwr_base") + lvl * T("patent.attack_pwr_lvl") + prime / dv("patent.attack_pwr_pdiv"));
        case PType::Defense: return static_cast<int>(T("patent.defense_pwr_base") + lvl * T("patent.defense_pwr_lvl") + prime / dv("patent.defense_pwr_pdiv"));
        case PType::Buff:
        case PType::Debuff: return static_cast<int>(T("patent.bd_pwr_base") + lvl * T("patent.bd_pwr_lvl") + prime / dv("patent.bd_pwr_pdiv"));
        default: return static_cast<int>(T("patent.heal_pwr_base") + lvl * T("patent.heal_pwr_lvl") + prime / dv("patent.heal_pwr_pdiv"));
    }
}
int patentBasePower(PType t, int rnd) {
    auto roll = [rnd](const char* start, const char* range) {
        return static_cast<int>(T(start)) + std::abs(rnd) % std::max(1, static_cast<int>(T(range)));
    };
    switch (t) {
        case PType::Attack: return roll("patent.attack_start", "patent.attack_range");
        case PType::Heal: return roll("patent.heal_start", "patent.heal_range");
        case PType::Defense: return roll("patent.defense_start", "patent.defense_range");
        case PType::Buff:
        case PType::Debuff: return roll("patent.bd_start", "patent.bd_range");
        default: return 10;
    }
}
int attackDamage(double power, double potency, double hustle, double ndas) {
    const int d = static_cast<int>(power * potency) + static_cast<int>(hustle / dv("patent.attack_hustle_div"));
    return std::max(1, d - static_cast<int>(ndas / dv("patent.attack_def_div")));
}
int aoeDamage(double power, double potency, double hustle, double ndas) {
    const int d = static_cast<int>(power * potency) + static_cast<int>(hustle / dv("patent.aoe_hustle_div"));
    return std::max(1, d - static_cast<int>(ndas / dv("patent.aoe_def_div")));
}
int healAmount(double power, double potency, double synergy) {
    return static_cast<int>(power * potency) + static_cast<int>(synergy / dv("patent.heal_synergy_div"));
}
bool regenAt(int level) { return level >= static_cast<int>(T("patent.regen_level")); }
int regenAmount(double power) { return static_cast<int>(power / dv("patent.regen_div")); }
int regenTurns() { return static_cast<int>(T("patent.regen_turns")); }
int defenseBoost(double power, double potency) { return static_cast<int>(power * potency / dv("patent.defense_div")); }
int defenseTurns(int level) { return static_cast<int>(T("patent.defense_turns_base") + level / dv("patent.defense_turns_div")); }
int buffAmount(double power, double potency) { return static_cast<int>(power * potency / dv("patent.buff_div")); }
int debuffAmount(double power, double potency) { return static_cast<int>(power * potency / dv("patent.debuff_div")); }
int effectTurns(int level) { return static_cast<int>(T("patent.buff_turns_base") + level / dv("patent.buff_turns_div")); }

// =====================================================================
// QUESTS
// =====================================================================
int questHustlesRequired(int index, int mainCount) {
    return index > mainCount ? 0 : static_cast<int>(T("quest.hustles_base") + index * T("quest.hustles_per_index"));
}
int questDevCount(int index) { return static_cast<int>(T("quest.devs_base")) + index / std::max(1, static_cast<int>(T("quest.devs_div"))); }
double questMainBudget(int index, double elapsed, double total, double usable, double growth, int n, double finalMin) {
    if (index > n) return std::max(finalMin, total - elapsed);      // final quest: rest of the clock
    const double remaining = std::max(0.0, total * usable - elapsed);
    double sumW = 0.0;
    for (int j = index; j <= n; ++j) sumW += 1.0 + growth * j;
    const double w = 1.0 + growth * index;
    return std::max(3600.0, remaining * w / std::max(sumW, 1e-9));
}
double questHustleBudget(double mainBudget, int k, double share) { return k > 0 ? mainBudget * (1.0 - share) / k : mainBudget * 0.1; }
GoalRange questGoalRange(int type, int tier) {
    GoalRange g;
    auto fill = [&](const char* lo, const char* hiBase, const char* hiDiv) {
        g.lo = static_cast<int>(T(lo));
        g.hi = std::max(g.lo, static_cast<int>(T(hiBase)) + tier / std::max(1, static_cast<int>(T(hiDiv))));
    };
    switch (type) {
        case 0: fill("quest.kill_lo", "quest.kill_hi_base", "quest.kill_hi_div"); break;
        case 1: fill("quest.collect_lo", "quest.collect_hi_base", "quest.collect_hi_div"); break;
        case 2: fill("quest.boss_lo", "quest.boss_hi_base", "quest.boss_hi_div"); break;
        default: fill("quest.town_lo", "quest.town_hi_base", "quest.town_hi_div"); break;
    }
    return g;
}
long long questSprintTotal(double budget, double avg, int devCount, double maxSprints) {
    long long total = std::llround(budget / std::max(avg, 1.0));
    return std::clamp<long long>(total, devCount * static_cast<long long>(T("quest.min_sprints_per_dev")), static_cast<long long>(maxSprints));
}

}  // namespace GameMath
