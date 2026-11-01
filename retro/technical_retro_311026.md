# FusionQuest retro

Idle ProgressQuest-style game, ftxui TUI. Real 20-year countdown = the only real win condition. Everything else is fake.

## What got built

| Area | Result |
|---|---|
| Quest layer | 21 main quests (config titles/stories). Main needs 1+k side hustles. Quest → devs → sprints → goals. Track switching after each dev. Final quest 21 forced near clock end, popup pauses engine, fake save button. |
| Pacing | Per-quest time budget = remaining clock × weight share. Goals tiny, sprint count fills budget. Sprints generated lazily (current + preview). |
| Keyword bias | Mobs/items biased toward active goal keyword so goals are reachable. |
| Rewards | Funding % + equipment on main/hustle done. Dev/sprint: nothing. |
| Config embedding | `tools/embed_config` → obfuscated byte arrays in exe. `ConfigFile` replaces `ifstream`. Startup `validateConfig()` fails loudly. `FQ_EXTERNAL_CONFIG` dev flag. |
| Save protection | SHA-256 + HMAC encrypt-then-MAC container. Edited/unsigned saves rejected. `verify()` before load wipes state. |
| Inventory | Stack counts, fixed-width columns, UTF-8 safe truncation. Special drops unsellable everywhere. Published Papers drop with negative value. |
| UI | Preamble page, final-warning page, shared `NavBar`, Special Items page wired up, battle log capped at 200 lines. |
| Debug | `--debug-instakill`, `--debug-clock <s>` (compresses clock-driven content), `--debug-tune`. |
| Tuner | ~350 tunables in `Tuning` registry, pure formulas in `Balance`. Factors + formula text, results for 8 enemy types, canvas plots with axes, resizable panels, dynamic plot list, checkbox popup for x/y variables, 4 x-axis modes incl. factor sweep, year-by-year projection table. Writes `tuning.txt`. |

## Bugs found and root causes

1. **-500M money.** `Monster::itemValue` uninitialised. Special MOB-type monsters (Safety, Conference, Paper, named minions) drop items → garbage value sold. Fixed with defaults, real values, money clamp.
2. **Never buys equipment.** Mostly consequence of #1 (negative money). Also buy cycle died after 8 slots, and could downgrade. Now new cycle + upgrade-only.
3. **Prime-check log spam.** `levelUpCharacterPatents` called every tick. Already runs once per series-up in `addFunding`. Removed tick call.
4. **Whale watcher forever.** Port dropped named-monster victory handling. `recordDefeat()` never called → blocking monster never cleared.
5. **Nemesis power -1.** Defeats never recorded → `types[-1]` out of bounds. Now recorded.
6. **Shadow reward int overflow.** `money*2 + 1e9` in int. Now clamped double math.
7. **Pre-existing compile issues.** `uniform_real_distribution<int>`, missing `<cmath>`.
8. **Latent:** equipment slot multipliers truncated by `int` param (1.1→1, 1.8→1). Kept defaults, exposed as tunables.

## Decisions worth remembering

- Elapsed time resets on load/restart. Intended: root mechanic is letting the clock run.
- Sprint design changed after balance feedback: first version had tens-of-thousands goals. Fix: tiny goals, many sprints, lazy generation.
- Every balance constant moved behind `T("key")`. Defaults reproduce original numbers (verified numerically).
- Tuner and game call the same `Balance::` functions. No duplicated formulas for the numbers it displays.
- Obfuscation/HMAC key is extractable by a determined reverse engineer. Stops casual editing only.

## What the tuner already revealed (defaults)

- Series ≈ 66 at year 20 vs intended ~20. Progression far too fast.
- Mobs trivial after ~year 2.
- Nemesis unbeatable from ~year 8 (defense > hustle).
- Boss HP formula exponent 0.5 → bosses weaker than mobs at high series.
- Funding scales slower than player stats.

## Verification done / not done

Done:
- Syntax-checked every touched file against ftxui v7.0.0 headers.
- Numeric parity of refactored formulas vs original.
- SHA-256 test vector, seal/tamper test, embedded-config load with `config/` hidden.
- Cross-check that every `T("key")` exists in the registry.
- Headless render of canvas charts and the checkbox popup.

Not done:
- Full CMake build (no cmake in sandbox).
- Running the game or tuner page against a live game.
- Save round trip, long-run balance sim.
- Real spawn/drop rates (`quest.sec_per_*` are guesses).

## Open risks

- `validateConfig` required-file list must match real `config/` (user-corrected once).
- Old saves invalid (v2 unsigned, v3/v4 rejected). Old saves may hold garbage item values.
- Nemesis spawns at second 0 unless `nemesis.first_delay_days` is set.
- Nothing equips special drops automatically.
- Projection model ignores patents/healing, assumes modeled gear.
- Tuner plot layout not persisted between runs.

## Open questions

1. Intended pace: series 20 at year 20? Decides what to tune first.
2. Should the nemesis be beatable? Intended win condition?
3. Add assumed patent contribution to the projection?
4. Auto-equip special drops when better?
5. Persist tuner plot layout?
6. Save a converter for v3/v4 saves, or leave rejected?

## Lessons

- Read the original (Qt) behaviour before trusting a port. Two bugs were dropped features.
- Uninitialised struct fields hide until something reads them. Default-initialise at declaration.
- Measure before tuning. A cheap analytic projection exposed more balance problems in minutes than playing would.
- Put constants behind a registry early. Retrofitting ~350 of them was the biggest single refactor.
