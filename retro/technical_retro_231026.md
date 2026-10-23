# Retro 2: technical work done

## Design

Hierarchy and track-switch model, lazy sprint model, pacing math.

## New code

Quest, QuestGenerator, QuestSystem: data, generation, state machine, events, gauges, final-quest rules.
preamble_page, final_warning_page: new UI pages and enum entries.
ConfigStore, ConfigCipher, tools/embed_config: config embedded in exe.
Sha256, SaveCrypto: SHA-256, HMAC, encrypt-then-MAC saves.
Config: preamble, 21 main quests, name lists, balance.txt.

## Engine edits

GameEngine: owns quests, event hooks, reward grant, bias sync, tick gating for popup, isGameWon = clock zero, detail log cap 200.
MonsterGenerator, ItemGenerator: keyword bias.
SpecialDropsLibrary: QUEST_DROP, quest equipment generator.
Character: item stacks (count map), special-drop sell protection hook, <cmath> fix.
Selloff: whole stacks, skips special gear.

## Save

v2 → v5 over iterations. Quest state, stack counts, then sealed container.
Old unsigned saves rejected (dev flag allows). v3/v4 rejected.
verify() before load wipes state.

## UI

Stats page: real quest gauges, sprint panel, fixed-width columns, UTF-8 safe truncation.
Load page error message.
main.cpp: popup page switch, startup config validation (fatal exit).

## Build

## CMake: embed tool, generated ConfigData.cpp, FQ_EXTERNAL_CONFIG dev flag, removed config copy to bin/, new sources.

## Fixes found on the way

uniform_real_distribution<int> in EquipmentGenerator.
Missing <cmath>.
Rebalance: goals were tens of thousands, now tiny with many sprints.

V## erification done

Syntax-checked all touched files against ftxui v7.0.0 headers.
SHA-256 test vector, seal/tamper test, embedded-config load test, generator sample counts.
Not done: full CMake build, running the game, save round trip, balance sim.

## Open risks

Required-file list in validateConfig guessed from loaders.
Pacing unvalidated against real spawn rates.
Obfuscation and key extractable by reverse engineer.
Existing saves invalid.