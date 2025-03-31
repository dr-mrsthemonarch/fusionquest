#include "quest_system.h"

namespace FusionQuest {

    QuestSystem::QuestSystem(Engine* engine) : engine(engine) {}

    void QuestSystem::initialize() {
        // Subscribe to relevant events
    }

    void QuestSystem::update(float deltaTime) {
        // Update active quests
    }

    void QuestSystem::addQuest(GameObject* character, const QuestComponent::Quest& quest) {
        auto* questComp = character->getComponent<QuestComponent>();
        if (questComp) {
            questComp->addQuest(quest);
        }
    }

    QuestComponent::Quest QuestSystem::createQuest(const std::string& id, const std::string& name,
                                                const std::string& description) {
        QuestComponent::Quest quest;
        quest.id = id;
        quest.name = name;
        quest.description = description;
        quest.active = false;
        quest.completed = false;
        quest.progress = 0.0f;
        return quest;
    }

    void QuestSystem::addObjective(QuestComponent::Quest& quest, const std::string& id,
                                 const std::string& description) {
        QuestComponent::Objective objective;
        objective.id = id;
        objective.description = description;
        objective.completed = false;
        quest.objectives.push_back(objective);
    }

} // namespace FusionQuest