#include "../include/QuestSystem.h"
#include <algorithm>
#include <cctype>

namespace {
std::string lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
bool icontains(const std::string& h, const std::string& n) {
    return !n.empty() && lower(h).find(lower(n)) != std::string::npos;
}
float questFrac(const Quest& q, int extra = 0) {
    size_t n = q.devs.size() + extra;
    if (n == 0) return 1.0f;
    float part = q.current < q.devs.size() ? q.devs[q.current].fraction() : 0.0f;
    return std::min(1.0f, (static_cast<float>(q.current) + part) / static_cast<float>(n));
}
}  // namespace

bool QuestSystem::loadConfig(const std::string& root) { return gen_.loadConfig(root); }

void QuestSystem::start(double elapsed) {
    state_ = QuestState{};
    elapsed_ = elapsed;
    awaitAck_ = warn_ = false;
    beginMain(1);
}

void QuestSystem::beginMain(int index) {
    state_.mainIndex = index;
    state_.main = gen_.generateMain(index, elapsed_);
    state_.hustleBudget = gen_.hustleBudget(index, elapsed_);
    state_.mainActive = true;
    state_.hustlesDone = 0;
    state_.track = 0;
    log("QUEST " + std::to_string(index) + ": " + state_.main.title);
    if (!state_.main.story.empty()) log(state_.main.story);
}

void QuestSystem::startFinal() {
    state_.finalStarted = true;
    if (state_.mainActive && !state_.main.done()) log("main quest abandoned: " + state_.main.title);
    beginMain(static_cast<int>(gen_.balance().mainQuestCount) + 1);
    awaitAck_ = true;
    warn_ = true;
}

void QuestSystem::tick(double elapsed) {
    elapsed_ = elapsed;
    const auto& b = gen_.balance();
    const double warnAt = b.totalSeconds - b.finalWarningDays * 86400.0;
    const double forceAt = b.totalSeconds - b.finalForceDays * 86400.0;
    if (!state_.finalPending && elapsed >= warnAt) {
        state_.finalPending = true;
        log("final quest looms. finish current quest.");
    }
    if (!state_.finalStarted && (elapsed >= forceAt || (state_.finalPending && !state_.mainActive))) {
        startFinal();
        return;
    }
    if (!activeQuest()) chooseTrack(false);
}

const Quest* QuestSystem::activeQuest() const {
    if (state_.track == 0)
        return (state_.mainActive && !state_.main.done()) ? &state_.main : nullptr;
    size_t i = static_cast<size_t>(state_.track - 1);
    return i < state_.hustles.size() ? &state_.hustles[i] : nullptr;
}

Quest* QuestSystem::active() {
    return const_cast<Quest*>(static_cast<const QuestSystem*>(this)->activeQuest());
}

const Dev* QuestSystem::activeDev() const {
    const Quest* q = activeQuest();
    if (!q || q->current >= q->devs.size()) return nullptr;
    return &q->devs[q->current];
}

int QuestSystem::activeCount() const {
    return (state_.mainActive && !state_.main.done() ? 1 : 0) + static_cast<int>(state_.hustles.size());
}

std::vector<Goal*> QuestSystem::activeGoals() {
    std::vector<Goal*> out;
    Quest* q = active();
    if (!q || q->done()) return out;
    Dev& d = q->devs[q->current];
    if (d.done()) return out;
    if (d.shown.empty()) return out;
    for (auto& g : d.shown[0].goals) if (!g.done()) out.push_back(&g);
    return out;
}

std::vector<std::string> QuestSystem::killKeywords() {
    std::vector<std::string> out;
    for (Goal* g : activeGoals()) if (g->type == GoalType::KILL) out.push_back(g->keyword);
    return out;
}

std::vector<std::string> QuestSystem::collectKeywords() {
    std::vector<std::string> out;
    for (Goal* g : activeGoals()) if (g->type == GoalType::COLLECT) out.push_back(g->keyword);
    return out;
}

void QuestSystem::onKill(const std::string& name, bool isBoss) {
    bool hit = false;
    for (Goal* g : activeGoals()) {
        if ((g->type == GoalType::KILL && icontains(name, g->keyword)) || (g->type == GoalType::BOSS && isBoss)) {
            g->progress++;
            hit = true;
        }
    }
    if (hit) afterProgress();
}

void QuestSystem::onItem(const std::string& name) {
    bool hit = false;
    for (Goal* g : activeGoals())
        if (g->type == GoalType::COLLECT && icontains(name, g->keyword)) { g->progress++; hit = true; }
    if (hit) afterProgress();
}

void QuestSystem::onTownVisit() {
    bool hit = false;
    for (Goal* g : activeGoals()) if (g->type == GoalType::TOWN) { g->progress++; hit = true; }
    if (hit) afterProgress();
}

