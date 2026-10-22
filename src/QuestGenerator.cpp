#include "../include/QuestGenerator.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

namespace {
std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
std::vector<std::string> readLines(const std::string& path) {
    std::vector<std::string> out;
    std::ifstream f(path);
    if (!f.is_open()) return out;
    std::string line;
    while (std::getline(f, line)) {
        line = trim(line);
        if (!line.empty() && line[0] != '#') out.push_back(line);
    }
    return out;
}
void append(std::vector<std::string>& dst, const std::vector<std::string>& src) {
    dst.insert(dst.end(), src.begin(), src.end());
}
}  // namespace

QuestGenerator::QuestGenerator() = default;

bool QuestGenerator::loadConfig(const std::string& root) {
    const std::string q = root + "quests/";
    hustleNames_ = readLines(q + "hustle_names.txt");
    devNames_ = readLines(q + "dev_names.txt");
    sprintNames_ = readLines(q + "sprint_names.txt");
    if (hustleNames_.empty()) hustleNames_ = {"Side Gig"};
    if (devNames_.empty()) devNames_ = {"Dev Work"};
    if (sprintNames_.empty()) sprintNames_ = {"Sprint"};

    // keywords come from the same lists the generators use for names
    killKw_.clear(); collectKw_.clear();
    append(killKw_, readLines(root + "mobs/mobs.txt"));
    append(killKw_, readLines(root + "mobs/mythical_nouns.txt"));
    append(collectKw_, readLines(root + "items/office_nouns.txt"));
    append(collectKw_, readLines(root + "items/science_nouns.txt"));
    append(collectKw_, readLines(root + "items/mythical_nouns.txt"));
    if (killKw_.empty()) killKw_ = {"Bug"};
    if (collectKw_.empty()) collectKw_ = {"Widget"};

    // preamble: raw text
    preamble_.clear();
    {
        std::ifstream f(q + "preamble.txt");
        if (f.is_open()) { std::stringstream ss; ss << f.rdbuf(); preamble_ = ss.str(); }
        else preamble_ = "Your fusion startup begins.";
    }

    // balance
    std::map<std::string, double*> keys = {
        {"usable_fraction", &bal_.usableFraction}, {"growth", &bal_.growth}, {"main_share", &bal_.mainShare},
        {"main_quest_count", &bal_.mainQuestCount}, {"sec_per_kill", &bal_.secPerKill},
        {"sec_per_collect", &bal_.secPerCollect}, {"sec_per_boss", &bal_.secPerBoss},
        {"sec_per_town", &bal_.secPerTown}, {"kill_bias", &bal_.killBias}, {"collect_bias", &bal_.collectBias},
        {"switch_chance", &bal_.switchChance}, {"new_hustle_weight", &bal_.newHustleWeight},
        {"max_open_hustles", &bal_.maxOpenHustles}, {"reward_main_pct", &bal_.rewardMainPct},
        {"reward_hustle_pct", &bal_.rewardHustlePct}, {"final_warning_days", &bal_.finalWarningDays},
        {"final_force_days", &bal_.finalForceDays}};
    for (const auto& line : readLines(q + "balance.txt")) {
        size_t p = line.find('=');
        if (p == std::string::npos) continue;
        auto it = keys.find(trim(line.substr(0, p)));
        if (it == keys.end()) continue;
        try { *it->second = std::stod(trim(line.substr(p + 1))); } catch (...) {}
    }

    // main quest defs
    mainDefs_.clear();
    std::ifstream f(q + "main_quests.txt");
    if (!f.is_open()) { std::cerr << "no main_quests.txt\n"; return false; }
    std::string line;
    MainDef cur; bool in = false;
    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (line == "[QUEST]") { cur = MainDef{}; in = true; continue; }
        if (line == "[END]") { if (in) mainDefs_.push_back(cur); in = false; continue; }
        size_t p = line.find('=');
        if (!in || p == std::string::npos) continue;
        std::string k = trim(line.substr(0, p)), v = trim(line.substr(p + 1));
        if (k == "title") cur.title = v;
        else if (k == "story") cur.story = v;
    }
    std::cout << "Loaded " << mainDefs_.size() << " main quests\n";
    return !mainDefs_.empty();
}

