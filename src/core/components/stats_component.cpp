#include "stats_component.h"

namespace FusionQuest {

    StatsComponent::StatsComponent(GameObject* owner) : Component(owner) {}

    void StatsComponent::addStat(const std::string& name, float value) {
        stats[name] = value;
        notifyStatChanged(name);
    }

    float StatsComponent::getStat(const std::string& name) const {
        auto it = stats.find(name);
        return (it != stats.end()) ? it->second : 0.0f;
    }

    void StatsComponent::setStat(const std::string& name, float value) {
        stats[name] = value;
        notifyStatChanged(name);
    }

    void StatsComponent::modifyStat(const std::string& name, float delta) {
        stats[name] = getStat(name) + delta;
        notifyStatChanged(name);
    }

    const std::map<std::string, float>& StatsComponent::getAllStats() const {
        return stats;
    }

    void StatsComponent::notifyStatChanged(const std::string& statName) {
        // Implementation for stat change notification
    }

} // namespace FusionQuest