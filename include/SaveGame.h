#ifndef SAVEGAME_H
#define SAVEGAME_H

#include <string>
#include "GameEngine.h"

// versioned binary save. format v1:
//   [int32 version]
//   [Character block: name, employeeNr, position, startup, money, series,
//    hustle/agility/ndas/luck/synergy/technicalDebt/busFactor, runway/maxRunway,
//    hypeEnergy/maxHypeEnergy, currentFunding]
//   [equippedItems_ : count + each (EquipmentType key + Equipment fields)]
//   [equipmentInventory_ : count + each Equipment]
//   [inventory map : count + each (name,value)]
//   [Patent::characterPatents (global/static) : count + each (name, level, currentPower, cost, type, target)]
//   [NemesisSystem: powerLevel]
//   [ReviewCalendarSystem: 240 entries' encountered/defeated bits]
//   [ShadowPlayerSystem: defeated bool]
//   [SpecialDropsLibrary: count + each SpecialDrop (equipment + meta)]
//   [Counters: totalBattles/Victories/Defeats/bossesDefeated/bankruptcies/townVisits]
//
// NOTE elapsed-clock NOT saved. on load, ftxui CountdownComponent restarts fresh (elapsed=0);
// GameEngine::lastNemesisTime_ and ReviewCalendarSystem schedule recompute relative to that
// fresh baseline (call reviewCalendarSystem_->generateReviewCalendar() again post-load, then
// re-apply saved encountered/defeated flags matched by year+month index).
//
// requires header additions applied (see include/*_header_additions.txt):
//   Character.h, NemesisSystem.h, Patent.h, ExpandedMonsterTypes.h (ReviewCalendarSystem)
// call sequence for a load: engine.initializeCharacter(...) first (fresh char + starter gear +
// fresh review calendar via fresh CountdownComponent), THEN SaveGame::load(engine, file)
// overwrites w/ saved progress. clock always starts at 0 regardless of save contents.

class SaveGame {
public:
    static bool save(const GameEngine& engine, const std::string& filename);
    static bool load(GameEngine& engine, const std::string& filename);
    // saves/<sanitized character name>.sav, creates saves/ if missing. outPath optional.
    static bool saveSlot(const GameEngine& engine, std::string* outPath = nullptr);
};

#endif