#include "../include/PlayerStrategy.h"
#include "../include/Archetype.h"
#include "../include/GameMath.h"
#include "../include/BattleEncounter.h"
#include "../include/Character.h"
#include "../include/Patent.h"
#include "../include/Tuning.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <iostream>

namespace PlayerStrategy {

PlayerChoice choose(Character& pc, BattleEncounter& enc, std::mt19937& rng) {
    PlayerChoice best;
    auto alive = enc.getAliveEnemies();
    Monster* low = enc.getLowestHPEnemy();
    if (alive.empty() || !low || !pc.canUsePatents()) return best;

    const ClassDef& cls = Archetypes::I().get(pc.getClassId());
    const double hustle = pc.getModifiedHustle(), ndas = pc.getModifiedNDAs();
    const double pm = T("combat.player_dmg_mult"), em = T("combat.enemy_dmg_mult");
    const double runway = std::max(1, pc.getRunway()), maxRunway = std::max(1, pc.getMaxRunway());
    const double hype = pc.getHypeEnergy(), maxHype = std::max(1, pc.getMaxHypeEnergy());

    // fight estimate
    double foeDmg = 0, foeHp = 0;
    for (auto* m : alive) {
        foeDmg += std::max(1.0, (m->getModifiedHustle() - ndas) * em);
        foeHp += m->health;
    }
    const double basic = std::max(1.0, (hustle - low->getModifiedNDAs()) * pm);
    const double rounds = std::max(1.0, std::ceil(foeHp / basic));
    const double danger = foeDmg * rounds * 0.5 / runway;      // ~fraction of runway lost if only basic hits
    const bool dangerous = danger > T("pstrat.danger_ratio");
    const bool shortFight = rounds <= T("pstrat.short_rounds");
    const double hpFrac = runway / maxRunway;
    const double potency = GameMath::patentPotency(pc.getModifiedSynergy());

    // keep hype for a heal when things look bad
    int reserve = 0;
    if (dangerous) {
        reserve = INT_MAX;
        for (const auto& [n, p] : Patent::characterPatents)
            if (p.getType() == PatentType::HEAL) reserve = std::min(reserve, p.effectiveCost(&pc));
        if (reserve == INT_MAX) reserve = 0;
    }

    Monster* fattest = alive.front();
    for (auto* m : alive) if (m->health > fattest->health) fattest = m;

    std::uniform_real_distribution<double> noise(1.0 - T("pstrat.noise"), 1.0 + T("pstrat.noise"));
    auto& fx = pc.getEffectsManager();
    const double ahd = std::max(0.01, T("patent.attack_hustle_div")), add = std::max(0.01, T("patent.attack_def_div"));

    for (const auto& [name, p] : Patent::characterPatents) {
        const int cost = p.effectiveCost(&pc);
        if (hype < cost) continue;
        const PatentType t = p.getType();
        if (t != PatentType::HEAL && reserve > 0 && hype - cost < reserve) continue;

        double score = 0.0;
        Monster* tgt = low;
        bool aoe = false;
        switch (t) {
            case PatentType::ATTACK: {
                double ex = p.getCurrentPower() * potency + hustle / ahd - low->getModifiedNDAs() / add;
                score = std::max(1.0, ex) / basic * cls.w.attack;
                break;
            }
            case PatentType::AOE: {
                double ex = p.getCurrentPower() * potency + hustle / ahd - low->getModifiedNDAs() / add;
                score = std::max(1.0, ex) * alive.size() / basic * cls.w.attack;
                aoe = true;
                break;
            }
            case PatentType::HEAL: {
                if (hpFrac > T("pstrat.heal_frac")) break;
                double missing = maxRunway - runway;
                double amount = std::min(missing, p.getCurrentPower() * potency + pc.getModifiedSynergy() / 2.0);
                score = amount / maxRunway * 6.0 * cls.w.heal;
                break;
            }
            case PatentType::DEFENSE:
                if (!dangerous || shortFight || fx.hasEffectFrom(name)) break;
                score = std::min(danger, 3.0) * 2.0 * cls.w.defense;
                break;
            case PatentType::BUFF:
                if (!dangerous || shortFight || fx.hasEffectFrom(name)) break;
                score = std::min(danger, 3.0) * 1.5 * cls.w.buff;
                break;
            case PatentType::DEBUFF: {
                if (shortFight || fattest->getEffectsManager().hasEffectFrom(name)) break;
                if (static_cast<double>(fattest->health) / std::max(1, fattest->maxHealth) < T("pstrat.debuff_min_hp_frac")) break;
                tgt = fattest;
                score = (1.0 + std::min(danger, 2.0)) * cls.w.debuff;
                break;
            }
            default: break;
        }
        if (score <= 0.0) continue;
        score /= 1.0 + cost / maxHype * T("pstrat.cost_weight");
        score *= noise(rng);
        if (score > best.score) { best.patent = name; best.aoe = aoe; best.target = tgt; best.score = score; }
    }

    if (best.score < T("pstrat.min_score")) return PlayerChoice{};
    std::cout << "[ai] " << best.patent << " score " << best.score << " danger " << danger << "\n";
    return best;
}

}  // namespace PlayerStrategy
