#ifndef QUEST_COMPONENT_H
#define QUEST_COMPONENT_H

#include "../engine_core.h"
#include <map>
#include <string>
#include <vector>

namespace FusionQuest {

    class QuestComponent : public Component {
    public:
        struct Objective {
            std::string id;
            std::string description;
            bool completed = false;
        };

        struct Quest {
            std::string id;
            std::string name;
            std::string description;
            std::vector<Objective> objectives;
            bool active = false;
            bool completed = false;
            float progress = 0.0f;
        };

        QuestComponent(GameObject* owner);

        void addQuest(const Quest& quest);
        bool activateQuest(const std::string& questId);
        bool completeObjective(const std::string& questId, const std::string& objectiveId);
        const std::map<std::string, Quest>& getAllQuests() const;
        std::vector<Quest> getActiveQuests() const;

    private:
        std::map<std::string, Quest> quests;

        void updateQuestProgress(Quest& quest);
    };

} // namespace FusionQuest

#endif // QUEST_COMPONENT_H