#include "../include/Quest.h"
#include <algorithm>

float Goal::fraction() const {
    if (target <= 0) return 1.0f;
    return static_cast<float>(std::min(1.0, static_cast<double>(progress) / static_cast<double>(target)));
}

std::string Goal::describe() const {
    std::string what;
    switch (type) {
        case GoalType::KILL: what = "Defeat " + keyword; break;
        case GoalType::COLLECT: what = "Collect " + keyword; break;
        case GoalType::BOSS: what = "Defeat bosses"; break;
        case GoalType::TOWN: what = "Visit town"; break;
    }
    return what + " " + std::to_string(std::min(progress, target)) + "/" + std::to_string(target);
}

bool Sprint::done() const {
    for (const auto& g : goals) if (!g.done()) return false;
    return true;
}

float Sprint::fraction() const {
    if (goals.empty()) return 1.0f;
    float s = 0.0f;
    for (const auto& g : goals) s += g.fraction();
    return s / static_cast<float>(goals.size());
}

float Dev::fraction() const {
    if (sprints.empty()) return 1.0f;
    float part = current < sprints.size() ? sprints[current].fraction() : 0.0f;
    return std::min(1.0f, (static_cast<float>(current) + part) / static_cast<float>(sprints.size()));
}
