#include "../include/Archetype.h"
#include "../include/ConfigStore.h"
#include "../include/Tuning.h"
#include <algorithm>
#include <iostream>

namespace {
std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}
bool setMod(StatMods& m, const std::string& k, double v) {
    if (k == "runway") m.runway = v; else if (k == "hype") m.hype = v; else if (k == "hustle") m.hustle = v;
    else if (k == "agility") m.agility = v; else if (k == "ndas") m.ndas = v; else if (k == "luck") m.luck = v;
    else if (k == "synergy") m.synergy = v; else return false;
    return true;
}
}  // namespace

Archetypes::Archetypes() { defs_.push_back(ClassDef{}); }

Archetypes& Archetypes::I() {
    static Archetypes a;
    return a;
}

const ClassDef& Archetypes::get(int id) const {
    if (defs_.empty()) { static ClassDef d; return d; }
    return defs_[static_cast<size_t>(std::clamp(id, 0, static_cast<int>(defs_.size()) - 1))];
}

std::vector<std::string> Archetypes::names() const {
    std::vector<std::string> n;
    for (const auto& d : defs_) n.push_back(d.name);
    return n;
}

double Archetypes::passive(int id, const char* key) const {
    if (get(id).passive != key) return 0.0;
    return T((std::string("passive.") + key).c_str());
}

bool Archetypes::load(const std::string& root) {
    ConfigFile f(root + "classes.txt");
    if (!f.is_open()) { std::cerr << "no classes.txt\n"; return false; }
    std::vector<ClassDef> out;
    ClassDef cur;
    bool in = false;
    std::string line;
    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (line == "[CLASS]") { cur = ClassDef{}; in = true; continue; }
        if (line == "[END]") { if (in) out.push_back(cur); in = false; continue; }
        size_t p = line.find('=');
        if (!in || p == std::string::npos) continue;
        std::string k = trim(line.substr(0, p)), v = trim(line.substr(p + 1));
        if (k == "name") { cur.name = v; continue; }
        if (k == "passive") { cur.passive = v; continue; }
        double d = 0;
        try { d = std::stod(v); } catch (...) { continue; }
        if (k == "goal_mult") cur.goalMult = d;
        else if (k == "cash_mult") cur.cashMult = d;
        else if (k == "repair_mult") cur.repairMult = d;
        else if (k.rfind("mod_", 0) == 0) setMod(cur.mods, k.substr(4), d);
        else if (k.rfind("grow_", 0) == 0) setMod(cur.grow, k.substr(5), d);
        else if (k == "w_attack") cur.w.attack = d;
        else if (k == "w_defense") cur.w.defense = d;
        else if (k == "w_heal") cur.w.heal = d;
        else if (k == "w_buff") cur.w.buff = d;
        else if (k == "w_debuff") cur.w.debuff = d;
    }
    if (out.empty()) return false;
    defs_ = out;
    return true;
}

std::string Archetypes::describe(const ClassDef& c) {
    auto sg = [](double v) { return (v >= 0 ? "+" : "") + std::to_string(static_cast<int>(v)); };
    std::string s = c.name + ":";
    auto add = [&](const char* n, double v) { if (v != 0) s += std::string(" ") + n + sg(v); };
    add("Runway", c.mods.runway); add("Hype", c.mods.hype); add("Hustle", c.mods.hustle);
    add("Agile", c.mods.agility); add("NDAs", c.mods.ndas); add("Luck", c.mods.luck); add("Syn", c.mods.synergy);
    s += " | goals x" + std::to_string(c.goalMult).substr(0, 4) + " cash x" + std::to_string(c.cashMult).substr(0, 4);
    if (!c.passive.empty()) s += " | " + c.passive;
    return s;
}
