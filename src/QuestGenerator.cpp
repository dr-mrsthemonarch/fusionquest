#include "../include/Tuning.h"
#include "../include/ConfigStore.h"
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
    ConfigFile f(path);
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
        ConfigFile f(q + "preamble.txt");
        if (f.is_open()) { std::stringstream ss; ss << f.rdbuf(); preamble_ = ss.str(); }
        else preamble_ = "Your fusion startup begins.";
    }

    refresh();

    // main quest defs
    mainDefs_.clear();
    ConfigFile f(q + "main_quests.txt");
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
    return index > static_cast<int>(bal_.mainQuestCount) ? 0 : static_cast<int>(T("quest.hustles_base") + index * T("quest.hustles_per_index"));
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
    const double wk = T("quest.w_kill"), wc = T("quest.w_collect"), wb = T("quest.w_boss"), wt = T("quest.w_town");
    std::uniform_real_distribution<double> d(0.0, std::max(1e-9, wk + wc + wb + wt));
    double r = d(rng_);
    if ((r -= wk) < 0) return GoalType::KILL;
    if ((r -= wc) < 0) return GoalType::COLLECT;
    if ((r -= wb) < 0) return GoalType::BOSS;
    return GoalType::TOWN;
}

double QuestGenerator::secPer(GoalType t) const {
    switch (t) {
        case GoalType::KILL: return std::max(1.0, bal_.secPerKill) * bal_.timeScale;
        case GoalType::COLLECT: return std::max(1.0, bal_.secPerCollect) * bal_.timeScale;
        case GoalType::BOSS: return std::max(1.0, bal_.secPerBoss) * bal_.timeScale;
        case GoalType::TOWN: return std::max(1.0, bal_.secPerTown) * bal_.timeScale;
    }
    return 60.0;
}

Sprint QuestGenerator::makeSprint(int tier) {
    Sprint sp;
    sp.name = pick(sprintNames_);
    std::uniform_int_distribution<int> pct(0, 99);
    int roll = pct(rng_);
    const double one = T("quest.goals_one_pct"), two = T("quest.goals_two_pct");
    int ng = roll < one ? 1 : roll < one + two ? 2 : 3;
    for (int i = 0; i < ng; ++i) {
        Goal g;
        g.type = rollType();
        int lo = 1, hi = 1;
        switch (g.type) {
            case GoalType::KILL: g.keyword = pick(killKw_); lo = (int)T("quest.kill_lo"); hi = (int)T("quest.kill_hi_base") + tier / std::max(1, (int)T("quest.kill_hi_div")); break;
            case GoalType::COLLECT: g.keyword = pick(collectKw_); lo = (int)T("quest.collect_lo"); hi = (int)T("quest.collect_hi_base") + tier / std::max(1, (int)T("quest.collect_hi_div")); break;
            case GoalType::BOSS: lo = (int)T("quest.boss_lo"); hi = (int)T("quest.boss_hi_base") + tier / std::max(1, (int)T("quest.boss_hi_div")); break;
            case GoalType::TOWN: lo = (int)T("quest.town_lo"); hi = (int)T("quest.town_hi_base") + tier / std::max(1, (int)T("quest.town_hi_div")); break;
        }
        std::uniform_int_distribution<int> td(lo, std::max(lo, hi));
        g.target = std::max(1LL, std::llround(td(rng_) * bal_.goalScale));
        sp.goals.push_back(g);
    }
    return sp;
}

double QuestGenerator::estimateSeconds(const Sprint& s) const {
    double t = 0.0;
    for (const auto& g : s.goals) t += static_cast<double>(g.target) * secPer(g.type);
    return t;
}

void QuestGenerator::refill(Dev& d) {
    while (d.shown.size() < 2 && d.current + static_cast<long long>(d.shown.size()) < d.sprintCount) {
        Sprint sp = makeSprint(d.tier);
        sp.name += " #" + std::to_string(d.current + static_cast<long long>(d.shown.size()) + 1);
        d.shown.push_back(std::move(sp));
    }
}

// many small sprints: count = budget / avg sprint time. goals stay tiny.
Quest QuestGenerator::build(int index, double budget) {
    Quest q;
    q.index = index;
    const int devCount = (int)T("quest.devs_base") + index / std::max(1, (int)T("quest.devs_div"));

    double avg = 0.0;
    const int samples = 64;
    for (int i = 0; i < samples; ++i) avg += estimateSeconds(makeSprint(index));
    avg = std::max(avg / samples, 1.0);

    long long total = std::llround(budget / avg);
    total = std::clamp<long long>(total, devCount * static_cast<long long>(T("quest.min_sprints_per_dev")), static_cast<long long>(bal_.maxSprints));

    for (int d = 0; d < devCount; ++d) {
        Dev dev;
        dev.name = pick(devNames_);
        dev.tier = index;
        dev.sprintCount = total / devCount + (d == 0 ? total % devCount : 0);
        refill(dev);
        q.devs.push_back(std::move(dev));
    }
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

// pull every quest factor from the live Tuning registry (timeScale is debug-clock state, untouched)
void QuestGenerator::refresh() {
    bal_.usableFraction = T("quest.usable_fraction");
    bal_.growth = T("quest.growth");
    bal_.mainShare = T("quest.main_share");
    bal_.mainQuestCount = T("quest.main_count");
    bal_.secPerKill = T("quest.sec_per_kill");
    bal_.secPerCollect = T("quest.sec_per_collect");
    bal_.secPerBoss = T("quest.sec_per_boss");
    bal_.secPerTown = T("quest.sec_per_town");
    bal_.killBias = T("quest.kill_bias");
    bal_.collectBias = T("quest.collect_bias");
    bal_.switchChance = T("quest.switch_chance");
    bal_.newHustleWeight = T("quest.new_hustle_weight");
    bal_.maxOpenHustles = T("quest.max_hustles");
    bal_.rewardMainPct = T("quest.reward_main_pct");
    bal_.rewardHustlePct = T("quest.reward_hustle_pct");
    bal_.finalWarningDays = T("quest.final_warning_days");
    bal_.finalForceDays = T("quest.final_force_days");
    bal_.goalScale = T("quest.goal_scale");
    bal_.maxSprints = T("quest.max_sprints");
}

QuestGenerator::QuestPreview QuestGenerator::preview(int index, double elapsed) {
    QuestPreview p;
    const double b = mainBudget(index, elapsed);
    const bool fin = index > static_cast<int>(bal_.mainQuestCount);
    const double mb = fin ? b : b * bal_.mainShare;
    p.budget = b;
    p.hustleBudget = hustleBudget(index, elapsed);
    p.hustlesRequired = hustlesRequired(index);
    p.devs = (int)T("quest.devs_base") + index / std::max(1, (int)T("quest.devs_div"));
    double avg = 0.0;
    for (int i = 0; i < 32; ++i) avg += estimateSeconds(makeSprint(index));
    p.avgSprintSeconds = std::max(avg / 32, 1.0);
    p.sprints = std::clamp<long long>(std::llround(mb / p.avgSprintSeconds), p.devs * static_cast<long long>(T("quest.min_sprints_per_dev")), static_cast<long long>(bal_.maxSprints));
    return p;
}
