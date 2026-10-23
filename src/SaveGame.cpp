#include "../include/SaveGame.h"
#include "../include/StdoutSink.h"
#include "../include/Patent.h"
#include "../include/SaveCrypto.h"
#include <map>
#include <filesystem>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {
constexpr int32_t SAVE_VERSION = 5; // v2: + employeeNr, position. v3: + quest state. v4: + inventory stack counts. v5: lazy sprints, counts before quest state (v3/v4 rejected)

template <typename T>
void w(std::ostream& f, const T& v) { f.write(reinterpret_cast<const char*>(&v), sizeof(v)); }
template <typename T>
void r(std::istream& f, T& v) { f.read(reinterpret_cast<char*>(&v), sizeof(v)); }

void writeStr(std::ostream& f, const std::string& s) {
    size_t len = s.size();
    w(f, len);
    f.write(s.data(), len);
}
std::string readStr(std::istream& f) {
    size_t len; r(f, len);
    std::string s(len, '\0');
    f.read(s.data(), len);
    return s;
}

void writeEquipment(std::ostream& f, const Equipment& eq) {
    w(f, eq.type); w(f, eq.rarity);
    writeStr(f, eq.name); writeStr(f, eq.equipmentclass);
    w(f, eq.runway); w(f, eq.hypeEnergy); w(f, eq.hustle); w(f, eq.agility);
    w(f, eq.ndas); w(f, eq.luck); w(f, eq.synergy); w(f, eq.technicalDebt);
    w(f, eq.busFactor); w(f, eq.inventoryLimit); w(f, eq.equipmentvalue);
}
Equipment readEquipment(std::istream& f) {
    Equipment eq;
    r(f, eq.type); r(f, eq.rarity);
    eq.name = readStr(f); eq.equipmentclass = readStr(f);
    r(f, eq.runway); r(f, eq.hypeEnergy); r(f, eq.hustle); r(f, eq.agility);
    r(f, eq.ndas); r(f, eq.luck); r(f, eq.synergy); r(f, eq.technicalDebt);
    r(f, eq.busFactor); r(f, eq.inventoryLimit); r(f, eq.equipmentvalue);
    return eq;
}
void writeSprint(std::ostream& f, const Sprint& s) {
    writeStr(f, s.name); w(f, s.goals.size());
    for (const auto& g : s.goals) { w(f, g.type); writeStr(f, g.keyword); w(f, g.target); w(f, g.progress); }
}
Sprint readSprint(std::istream& f) {
    Sprint s; s.name = readStr(f);
    size_t ng; r(f, ng);
    for (size_t k = 0; k < ng; ++k) {
        Goal g; r(f, g.type); g.keyword = readStr(f); r(f, g.target); r(f, g.progress);
        s.goals.push_back(g);
    }
    return s;
}
void writeQuest(std::ostream& f, const Quest& q) {
    writeStr(f, q.title); writeStr(f, q.story);
    w(f, q.index); w(f, q.hustlesRequired); w(f, q.current);
    w(f, q.devs.size());
    for (const auto& d : q.devs) {
        writeStr(f, d.name); w(f, d.sprintCount); w(f, d.current); w(f, d.tier);
        w(f, d.shown.size());
        for (const auto& s : d.shown) writeSprint(f, s);
    }
}
Quest readQuest(std::istream& f) {
    Quest q;
    q.title = readStr(f); q.story = readStr(f);
    r(f, q.index); r(f, q.hustlesRequired); r(f, q.current);
    size_t nd; r(f, nd);
    for (size_t i = 0; i < nd; ++i) {
        Dev d; d.name = readStr(f);
        r(f, d.sprintCount); r(f, d.current); r(f, d.tier);
        size_t ns; r(f, ns);
        for (size_t j = 0; j < ns; ++j) d.shown.push_back(readSprint(f));
        q.devs.push_back(d);
    }
    return q;
}
void writeQuestState(std::ostream& f, const QuestState& s) {
    writeQuest(f, s.main);
    w(f, s.mainActive); w(f, s.mainIndex); w(f, s.hustlesDone); w(f, s.hustleBudget);
    w(f, s.track); w(f, s.completedQuests); w(f, s.finalPending); w(f, s.finalStarted);
    w(f, s.hustles.size());
    for (const auto& h : s.hustles) writeQuest(f, h);
}
QuestState readQuestState(std::istream& f) {
    QuestState s;
    s.main = readQuest(f);
    r(f, s.mainActive); r(f, s.mainIndex); r(f, s.hustlesDone); r(f, s.hustleBudget);
    r(f, s.track); r(f, s.completedQuests); r(f, s.finalPending); r(f, s.finalStarted);
    size_t n; r(f, n);
    for (size_t i = 0; i < n; ++i) s.hustles.push_back(readQuest(f));
    return s;
}
// whole file -> verified plaintext. unsigned files only accepted in dev builds.
bool readProtected(const std::string& filename, std::string& plain) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) { std::cerr << "SaveGame: cannot open " << filename << "\n"; return false; }
    std::stringstream ss;
    ss << in.rdbuf();
    const std::string file = ss.str();
    if (SaveCrypto::open(file, plain)) return true;
