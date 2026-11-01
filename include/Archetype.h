#ifndef ARCHETYPE_H
#define ARCHETYPE_H

#include <string>
#include <vector>

// player classes (config/classes.txt). races are flavor only.
struct StatMods { double runway = 0, hype = 0, hustle = 0, agility = 0, ndas = 0, luck = 0, synergy = 0; };
struct StratWeights { double attack = 1, defense = 1, heal = 1, buff = 1, debuff = 1; };

struct ClassDef {
    std::string name = "Median Manager";
    double goalMult = 1.0;     // quest goal targets x (higher = slower quests)
    double cashMult = 1.0;     // monster cash x (funding unaffected)
    double repairMult = 1.0;   // town repair cost x
    StatMods mods;             // flat, at creation
    StatMods grow;             // per series
    StratWeights w;            // patent choice weights
    std::string passive;       // extra_action luck_drop break_resist buff_potency crit_mult hype_discount
};

class Archetypes {
public:
    static Archetypes& I();
    bool load(const std::string& root);              // root = "config/"
    size_t count() const { return defs_.size(); }
    const ClassDef& get(int id) const;               // clamped
    std::vector<std::string> names() const;
    // magnitude (Tuning "passive.<key>") if class has this passive, else 0
    double passive(int id, const char* key) const;
    static std::string describe(const ClassDef& c);

private:
    Archetypes();
    std::vector<ClassDef> defs_;
};

#endif
