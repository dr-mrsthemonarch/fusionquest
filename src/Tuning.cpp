#include "../include/Tuning.h"
#include "../include/ConfigStore.h"
#include <fstream>
#include <iostream>

namespace {
struct Def { const char* key; const char* group; const char* desc; double def; double step; };

const Def kDefs[] = {
    // ---- player start (before level gains) ----
    {"player.runway", "Player start", "Starting Runway (HP)", 80, 5},
    {"player.hype", "Player start", "Starting Hype Energy (MP)", 10, 1},
    {"player.hustle", "Player start", "Starting Hustle (ATK)", 20, 1},
    {"player.agility", "Player start", "Starting Agility", 10, 1},
    {"player.ndas", "Player start", "Starting NDAs (DEF)", 9, 1},
    {"player.luck", "Player start", "Starting Luck", 5, 1},
    {"player.synergy", "Player start", "Starting Synergy", 5, 1},
    {"player.techdebt", "Player start", "Starting Technical Debt", -10, 1},
    {"player.bus", "Player start", "Starting Bus Factor", 1.0, 0.1},
    // ---- level gain: gain = base + (series-1) * per ----
    {"gain.runway_base", "Level gain", "Runway gain per series: base", 20, 1},
    {"gain.runway_per", "Level gain", "Runway gain per series: extra per series", 5, 0.5},
    {"gain.hype_base", "Level gain", "Hype gain: base", 15, 1},
    {"gain.hype_per", "Level gain", "Hype gain: extra per series", 3, 0.5},
    {"gain.hustle_base", "Level gain", "Hustle gain: base", 3, 0.5},
    {"gain.hustle_per", "Level gain", "Hustle gain: extra per series", 1, 0.1},
    {"gain.agility_base", "Level gain", "Agility gain: base", 4, 0.5},
    {"gain.agility_per", "Level gain", "Agility gain: extra per series", 0.5, 0.1},
    {"gain.ndas_base", "Level gain", "NDAs gain: base", 2, 0.5},
    {"gain.ndas_per", "Level gain", "NDAs gain: extra per series", 1, 0.1},
    {"gain.luck_base", "Level gain", "Luck gain: base", 1, 0.5},
    {"gain.luck_per", "Level gain", "Luck gain: extra per series", 0.3, 0.1},
    {"gain.synergy_base", "Level gain", "Synergy gain: base", 3, 0.5},
    {"gain.synergy_per", "Level gain", "Synergy gain: extra per series", 0.5, 0.1},
    {"gain.techdebt_base", "Level gain", "Tech debt added per series: base", 4, 0.5},
    {"gain.techdebt_per", "Level gain", "Tech debt: reduction per series", 0.5, 0.1},
    {"gain.bus_base", "Level gain", "Bus factor gain: base", 0.2, 0.05},
    {"gain.bus_per", "Level gain", "Bus factor gain: extra per series", 0.05, 0.01},
    // ---- funding (series progression) ----
    {"funding.base", "Funding", "Funding for series 1 (cost = base * series^exp / slowdown)", 1000, 100},
    {"funding.exponent", "Funding", "Series exponent", 2.8970554778884745, 0.05},
    {"funding.k", "Funding", "Time slowdown steepness (sigmoid K)", 1.0, 0.1},
    {"funding.c", "Funding", "Time slowdown midpoint (0..1 of the game)", 0.5, 0.05},
    {"funding.min_slowdown", "Funding", "Slowdown floor", 0.05, 0.01},
    {"funding.per_money", "Funding", "Funding gained per $ of monster reward", 0.1, 0.01},
    // ---- mobs: stat = (base + series*per) * difficulty ----
    {"mob.hp_base", "Mobs", "HP base", 50, 5},
    {"mob.hp_per", "Mobs", "HP per series", 22, 1},
    {"mob.atk_base", "Mobs", "Attack base", 40, 1},
    {"mob.atk_per", "Mobs", "Attack per series", 3.5, 0.1},
    {"mob.def_base", "Mobs", "Defense base", 10, 1},
    {"mob.def_per", "Mobs", "Defense per series", 3, 0.1},
    {"mob.spd_base", "Mobs", "Speed base", 5, 1},
    {"mob.spd_per", "Mobs", "Speed per series", 2.5, 0.1},
    {"mob.money_base", "Mobs", "Money base", 5, 1},
    {"mob.money_per", "Mobs", "Money per series", 15, 1},
    {"mob.var_hp", "Mobs", "HP random variance (+/- fraction)", 0.15, 0.01},
    {"mob.var_atk", "Mobs", "Attack variance", 0.2, 0.01},
    {"mob.var_def", "Mobs", "Defense variance", 0.2, 0.01},
    {"mob.var_spd", "Mobs", "Speed variance", 0.25, 0.01},
    {"mob.var_money", "Mobs", "Money variance", 0.2, 0.01},
    // ---- bosses ----
    {"boss.hp_base", "Bosses", "HP base (HP = base*(1+series)^exp*mult)", 50, 5},
    {"boss.hp_exp", "Bosses", "HP series exponent (orig 0.5: weak growth!)", 0.5, 0.05},
    {"boss.hp_mult", "Bosses", "HP multiplier", 2.5, 0.1},
    {"boss.atk_base", "Bosses", "Attack base", 35, 1},
    {"boss.atk_per", "Bosses", "Attack per series", 3.5, 0.1},
    {"boss.atk_mult", "Bosses", "Attack multiplier", 1.3, 0.05},
    {"boss.def_base", "Bosses", "Defense base", 5, 1},
    {"boss.def_per", "Bosses", "Defense per series", 3, 0.1},
    {"boss.def_mult", "Bosses", "Defense multiplier", 1.5, 0.05},
    {"boss.spd_base", "Bosses", "Speed base", 5, 1},
    {"boss.spd_per", "Bosses", "Speed per series", 2.5, 0.1},
    {"boss.spd_mult", "Bosses", "Speed multiplier", 1.2, 0.05},
    {"boss.money_base", "Bosses", "Money base", 30, 1},
    {"boss.money_per", "Bosses", "Money per series", 15, 1},
    {"boss.money_mult", "Bosses", "Money multiplier", 4.0, 0.1},
    {"boss.var_hp", "Bosses", "HP variance", 0.1, 0.01},
    {"boss.var_atk", "Bosses", "Attack variance", 0.15, 0.01},
    {"boss.var_def", "Bosses", "Defense variance", 0.15, 0.01},
    {"boss.var_spd", "Bosses", "Speed variance", 0.2, 0.01},
    {"boss.var_money", "Bosses", "Money variance", 0.15, 0.01},
    // ---- difficulty multipliers (all enemy stats) ----
    {"diff.easy", "Difficulty", "Easy multiplier", 0.75, 0.05},
    {"diff.medium", "Difficulty", "Medium multiplier", 1.0, 0.05},
    {"diff.hard", "Difficulty", "Hard multiplier", 1.35, 0.05},
    {"diff.legendary", "Difficulty", "Legendary multiplier", 1.8, 0.05},
    // ---- encounter mix ----
    {"enc.single_pct", "Encounters", "% single mob", 60, 1},
    {"enc.small_pct", "Encounters", "% small group (2-3)", 25, 1},
    {"enc.large_pct", "Encounters", "% large group (4); rest = boss + minions", 10, 1},
    {"enc.small_scale", "Encounters", "Small group per-mob stat scale", 0.7, 0.05},
    {"enc.large_scale", "Encounters", "Large group per-mob stat scale", 0.6, 0.05},
    {"enc.minion_scale", "Encounters", "Boss minion stat scale", 0.5, 0.05},
    // ---- nemesis ----
    {"nemesis.hp_base", "Nemesis", "HP base", 100, 10},
    {"nemesis.hp_per", "Nemesis", "HP per series", 30, 1},
    {"nemesis.atk_base", "Nemesis", "Attack base", 15, 1},
    {"nemesis.atk_per", "Nemesis", "Attack per series", 4, 0.1},
    {"nemesis.def_base", "Nemesis", "Defense base", 10, 1},
    {"nemesis.def_per", "Nemesis", "Defense per series", 3.5, 0.1},
    {"nemesis.spd_base", "Nemesis", "Speed base", 12, 1},
    {"nemesis.spd_per", "Nemesis", "Speed per series", 2.5, 0.1},
    {"nemesis.money_base", "Nemesis", "Money base", 200, 10},
    {"nemesis.money_per", "Nemesis", "Money per series", 50, 5},
    {"nemesis.hp_mult", "Nemesis", "HP multiplier", 4.0, 0.1},
    {"nemesis.atk_mult", "Nemesis", "Attack multiplier", 2.0, 0.1},
    {"nemesis.def_mult", "Nemesis", "Defense multiplier", 2.5, 0.1},
    {"nemesis.spd_mult", "Nemesis", "Speed multiplier", 1.8, 0.1},
    {"nemesis.money_mult", "Nemesis", "Money multiplier", 10.0, 0.5},
    {"nemesis.time_max", "Nemesis", "Extra scaling at end of game (1+x)", 8.0, 0.5},
    {"nemesis.time_k", "Nemesis", "Time sigmoid steepness", 12.0, 0.5},
    {"nemesis.time_c", "Nemesis", "Time sigmoid midpoint", 0.5, 0.05},
    {"nemesis.type0", "Nemesis", "Type mult years 1-5", 1.0, 0.1},
    {"nemesis.type1", "Nemesis", "Type mult years 6-10", 1.5, 0.1},
    {"nemesis.type2", "Nemesis", "Type mult years 11-15", 2.2, 0.1},
    {"nemesis.type3", "Nemesis", "Type mult years 16-20", 3.5, 0.1},
    {"nemesis.power", "Nemesis", "Per-defeat power multiplier", 1.1, 0.01},
    // ---- combat ----
    {"combat.variance", "Combat", "Damage random +/- (flat)", 4, 1},
    {"combat.player_dmg_mult", "Combat", "Player damage multiplier ((hustle-def)*x)", 1.0, 0.05},
    {"combat.enemy_dmg_mult", "Combat", "Enemy damage multiplier ((atk-ndas)*x)", 1.0, 0.05},
    // ---- named monsters ----
    {"named.growth_per_series", "Named monsters", "Stat growth per series (fraction of base)", 0.1, 0.01},
    {"named.power_mult", "Named monsters", "Global named-monster stat multiplier", 1.0, 0.05},
    // ---- equipment ----
    {"equip.base", "Equipment", "Stat base", 5, 1},
    {"equip.per_series", "Equipment", "Stat per series", 1.2, 0.1},
    {"equip.variance", "Equipment", "Stat variance (+/- fraction)", 0.2, 0.01},
    // ---- quests (replaces balance.txt) ----
    {"quest.usable_fraction", "Quests", "Fraction of clock main quests may use", 0.95, 0.01},
    {"quest.growth", "Quests", "Later quests get more time (weight = 1+growth*index)", 0.15, 0.01},
    {"quest.main_share", "Quests", "Main devs share of quest budget (rest = hustles)", 0.6, 0.05},
    {"quest.main_count", "Quests", "Number of normal main quests (final = +1)", 20, 1},
    {"quest.sec_per_kill", "Quests", "Est. seconds per matching kill", 40, 5},
    {"quest.sec_per_collect", "Quests", "Est. seconds per matching item", 60, 5},
    {"quest.sec_per_boss", "Quests", "Est. seconds per boss kill", 900, 50},
    {"quest.sec_per_town", "Quests", "Est. seconds per town visit", 3600, 100},
    {"quest.kill_bias", "Quests", "Chance a mob uses the active kill keyword", 0.15, 0.01},
    {"quest.collect_bias", "Quests", "Chance an item uses the active collect keyword", 0.15, 0.01},
    {"quest.switch_chance", "Quests", "Chance to switch track after a dev", 0.35, 0.05},
    {"quest.new_hustle_weight", "Quests", "Weight of starting a new hustle", 1.0, 0.1},
    {"quest.max_hustles", "Quests", "Max open side hustles", 3, 1},
    {"quest.reward_main_pct", "Quests", "Main quest funding reward (% of next series)", 25, 1},
    {"quest.reward_hustle_pct", "Quests", "Hustle funding reward (% of next series)", 10, 1},
    {"quest.final_warning_days", "Quests", "Final quest pending this many days before end", 182.5, 5},
    {"quest.final_force_days", "Quests", "Final quest forced this many days before end", 1.0, 0.5},
    {"quest.goal_scale", "Quests", "Multiplier on all goal targets", 1.0, 0.1},
    {"quest.max_sprints", "Quests", "Max sprints per quest", 50000, 1000},
    // ---- quest goal shapes ----
    {"quest.kill_lo", "Quest goals", "Kill goal min target", 2, 1},
    {"quest.kill_hi_base", "Quest goals", "Kill goal max = base + tier/div", 3, 1},
    {"quest.kill_hi_div", "Quest goals", "Kill goal tier divisor", 2, 1},
    {"quest.collect_lo", "Quest goals", "Collect goal min target", 1, 1},
    {"quest.collect_hi_base", "Quest goals", "Collect goal max base", 2, 1},
    {"quest.collect_hi_div", "Quest goals", "Collect goal tier divisor", 3, 1},
    {"quest.boss_lo", "Quest goals", "Boss goal min target", 1, 1},
    {"quest.boss_hi_base", "Quest goals", "Boss goal max base", 1, 1},
    {"quest.boss_hi_div", "Quest goals", "Boss goal tier divisor", 10, 1},
    {"quest.town_lo", "Quest goals", "Town goal min target", 1, 1},
    {"quest.town_hi_base", "Quest goals", "Town goal max base", 1, 1},
    {"quest.town_hi_div", "Quest goals", "Town goal tier divisor", 10, 1},
    {"quest.w_kill", "Quest goals", "Goal type weight: kill", 55, 5},
    {"quest.w_collect", "Quest goals", "Goal type weight: collect", 30, 5},
    {"quest.w_boss", "Quest goals", "Goal type weight: boss", 5, 1},
    {"quest.w_town", "Quest goals", "Goal type weight: town visit", 10, 1},
    {"quest.goals_one_pct", "Quest goals", "% sprints with 1 goal", 60, 5},
    {"quest.goals_two_pct", "Quest goals", "% sprints with 2 goals (rest 3)", 30, 5},
    {"quest.devs_base", "Quest shape", "Devs per quest = base + index/div", 1, 1},
    {"quest.devs_div", "Quest shape", "Devs index divisor", 2, 1},
    {"quest.min_sprints_per_dev", "Quest shape", "Minimum sprints per dev", 3, 1},
    {"quest.hustles_base", "Quest shape", "Hustles needed = base + index*per", 1, 1},
    {"quest.hustles_per_index", "Quest shape", "Hustles needed per quest index", 1, 0.5},
    // ---- timing ----
    {"time.ticks_per_action", "Timing", "100ms ticks per battle action (1 round = 2 actions)", 10, 1},
    {"time.ticks_between_battles", "Timing", "100ms ticks of rest between battles", 20, 1},
    {"time.battle_start_delay", "Timing", "Seconds before a battle begins", 2.0, 0.5},
    {"time.sell_step_ticks", "Timing", "100ms ticks per item sold in town", 3, 1},
    {"special.spawn_odds", "Timing", "1-in-N chance per special encounter check", 1000, 50},
    // ---- economy ----
    {"init.money", "Economy", "Starting money", 5000, 500},
    {"econ.sell_ratio", "Economy", "Equipment sell price / value", 0.6, 0.05},
    {"econ.default_inventory", "Economy", "Inventory slots without a backpack", 5, 1},
    {"hype.low_threshold", "Economy", "Buy hype when below this fraction of max", 0.1, 0.01},
    {"hype.cost_pct", "Economy", "Hype purchase cost (fraction of money)", 0.2, 0.01},
    {"hype.recover_frac", "Economy", "Hype restored (fraction of max)", 0.5, 0.05},
    {"emerg.sale_runway", "Economy", "Emergency sale when runway below", 10, 1},
    {"emerg.runway_money_div", "Economy", "Emergency sale: runway = money / this", 10, 1},
    {"emerg.heal_frac", "Economy", "Auto-heal patents when runway below this fraction", 0.3, 0.05},
    {"item.base_range", "Economy", "Item base value 1..N", 15, 1},
    {"item.bonus_fancy", "Economy", "Item value bonus: fancy names", 10, 1},
    {"item.bonus_epic", "Economy", "Item value bonus: epic names", 15, 1},
    {"item.bonus_corporate", "Economy", "Item value bonus: corporate names", 20, 1},
    {"drop.special_mult", "Economy", "Multiplier on every special-drop / quest equipment stat", 1.0, 0.1},
    // ---- patents (player) ----
    {"patent.max_level", "Patents", "Max patent level", 10, 1},
    {"patent.named_pct", "Patents", "% of new patents that are named ones", 20, 5},
    {"patent.use_base_pct", "Patents", "Use-patent chance (%) normally", 30, 5},
    {"patent.low_hp_frac", "Patents", "Low HP threshold (fraction)", 0.3, 0.05},
    {"patent.low_hp_pct", "Patents", "Use-patent chance at low HP", 70, 5},
    {"patent.mid_hp_frac", "Patents", "Mid HP threshold", 0.5, 0.05},
    {"patent.mid_hp_pct", "Patents", "Use-patent chance at mid HP", 50, 5},
    {"patent.low_hype_frac", "Patents", "Low hype threshold", 0.3, 0.05},
    {"patent.low_hype_pct", "Patents", "Use-patent chance at low hype", 15, 5},
    {"patent.min_hype_frac", "Patents", "No patents below this hype fraction", 0.2, 0.05},
    {"patent.defensive_pct", "Patents", "Auto defensive patent chance per hit (%)", 15, 5},
    {"patent.heal_choice_frac", "Patents", "Prefer heal patent below this HP fraction", 0.35, 0.05},
    {"patent.attack_hustle_div", "Patent effects", "Attack dmg = power + hustle/div", 2, 0.5},
    {"patent.attack_def_div", "Patent effects", "Attack reduced by target NDAs/div", 3, 0.5},
    {"patent.aoe_hustle_div", "Patent effects", "AOE dmg = power + hustle/div", 3, 0.5},
    {"patent.aoe_def_div", "Patent effects", "AOE reduced by NDAs/div", 3, 0.5},
    {"patent.heal_synergy_div", "Patent effects", "Heal = power + synergy/div", 2, 0.5},
    {"patent.defense_div", "Patent effects", "Defense shield = power/div", 2, 0.5},
    {"patent.defense_turns_base", "Patent effects", "Shield turns = base + level/div", 2, 1},
    {"patent.defense_turns_div", "Patent effects", "Shield turns level divisor", 3, 1},
    {"patent.buff_div", "Patent effects", "Buff amount = power/div", 10, 1},
    {"patent.debuff_div", "Patent effects", "Debuff amount = power/div", 8, 1},
    {"patent.buff_turns_base", "Patent effects", "Buff/debuff turns = base + level/div", 2, 1},
    {"patent.buff_turns_div", "Patent effects", "Buff/debuff level divisor", 4, 1},
    {"patent.regen_level", "Patent effects", "Heal gains regen at this level", 5, 1},
    {"patent.regen_div", "Patent effects", "Regen per turn = power/div", 20, 1},
    {"patent.regen_turns", "Patent effects", "Regen turns", 2, 1},
    {"patent.attack_pwr_base", "Patent growth", "Attack level-up power: base", 25, 1},
    {"patent.attack_pwr_lvl", "Patent growth", "Attack: per level", 4, 1},
    {"patent.attack_pwr_pdiv", "Patent growth", "Attack: prime/div", 2, 1},
    {"patent.heal_pwr_base", "Patent growth", "Heal level-up power: base", 20, 1},
    {"patent.heal_pwr_lvl", "Patent growth", "Heal: per level", 3, 1},
    {"patent.heal_pwr_pdiv", "Patent growth", "Heal: prime/div", 3, 1},
    {"patent.defense_pwr_base", "Patent growth", "Defense level-up power: base", 15, 1},
    {"patent.defense_pwr_lvl", "Patent growth", "Defense: per level", 2, 1},
    {"patent.defense_pwr_pdiv", "Patent growth", "Defense: prime/div", 4, 1},
    {"patent.bd_pwr_base", "Patent growth", "Buff/Debuff level-up power: base", 10, 1},
    {"patent.bd_pwr_lvl", "Patent growth", "Buff/Debuff: per level", 2, 1},
    {"patent.bd_pwr_pdiv", "Patent growth", "Buff/Debuff: prime/div", 5, 1},
    {"patent.attack_cost_base", "Patent cost", "Attack cost = base + power/div", 5, 1},
    {"patent.attack_cost_div", "Patent cost", "Attack cost divisor", 10, 1},
    {"patent.aoe_cost_base", "Patent cost", "AOE cost base", 7, 1},
    {"patent.aoe_cost_div", "Patent cost", "AOE cost divisor", 8, 1},
    {"patent.aoe_cost_mult", "Patent cost", "AOE cost multiplier", 1.3, 0.1},
    {"patent.heal_cost_base", "Patent cost", "Heal cost base", 8, 1},
    {"patent.heal_cost_div", "Patent cost", "Heal cost divisor", 8, 1},
    {"patent.defense_cost_base", "Patent cost", "Defense cost base", 6, 1},
    {"patent.defense_cost_div", "Patent cost", "Defense cost divisor", 12, 1},
    {"patent.bd_cost_base", "Patent cost", "Buff/Debuff cost base", 7, 1},
    {"patent.bd_cost_div", "Patent cost", "Buff/Debuff cost divisor", 15, 1},
    {"patent.attack_start", "Patent cost", "Random attack patent base power", 15, 1},
    {"patent.attack_range", "Patent cost", "  + random 0..range-1", 20, 1},
    {"patent.heal_start", "Patent cost", "Random heal patent base power", 12, 1},
    {"patent.heal_range", "Patent cost", "  + random range", 15, 1},
    {"patent.defense_start", "Patent cost", "Random defense patent base power", 10, 1},
    {"patent.defense_range", "Patent cost", "  + random range", 12, 1},
    {"patent.bd_start", "Patent cost", "Random buff/debuff patent base power", 8, 1},
    {"patent.bd_range", "Patent cost", "  + random range", 10, 1},
    // ---- difficulty roll ----
    {"diff.pct_easy", "Difficulty", "% easy spawn", 50, 5},
    {"diff.pct_medium", "Difficulty", "% medium spawn", 30, 5},
    {"diff.pct_hard", "Difficulty", "% hard spawn (rest legendary)", 19, 1},
    // ---- monsters misc ----
    {"monster.mob_hype_base", "Monster misc", "Mob hype base", 20, 1},
    {"monster.mob_hype_per", "Monster misc", "Mob hype per series", 5, 1},
    {"monster.boss_hype_base", "Monster misc", "Boss hype base", 50, 5},
    {"monster.boss_hype_per", "Monster misc", "Boss hype per series", 10, 1},
    {"monster.mob_patent_easy", "Monster misc", "Mob patent chance % easy", 30, 5},
    {"monster.mob_patent_medium", "Monster misc", "Mob patent chance % medium", 40, 5},
    {"monster.mob_patent_hard", "Monster misc", "Mob patent chance % hard", 60, 5},
    {"monster.mob_patent_legendary", "Monster misc", "Mob patent chance % legendary", 80, 5},
    {"monster.mob_patent_pow", "Monster misc", "Mob patent power base", 12, 1},
    {"monster.mob_patent_pow_per", "Monster misc", "Mob patent power per series", 2, 0.5},
    {"monster.mob_patent_cost", "Monster misc", "Mob patent cost base", 6, 1},
    {"monster.mob_patent_cost_per", "Monster misc", "Mob patent cost per series", 1, 0.5},
    {"monster.boss_patent_pow", "Monster misc", "Boss patent power base", 20, 1},
    {"monster.boss_patent_pow_per", "Monster misc", "Boss patent power per series", 3, 0.5},
    {"monster.boss_patent_cost", "Monster misc", "Boss patent cost base", 10, 1},
    {"monster.boss_patent_cost_per", "Monster misc", "Boss patent cost per series", 2, 0.5},
    {"monster.debuff_mult", "Monster misc", "Boss debuff power multiplier", 1.3, 0.1},
    // ---- special encounter monsters ----
    {"special.hp_base", "Special mobs", "Base HP", 50, 5},
    {"special.hp_per", "Special mobs", "HP per series", 22, 1},
    {"special.atk_base", "Special mobs", "Base attack", 8, 1},
    {"special.atk_per", "Special mobs", "Attack per series", 3.5, 0.1},
    {"special.def_base", "Special mobs", "Base defense", 5, 1},
    {"special.def_per", "Special mobs", "Defense per series", 3, 0.1},
    {"special.spd_base", "Special mobs", "Base speed", 5, 1},
    {"special.spd_per", "Special mobs", "Speed per series", 2.5, 0.1},
    {"special.money_base", "Special mobs", "Base money", 30, 5},
    {"special.money_per", "Special mobs", "Money per series", 15, 1},
    {"special.safety_hp", "Special mobs", "Safety Manager HP mult", 1.8, 0.1},
    {"special.safety_atk", "Special mobs", "Safety Manager attack mult", 0.5, 0.05},
    {"special.safety_def", "Special mobs", "Safety Manager defense mult", 10, 0.5},
    {"special.safety_spd", "Special mobs", "Safety Manager speed mult", 1, 0.1},
    {"special.safety_money", "Special mobs", "Safety Manager money mult", 2, 0.1},
    {"special.conf_hp", "Special mobs", "Conference HP mult", 0.9, 0.05},
    {"special.conf_atk", "Special mobs", "Conference attack mult", 0.8, 0.05},
    {"special.conf_def", "Special mobs", "Conference defense mult", 0.9, 0.05},
    {"special.conf_spd", "Special mobs", "Conference speed mult", 1.2, 0.05},
    {"special.conf_money", "Special mobs", "Conference money mult", 1.5, 0.1},
    {"special.paper_hp", "Special mobs", "Paper HP mult (orig 110!)", 110, 5},
    {"special.paper_atk", "Special mobs", "Paper attack mult", 0.7, 0.05},
    {"special.paper_def", "Special mobs", "Paper defense mult", 1.2, 0.05},
    {"special.paper_spd", "Special mobs", "Paper speed mult", 0.8, 0.05},
    {"special.paper_money", "Special mobs", "Paper money mult", 1.3, 0.1},
    // ---- monthly review bosses ----
    {"review.hp", "Review bosses", "Base HP (x yearScale x monthScale x diff)", 150, 10},
    {"review.atk", "Review bosses", "Base attack", 20, 1},
    {"review.def", "Review bosses", "Base defense", 15, 1},
    {"review.spd", "Review bosses", "Base speed", 12, 1},
    {"review.money", "Review bosses", "Base money", 100, 10},
    {"review.year_growth", "Review bosses", "yearScale = 1 + year*x", 0.5, 0.05},
    {"review.month_growth", "Review bosses", "monthScale = 1 + month/12*x", 0.2, 0.05},
    // ---- shadow player ----
    {"shadow.appear_years", "Shadow", "Can appear after N years", 10, 1},
    {"shadow.odds", "Shadow", "1-in-N chance per check", 1000000000, 100000000},
    {"shadow.hp_mult", "Shadow", "HP vs player", 1.1, 0.05},
    {"shadow.hype_mult", "Shadow", "Hype vs player", 1.2, 0.05},
    {"shadow.atk_mult", "Shadow", "Attack vs player", 1.1, 0.05},
    {"shadow.def_mult", "Shadow", "Defense vs player", 1.1, 0.05},
    {"shadow.spd_mult", "Shadow", "Speed vs player luck", 1.05, 0.05},
    {"shadow.patent_mult", "Shadow", "Patent power vs player", 1.15, 0.05},
    {"shadow.reward_flat", "Shadow", "Reward flat money", 1000000000, 100000000},
    {"shadow.reward_money_mult", "Shadow", "Reward + money x this", 2, 0.5},
    // ---- nemesis extras ----
    {"nemesis.interval_days", "Nemesis", "Days between nemesis appearances", 365, 10},
    {"nemesis.first_delay_days", "Nemesis", "First nemesis after N days (0 = immediately!)", 0, 30},
    {"nemesis.hype_base", "Nemesis", "Hype base", 120, 10},
    {"nemesis.hype_per", "Nemesis", "Hype per series", 20, 1},
    {"nemesis.hype_pow", "Nemesis", "Hype per power level", 30, 5},
    {"nemesis.patent_pow_base", "Nemesis", "Patent power base", 35, 1},
    {"nemesis.patent_pow_per", "Nemesis", "Patent power per series", 5, 0.5},
    {"nemesis.patent_pow_pow", "Nemesis", "Patent power per power level", 15, 1},
    {"nemesis.patent_cost_base", "Nemesis", "Patent cost base", 15, 1},
    {"nemesis.patent_cost_per", "Nemesis", "Patent cost per series", 2, 0.5},
    // ---- named monsters extras ----
    {"named.money_base", "Named monsters", "Money base", 500, 50},
    {"named.money_per", "Named monsters", "Money per series", 50, 5},
    {"named.minion_hp", "Named monsters", "Minion base HP", 80, 5},
    {"named.minion_atk", "Named monsters", "Minion base attack", 30, 1},
    {"named.minion_def", "Named monsters", "Minion base defense", 20, 1},
    {"named.minion_spd", "Named monsters", "Minion base speed", 25, 1},
    {"named.hype_base", "Named monsters", "Hype base", 100, 10},
    {"named.hype_per", "Named monsters", "Hype per series", 10, 1},
    {"named.patent_pow_per", "Named monsters", "Patent power per series", 5, 0.5},
    // ---- equipment slot multipliers ----
    {"equip.laptop_hustle", "Equipment", "Laptop hustle mult (orig code truncated 1.1 -> 1)", 1, 0.1},
    {"equip.laptop_td", "Equipment", "Laptop tech debt mult (orig 1.8 -> 1)", 1, 0.1},
    {"equip.laptop_ndas", "Equipment", "Laptop NDAs mult (orig 1.3 -> 1)", 1, 0.1},
    {"equip.bike_agility", "Equipment", "Bike agility mult", 2, 0.1},
    {"equip.phone_synergy", "Equipment", "Phone synergy mult", 2, 0.1},
    {"equip.headphones_td", "Equipment", "Headphones tech debt mult (orig 1.5 -> 1)", 1, 0.1},
    {"equip.jacket_luck", "Equipment", "Jacket luck mult", 2, 0.1},
    {"equip.gloves_each", "Equipment", "Gloves mult (synergy, hustle, agility)", 2, 0.1},
    {"equip.helmet_ndas", "Equipment", "Helmet NDAs mult", 2, 0.1},
    {"equip.backpack_base", "Equipment", "Backpack slots base", 10, 1},
    {"equip.backpack_per", "Equipment", "Backpack slots per series", 5, 0.5},
    {"equip.backpack_var", "Equipment", "Backpack slots +/- random", 2, 1},
    {"equip.backpack_min", "Equipment", "Backpack slots minimum", 5, 1},
    {"equip.val_runway", "Equip value", "Price weight: runway", 10, 1},
    {"equip.val_hype", "Equip value", "Price weight: hype", 8, 1},
    {"equip.val_hustle", "Equip value", "Price weight: hustle", 12, 1},
    {"equip.val_agility", "Equip value", "Price weight: agility", 12, 1},
    {"equip.val_ndas", "Equip value", "Price weight: NDAs", 6, 1},
    {"equip.val_luck", "Equip value", "Price weight: luck", 8, 1},
    {"equip.val_synergy", "Equip value", "Price weight: synergy", 8, 1},
    {"equip.val_td", "Equip value", "Price weight: tech debt", 3, 1},
    {"equip.val_bus", "Equip value", "Price weight: bus factor", 1, 1},
    {"equip.val_inv", "Equip value", "Price weight: inventory slot", 25, 1},
    {"equip.val_min", "Equip value", "Minimum price", 50, 10},
    {"rarity.stat_uncommon", "Rarity", "Stat mult uncommon", 1.2, 0.1},
    {"rarity.stat_rare", "Rarity", "Stat mult rare", 1.5, 0.1},
    {"rarity.stat_epic", "Rarity", "Stat mult epic", 2.0, 0.1},
    {"rarity.stat_legendary", "Rarity", "Stat mult legendary", 5.0, 0.5},
    {"rarity.val_uncommon", "Rarity", "Price mult uncommon", 1.5, 0.1},
    {"rarity.val_rare", "Rarity", "Price mult rare", 2.5, 0.1},
    {"rarity.val_epic", "Rarity", "Price mult epic", 4.0, 0.1},
    {"rarity.val_legendary", "Rarity", "Price mult legendary", 7.0, 0.5},
    {"rarity.cum_legendary", "Rarity", "Boss drop: legendary if roll<=N of 10000", 10, 5},
    {"rarity.cum_epic", "Rarity", "epic if roll<=N", 1500, 100},
    {"rarity.cum_rare", "Rarity", "rare if roll<=N", 3000, 100},
    {"rarity.cum_uncommon", "Rarity", "uncommon if roll<=N (rest common)", 8000, 100},
    // ---- projection model (tuner only) ----
    {"proj.gear_fraction", "Projection", "Fraction of the modeled gear kit equipped", 1.0, 0.1},
};

std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}
}  // namespace

