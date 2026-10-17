#include "../include/SaveGame.h"
#include "../include/Patent.h"
#include <fstream>
#include <iostream>

namespace {
constexpr int32_t SAVE_VERSION = 1;

template <typename T>
void w(std::ofstream& f, const T& v) { f.write(reinterpret_cast<const char*>(&v), sizeof(v)); }
template <typename T>
void r(std::ifstream& f, T& v) { f.read(reinterpret_cast<char*>(&v), sizeof(v)); }

void writeStr(std::ofstream& f, const std::string& s) {
    size_t len = s.size();
    w(f, len);
    f.write(s.data(), len);
}
std::string readStr(std::ifstream& f) {
    size_t len; r(f, len);
    std::string s(len, '\0');
    f.read(s.data(), len);
    return s;
}

void writeEquipment(std::ofstream& f, const Equipment& eq) {
    w(f, eq.type); w(f, eq.rarity);
    writeStr(f, eq.name); writeStr(f, eq.equipmentclass);
    w(f, eq.runway); w(f, eq.hypeEnergy); w(f, eq.hustle); w(f, eq.agility);
    w(f, eq.ndas); w(f, eq.luck); w(f, eq.synergy); w(f, eq.technicalDebt);
    w(f, eq.busFactor); w(f, eq.inventoryLimit); w(f, eq.equipmentvalue);
}
Equipment readEquipment(std::ifstream& f) {
    Equipment eq;
    r(f, eq.type); r(f, eq.rarity);
    eq.name = readStr(f); eq.equipmentclass = readStr(f);
    r(f, eq.runway); r(f, eq.hypeEnergy); r(f, eq.hustle); r(f, eq.agility);
    r(f, eq.ndas); r(f, eq.luck); r(f, eq.synergy); r(f, eq.technicalDebt);
    r(f, eq.busFactor); r(f, eq.inventoryLimit); r(f, eq.equipmentvalue);
    return eq;
}
} // namespace

bool SaveGame::save(const GameEngine& engine, const std::string& filename) {
    const Character* c = engine.getCharacter();
    if (!c) { std::cerr << "SaveGame: no character\n"; return false; }

    std::ofstream f(filename, std::ios::binary);
    if (!f) { std::cerr << "SaveGame: cannot open " << filename << "\n"; return false; }

    w(f, SAVE_VERSION);
    writeStr(f, c->getName());
    writeStr(f, c->getStartup());

    // base profile (pre-equipment stats) - restoring this + equipment gives correct profile_ post-recalc
    const CharacterProfile& bp = c->getBaseProfile();
    w(f, bp.runway); w(f, bp.hypeEnergy); w(f, bp.hustle); w(f, bp.agility);
    w(f, bp.ndas); w(f, bp.luck); w(f, bp.synergy); w(f, bp.technicalDebt); w(f, bp.busFactor);

    w(f, c->getSeries());
    w(f, c->getMoney());
    w(f, c->getRunway());
    w(f, c->getMaxRunway());
    w(f, c->getHypeEnergy());
    w(f, c->getMaxHypeEnergy());
    w(f, c->getCurrentFunding());

    // equipped items
    const auto& equipped = c->getEquippedItemsMap();
    w(f, equipped.size());
    for (const auto& [type, eq] : equipped) { w(f, type); writeEquipment(f, eq); }

    // equipment inventory (unequipped)
    const auto& eqInv = c->getEquipmentInventoryVec();
    w(f, eqInv.size());
    for (const auto& eq : eqInv) writeEquipment(f, eq);

    // item inventory (name -> value), static/shared map
    const auto& inv = c->getInventoryMap();
    w(f, inv.size());
    for (const auto& [name, val] : inv) { writeStr(f, name); w(f, val); }

    // patents (global static table)
    w(f, Patent::characterPatents.size());
    for (const auto& [name, patent] : Patent::characterPatents) {
        writeStr(f, name);
        w(f, patent.getLevel()); w(f, patent.getCurrentPower()); w(f, patent.getCost());
        w(f, patent.getType()); w(f, patent.getTarget());
    }

    // nemesis power level
    w(f, engine.getNemesisSystem() ? engine.getNemesisSystem()->getNemesisPowerLevel() : 0);

    // review calendar flags
    if (auto* rc = const_cast<GameEngine&>(engine).getReviewCalendarSystem()) {
        auto flags = rc->exportFlags();
        w(f, flags.size());
        for (const auto& fl : flags) { w(f, fl.year); w(f, fl.month); w(f, fl.encountered); w(f, fl.defeated); }
    } else {
        size_t zero = 0; w(f, zero);
    }

    // shadow player defeated
    w(f, engine.isShadowPlayerDefeated());

    // special drops library
    const auto& lib = const_cast<GameEngine&>(engine).getSpecialDropsLibrary().getLibrary();
    w(f, lib.size());
    for (const auto& drop : lib) {
        w(f, drop.type);
        writeEquipment(f, drop.equipment);
        writeStr(f, drop.name);
        writeStr(f, drop.acquiredFrom);
        w(f, drop.yearAcquired); w(f, drop.monthAcquired);
        w(f, drop.isEquipped);
        writeStr(f, drop.loreText);
    }

    // counters
    w(f, engine.getCounters());

    f.close();
    std::cout << "saved: " << filename << "\n";
    return true;
}

