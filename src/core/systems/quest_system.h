#ifndef QUEST_SYSTEM_H
#define QUEST_SYSTEM_H

#include "../engine_core.h"
#include "../components/quest_component.h"

namespace FusionQuest {

    class QuestSystem : public System {
    public:
        QuestSystem(Engine* engine);

        void initialize() override;
        void update(float deltaTime) override;

        void addQuest(GameObject* character, const QuestComponent::Quest& quest);
        QuestComponent::Quest createQuest(const std::string& id, const std::string& name,
                                       const std::string& description);
        void addObjective(QuestComponent::Quest& quest, const std::string& id,
                        const std::string& description);

    private:
        Engine* engine;
    };

} // namespace FusionQuest

#endif // QUEST_SYSTEM_H