Tuning::Tuning() {
    for (const auto& d : kDefs) {
        TuneParam p;
        p.key = d.key; p.group = d.group; p.desc = d.desc;
        p.value = p.def = d.def; p.step = d.step;
        index_[p.key] = params_.size();
        params_.push_back(p);
    }
}

Tuning& Tuning::I() {
    static Tuning t;
    return t;
}

void Tuning::loadOverrides(std::istream& in) {
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        size_t p = line.find('=');
        if (p == std::string::npos) continue;
        auto it = index_.find(trim(line.substr(0, p)));
        if (it == index_.end()) { std::cerr << "tuning: unknown key " << line.substr(0, p) << "\n"; continue; }
        try { params_[it->second].value = std::stod(trim(line.substr(p + 1))); } catch (...) {}
    }
}

void Tuning::init(bool debugTune) {
    enabled_ = debugTune;
    resetAll();
    { ConfigFile f("config/tuning.txt"); if (f.is_open()) loadOverrides(f); }
    if (debugTune) { std::ifstream f("tuning.txt"); if (f) loadOverrides(f); }
}

double Tuning::get(const char* key) const {
    auto it = index_.find(key);
    if (it == index_.end()) {
        if (!warned_) { std::cerr << "tuning: unknown key " << key << "\n"; warned_ = true; }
        return 0.0;
    }
    return params_[it->second].value;
}

bool Tuning::set(const std::string& key, double v) {
    auto it = index_.find(key);
    if (it == index_.end()) return false;
    params_[it->second].value = v;
    return true;
}

void Tuning::reset(size_t i) { if (i < params_.size()) params_[i].value = params_[i].def; }
void Tuning::resetAll() { for (auto& p : params_) p.value = p.def; }

bool Tuning::save(const std::string& path) const {
    std::ofstream out(path);
    if (!out) return false;
    out << "# tuning overrides. copy to config/tuning.txt and rebuild to ship. '*' = changed from default\n";
    std::string group;
    for (const auto& p : params_) {
        if (p.group != group) { group = p.group; out << "\n# ---- " << group << " ----\n"; }
        out << "# " << p.desc << (p.modified() ? "  (default " + std::to_string(p.def) + ")" : "") << "\n";
        out << p.key << "=" << p.value << "\n";
    }
    return true;
}