void QuestSystem::afterProgress() {
    Quest* q = active();
    if (!q) return;
    bool devDone = false;
    while (!q->done()) {
        Dev& d = q->devs[q->current];
        if (d.done()) { log("dev done: " + d.name); q->current++; devDone = true; continue; }
        if (d.shown.empty()) { gen_.refill(d); if (d.shown.empty()) break; }
        if (d.shown[0].done()) { d.current++; d.shown.erase(d.shown.begin()); gen_.refill(d); continue; } // no log: too many sprints
        break;
    }
    if (q->done()) { finishActive(); return; }
    if (devDone) {
        std::uniform_real_distribution<double> r(0.0, 1.0);
        if (r(rng_) < gen_.balance().switchChance) chooseTrack(true);
    }
}

void QuestSystem::finishActive() {
    if (state_.track == 0) {
        if (state_.hustlesDone >= state_.main.hustlesRequired) completeMain();
        else { log("main waits on side hustles"); chooseTrack(false); }
        return;
    }
    size_t idx = static_cast<size_t>(state_.track - 1);
    std::string title = state_.hustles[idx].title;
    state_.hustles.erase(state_.hustles.begin() + idx);
    state_.track = 0;
    state_.completedQuests++;
    state_.hustlesDone++;
    log("SIDE HUSTLE DONE: " + title);
    if (rewardFn_) rewardFn_(false, title);
    if (state_.mainActive && state_.main.done() && state_.hustlesDone >= state_.main.hustlesRequired) completeMain();
    else chooseTrack(false);
}

void QuestSystem::completeMain() {
    const std::string title = state_.main.title;
    state_.completedQuests++;
    state_.mainActive = false;
    state_.track = 0;
    log("MAIN QUEST DONE: " + title);
    if (rewardFn_) rewardFn_(true, title);
    const int last = static_cast<int>(gen_.balance().mainQuestCount);
    if (state_.mainIndex > last) { log("power plant commissioned. wait for the clock."); chooseTrack(false); return; }
    if (state_.finalPending && !state_.finalStarted) { startFinal(); return; }
    if (state_.mainIndex < last) beginMain(state_.mainIndex + 1);
    else chooseTrack(false); // all normal mains done, hustles until final
}

void QuestSystem::chooseTrack(bool excludeCurrent) {
    struct C { int t; double w; };
    std::vector<C> c;
    double sum = 0.0;
    auto add = [&](int t, double w) { c.push_back({t, w}); sum += w; };
    if (state_.mainActive && !state_.main.done() && !(excludeCurrent && state_.track == 0)) add(0, 1.0);
    for (size_t i = 0; i < state_.hustles.size(); ++i)
        if (!(excludeCurrent && state_.track == static_cast<int>(i) + 1)) add(static_cast<int>(i) + 1, 1.0);
    if (static_cast<int>(state_.hustles.size()) < static_cast<int>(gen_.balance().maxOpenHustles))
        add(-1, gen_.balance().newHustleWeight);
    if (c.empty() || sum <= 0.0) return;

    std::uniform_real_distribution<double> r(0.0, sum);
    double x = r(rng_);
    int chosen = c.back().t;
    for (const auto& e : c) { x -= e.w; if (x <= 0.0) { chosen = e.t; break; } }

    if (chosen == -1) {
        state_.hustles.push_back(gen_.generateHustle(state_.mainIndex, std::max(state_.hustleBudget, 86400.0)));
        state_.track = static_cast<int>(state_.hustles.size());
        log("SIDE HUSTLE: " + state_.hustles.back().title);
    } else if (chosen != state_.track) {
        state_.track = chosen;
        const Quest* q = activeQuest();
        if (q) log("focus: " + q->title);
    }
}

QuestSystem::Gauge QuestSystem::mainGauge() const {
    if (!state_.mainActive) return {"-", 0.0f};
    const Quest& m = state_.main;
    float total = static_cast<float>(m.devs.size() + m.hustlesRequired);
    float part = m.current < m.devs.size() ? m.devs[m.current].fraction() : 0.0f;
    float done = static_cast<float>(m.current) + part + static_cast<float>(std::min(state_.hustlesDone, m.hustlesRequired));
    return {std::to_string(state_.mainIndex) + ". " + m.title, total > 0 ? std::min(1.0f, done / total) : 1.0f};
}

QuestSystem::Gauge QuestSystem::hustleGauge() const {
    const Quest* h = nullptr;
    bool paused = false;
    if (state_.track > 0 && static_cast<size_t>(state_.track - 1) < state_.hustles.size()) h = &state_.hustles[state_.track - 1];
    else if (!state_.hustles.empty()) { h = &state_.hustles[0]; paused = true; }
    if (!h) return {"-", 0.0f};
    return {(paused ? "(paused) " : "") + h->title, questFrac(*h)};
}

QuestSystem::Gauge QuestSystem::devGauge() const {
    const Dev* d = activeDev();
    if (!d) return {"-", 0.0f};
    return {d->name, d->fraction()};
}