#ifdef FQ_EXTERNAL_CONFIG
    if (!SaveCrypto::isSealed(file)) { plain = file; return true; }
#endif
    std::cerr << "SaveGame: save is corrupt, unsigned or was modified\n";
    return false;
}
} // namespace

bool SaveGame::verify(const std::string& filename) {
    StdoutSink silence;
    std::string plain;
    return readProtected(filename, plain);
}

bool SaveGame::save(const GameEngine& engine, const std::string& filename) {
    StdoutSink silence;
    const Character* c = engine.getCharacter();
    if (!c) { std::cerr << "SaveGame: no character\n"; return false; }

    std::ostringstream f(std::ios::binary); // sealed + written at the end

    w(f, SAVE_VERSION);
    writeStr(f, c->getName());
    writeStr(f, c->getStartup());
    writeStr(f, engine.getEmployeeNr());
    writeStr(f, engine.getPosition());

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
    if (auto* rc = engine.getReviewCalendarSystem()) {
        auto flags = rc->exportFlags();
        w(f, flags.size());
        for (const auto& fl : flags) { w(f, fl.year); w(f, fl.month); w(f, fl.encountered); w(f, fl.defeated); }
    } else {
        size_t zero = 0; w(f, zero);
    }

    // shadow player defeated
    w(f, engine.isShadowPlayerDefeated());

    // special drops library
    const auto& lib = engine.getSpecialDropsLibrary().getLibrary();
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

    // inventory stack counts
    const auto& cnts = Character::getInventoryCountsMap();
    w(f, cnts.size());
    for (const auto& [name, n] : cnts) { writeStr(f, name); w(f, n); }

    writeQuestState(f, engine.getQuestSystem()->state());

    std::ofstream out(filename, std::ios::binary);
    if (!out) { std::cerr << "SaveGame: cannot open " << filename << "\n"; return false; }
    const std::string sealed = SaveCrypto::seal(f.str());
    out.write(sealed.data(), static_cast<std::streamsize>(sealed.size()));
    out.close();
    std::cout << "saved: " << filename << "\n";
    return true;
}

bool SaveGame::load(GameEngine& engine, const std::string& filename) {
    StdoutSink silence;
    std::string plain;
    if (!readProtected(filename, plain)) return false;
    std::istringstream f(plain, std::ios::binary);

    int32_t version; r(f, version);
    if (version < 2 || version > SAVE_VERSION || version == 3 || version == 4) { std::cerr << "SaveGame: version mismatch (" << version << ")\n"; return false; }

    std::string name = readStr(f);
    std::string startup = readStr(f);
    std::string employeeNr = readStr(f);
    std::string position = readStr(f);

    CharacterProfile bp{};
    r(f, bp.runway); r(f, bp.hypeEnergy); r(f, bp.hustle); r(f, bp.agility);
    r(f, bp.ndas); r(f, bp.luck); r(f, bp.synergy); r(f, bp.technicalDebt); r(f, bp.busFactor);

    int series, money, runway, maxRunway, hype, maxHype;
    double funding;
    r(f, series); r(f, money); r(f, runway); r(f, maxRunway); r(f, hype); r(f, maxHype); r(f, funding);

    size_t equippedCount; r(f, equippedCount);
    std::map<EquipmentType, Equipment> equipped;
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
    const bool haveCounts = version >= 5; // v2 saves: stacks of 1, fresh quest 1
    std::unordered_map<std::string, int> invCounts;
    if (haveCounts) {
        size_t n; r(f, n);
        for (size_t i = 0; i < n; ++i) { std::string k = readStr(f); int v; r(f, v); invCounts[k] = v; }
    }
    QuestState qs;
    const bool haveQs = version >= 5;
    if (haveQs) qs = readQuestState(f);

    // --- apply ---
    // NOTE: initializeCharacter() already ran generic startup (money, funding, starter gear,
    // starting patents, fresh review calendar) via caller before calling load(). we overwrite
    // that here with saved state. clock stays at whatever fresh CountdownComponent gives (elapsed=0).
    Character* c = engine.getCharacter();
    if (!c) { std::cerr << "SaveGame::load: engine has no character (call initializeCharacter first)\n"; return false; }

    engine.setIdentityDirect(employeeNr, position);
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
    if (haveCounts) Character::setInventoryCountsDirect(invCounts);

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

    if (haveQs) engine.getQuestSystem()->state() = std::move(qs);
    engine.getQuestSystem()->restored();

    engine.mutableCounters() = counters;
    engine.onStateRestored();

    std::cout << "loaded: " << filename << "\n";
    return true;
}

bool SaveGame::saveSlot(const GameEngine& engine, std::string* outPath) {
    const Character* c = engine.getCharacter();
    if (!c) return false;
    std::string fname = c->getName();
    for (char& ch : fname) if (!std::isalnum(static_cast<unsigned char>(ch))) ch = '_';
    if (fname.empty()) fname = "save";
    std::error_code ec;
    std::filesystem::create_directories("saves", ec);
    std::string path = "saves/" + fname + ".sav";
    bool ok = save(engine, path);
    if (ok && outPath) *outPath = path;
    return ok;
}