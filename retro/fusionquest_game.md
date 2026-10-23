# Retro 1: game as I understand it

## Premise

FusionQuest. ProgressQuest-style idle game. Plays itself. Player watches, saves, quits.
Theme: fusion startup satire. Stats renamed: Runway (HP), Hype Energy (MP), Hustle (ATK), NDAs (DEF), Agility, Luck, Synergy, Tech Debt, Bus Factor.
Real clock = 20-year countdown. Only true win: clock hits zero. Everything else fake. Series (levels) can pass 20.
Close game = clock restarts. Load = clock resets to 0. Intended.

## Core loop (engine ticks 100ms)

Encounter generated → battle auto → victory/defeat → short wait → next.
Player turn: patent (spell) with luck-weighted chance, else basic hit on lowest-HP enemy. Enemies use strategies (aggressive, balanced, debuff, tactical, mirror, defensive).
Victory: money, funding (→ series up), item drops (mobs), equipment (bosses), special drops.
Series up: stats grow, auto-buy equipment, patents level on prime series.
Inventory full → town: animated selloff, buy gear. Runway under 10 → emergency sale. Defeat → bankruptcy, liquidate, recover.

## Content

Encounters: single mob, groups, boss + minions.
Rare: Safety Manager (+2 cronies), Conference, Paper Deadline.
Yearly nemesis: evolves each defeat, scrambled player name.
240 monthly review bosses.
Shadow player: 1 in 1 billion, after year 10.
Named monsters: config-defined, block everything until killed.
Special drops: unsellable library.

## Quest layer (new)

21 main quests, config titles and stories. Quest 21 = Power Plant Commissioning.
Main needs 1+k side hustles done. Side hustles = distractions, random switching after each dev finish.
Quest → devs → sprints → goals. Goals tiny (kill X keyword, collect Y, boss, town). Many sprints per quest.
Name keyword bias on mobs and items so goals reachable.
Rewards: funding % plus equipment on quest/hustle done. Dev and sprint: nothing.
Pacing: budget from remaining clock, tunable in balance.txt.
Final quest: pending 6 months out, forced 1 day out. Popup pauses engine, fake save button. Then generated devs/sprints like others.
Preamble page once on new game.

## UI

ftxui TUI: main menu, char creation (roll stats), preamble, stats page (3 columns), battle log page, equipment, special drops, load.
Qt MainWindow = legacy, unused.

## Known soft spots

Real kill/drop rates unmeasured. sec_per_* are guesses.
Pre-existing quirks: static shared inventory, currentFunding_ int vs double funding, many stdout prints captured as log.
