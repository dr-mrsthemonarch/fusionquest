#ifndef STATS_COMPONENT_H
#define STATS_COMPONENT_H

#include "../engine_core.h"
#include <map>
#include <string>

namespace FusionQuest {

    class StatsComponent : public Component {
    public:
        StatsComponent(GameObject* owner);

        void addStat(const std::string& name, float value);
        float getStat(const std::string& name) const;
        void setStat(const std::string& name, float value);
        void modifyStat(const std::string& name, float delta);
        const std::map<std::string, float>& getAllStats() const;

    private:
        std::map<std::string, float> stats;

        void notifyStatChanged(const std::string& statName);
    };

} // namespace FusionQuest

#endif // STATS_COMPONENT_H