int QuestGenerator::hustlesRequired(int index) const {
    return index > static_cast<int>(bal_.mainQuestCount) ? 0 : 1 + index;
}

double QuestGenerator::mainBudget(int index, double elapsed) const {
    const int n = static_cast<int>(bal_.mainQuestCount);
    if (index > n) return std::max(86400.0, bal_.totalSeconds - elapsed); // final: rest of clock
    double remaining = std::max(0.0, bal_.totalSeconds * bal_.usableFraction - elapsed);
    double sumW = 0.0;
    for (int j = index; j <= n; ++j) sumW += 1.0 + bal_.growth * j;
    double w = 1.0 + bal_.growth * index;
    return std::max(3600.0, remaining * w / std::max(sumW, 1e-9));
}

double QuestGenerator::hustleBudget(int index, double elapsed) const {
    double b = mainBudget(index, elapsed);
    int k = hustlesRequired(index);
    return k > 0 ? b * (1.0 - bal_.mainShare) / k : b * 0.1;
}

std::string QuestGenerator::pick(const std::vector<std::string>& v) {
    std::uniform_int_distribution<size_t> d(0, v.size() - 1);
    return v[d(rng_)];
}

GoalType QuestGenerator::rollType() {
    std::uniform_int_distribution<int> d(0, 99);
    int r = d(rng_);
    if (r < 55) return GoalType::KILL;
    if (r < 85) return GoalType::COLLECT;
    if (r < 90) return GoalType::BOSS;
    return GoalType::TOWN;
}

double QuestGenerator::secPer(GoalType t) const {
    switch (t) {
        case GoalType::KILL: return std::max(1.0, bal_.secPerKill);
        case GoalType::COLLECT: return std::max(1.0, bal_.secPerCollect);
        case GoalType::BOSS: return std::max(1.0, bal_.secPerBoss);
        case GoalType::TOWN: return std::max(1.0, bal_.secPerTown);
    }
    return 60.0;
}

Quest QuestGenerator::build(int index, double budget) {
    Quest q;
    q.index = index;
    const int devCount = 1 + index / 2;
    const int sprintCount = 2 + index / 2;
    std::uniform_int_distribution<int> goalCount(1, 3);

    for (int d = 0; d < devCount; ++d) {
        Dev dev;
        dev.name = pick(devNames_);
        for (int s = 0; s < sprintCount; ++s) {
            Sprint sp;
            sp.name = pick(sprintNames_);
            int ng = goalCount(rng_);
            for (int g = 0; g < ng; ++g) {
                Goal goal;
                goal.type = rollType();
                if (goal.type == GoalType::KILL) goal.keyword = pick(killKw_);
                else if (goal.type == GoalType::COLLECT) goal.keyword = pick(collectKw_);
                sp.goals.push_back(goal);
            }
            dev.sprints.push_back(std::move(sp));
        }
        q.devs.push_back(std::move(dev));
    }

    // targets: budget split by random weights, converted by seconds-per-unit
    std::uniform_real_distribution<double> wd(0.5, 1.5);
    std::vector<std::pair<Goal*, double>> refs;
    double sumW = 0.0;
    for (auto& d : q.devs) for (auto& s : d.sprints) for (auto& g : s.goals) {
        double w = wd(rng_);
        refs.push_back({&g, w});
        sumW += w;
    }
    for (auto& [g, w] : refs)
        g->target = std::max(1LL, std::llround(budget * w / sumW / secPer(g->type)));
    return q;
}

Quest QuestGenerator::generateMain(int index, double elapsed) {
    double b = mainBudget(index, elapsed);
    bool fin = index > static_cast<int>(bal_.mainQuestCount);
    Quest q = build(index, fin ? b : b * bal_.mainShare);
    q.hustlesRequired = hustlesRequired(index);
    if (index >= 1 && static_cast<size_t>(index - 1) < mainDefs_.size()) {
        q.title = mainDefs_[index - 1].title;
        q.story = mainDefs_[index - 1].story;
    } else {
        q.title = "Main Quest " + std::to_string(index);
    }
    return q;
}

Quest QuestGenerator::generateHustle(int mainIndex, double budgetSeconds) {
    Quest q = build(std::max(1, mainIndex), std::max(budgetSeconds, 86400.0));
    q.title = pick(hustleNames_);
    return q;
}