bool SaveGame::load(GameEngine& engine, const std::string& filename) {
    std::ifstream f(filename, std::ios::binary);
    if (!f) { std::cerr << "SaveGame: cannot open " << filename << "\n"; return false; }

    int32_t version; r(f, version);
    if (version != SAVE_VERSION) { std::cerr << "SaveGame: version mismatch (" << version << ")\n"; return false; }

    std::string name = readStr(f);
    std::string startup = readStr(f);

    CharacterProfile bp{};
    r(f, bp.runway); r(f, bp.hypeEnergy); r(f, bp.hustle); r(f, bp.agility);
    r(f, bp.ndas); r(f, bp.luck); r(f, bp.synergy); r(f, bp.technicalDebt); r(f, bp.busFactor);

    int series, money, runway, maxRunway, hype, maxHype;
    double funding;
    r(f, series); r(f, money); r(f, runway); r(f, maxRunway); r(f, hype); r(f, maxHype); r(f, funding);

    size_t equippedCount; r(f, equippedCount);
    std::unordered_map<EquipmentType, Equipment> equipped;
    for (size_t i = 0; i < equippedCount; ++i) {
        EquipmentType type; r(f, type);
        equipped[type] = readEquipment(f);
    }

    size_t eqInvCount; r(f, eqInvCount);
    std::vector<Equipment> eqInv;
    for (size_t i = 0; i < eqInvCount; ++i) eqInv.push_back(readEquipment(f));

    size_t invCount; r(f, invCount);
    std::unordered_map<std::string, int> inv;
    for (size_t i = 0; i < invCount; ++i) { std::string n = readStr(f); int v; r(f, v); inv[n] = v; }

    size_t patentCount; r(f, patentCount);
    struct SavedPatent { std::string name; int level, power, cost; PatentType type; PatentTarget target; };
    std::vector<SavedPatent> savedPatents;
    for (size_t i = 0; i < patentCount; ++i) {
        SavedPatent sp; sp.name = readStr(f);
        r(f, sp.level); r(f, sp.power); r(f, sp.cost);
        r(f, sp.type); r(f, sp.target);
        savedPatents.push_back(sp);
    }

    int nemesisPower; r(f, nemesisPower);

    size_t flagCount; r(f, flagCount);
    std::vector<ReviewCalendarSystem::ReviewFlag> flags;
    for (size_t i = 0; i < flagCount; ++i) {
        ReviewCalendarSystem::ReviewFlag fl{};
        r(f, fl.year); r(f, fl.month); r(f, fl.encountered); r(f, fl.defeated);
        flags.push_back(fl);
    }

    bool shadowDefeated; r(f, shadowDefeated);

    size_t dropCount; r(f, dropCount);
    struct SavedDrop { SpecialDropType type; Equipment eq; std::string name, from; int year, month; bool equipped; std::string lore; };
    std::vector<SavedDrop> savedDrops;
    for (size_t i = 0; i < dropCount; ++i) {
        SavedDrop d{};
        r(f, d.type);
        d.eq = readEquipment(f);
        d.name = readStr(f); d.from = readStr(f);
        r(f, d.year); r(f, d.month); r(f, d.equipped);
        d.lore = readStr(f);
        savedDrops.push_back(d);
    }

    GameEngine::Counters counters{};
    r(f, counters);
    f.close();

    // --- apply ---
    // NOTE: initializeCharacter() already ran generic startup (money, funding, starter gear,
    // starting patents, fresh review calendar) via caller before calling load(). we overwrite
    // that here with saved state. clock stays at whatever fresh CountdownComponent gives (elapsed=0).
    Character* c = engine.getCharacter();
    if (!c) { std::cerr << "SaveGame::load: engine has no character (call initializeCharacter first)\n"; return false; }

    c->setNameDirect(name);
    c->setStartupDirect(startup);
    c->setBaseProfileDirect(bp); // also recalculates profile_ from base + (not-yet-set) equipment
    c->setSeriesDirect(series);
    c->setMoneyDirect(money);
    c->setMaxRunwayDirect(maxRunway);
    c->setMaxHypeEnergyDirect(maxHype);
    c->setRunway(runway);
    c->setHypeEnergy(hype);
    c->setCurrentFundingDirect(funding);
    c->setEquippedItemsDirect(equipped); // recalculates profile_ again w/ equipment bonuses
    c->setEquipmentInventoryDirect(eqInv);
    Character::setInventoryMapDirect(inv);

    Patent::characterPatents.clear();
    for (const auto& sp : savedPatents) {
        Patent p(sp.name, 0, sp.type, sp.power, sp.cost, sp.target);
        p.setLevelDirect(sp.level);
        Patent::characterPatents.insert({sp.name, p});
    }

    if (engine.getNemesisSystem()) engine.getNemesisSystem()->setPowerLevelDirect(nemesisPower);

    engine.rebuildReviewCalendarForLoad();
    if (engine.getReviewCalendarSystem()) engine.getReviewCalendarSystem()->restoreFlags(flags);

    engine.setShadowPlayerDefeated(shadowDefeated);

    for (const auto& d : savedDrops) {
        SpecialDrop drop;
        drop.type = d.type; drop.equipment = d.eq; drop.name = d.name;
        drop.acquiredFrom = d.from; drop.yearAcquired = d.year; drop.monthAcquired = d.month;
        drop.isEquipped = d.equipped; drop.loreText = d.lore;
        engine.getSpecialDropsLibrary().addSpecialDrop(drop); // reassigns sequential uniqueID, order preserved
    }

    engine.mutableCounters() = counters;

    std::cout << "loaded: " << filename << "\n";
    return true;
}
