#include "quest_component.h"

namespace FusionQuest {

QuestComponent::QuestComponent(GameObject* owner) : Component(owner) {}

void QuestComponent::addQuest(const Quest& quest) {
    quests[quest.id] = quest;
}

bool QuestComponent::activateQuest(const std::string& questId) {
    auto it = quests.find(questId);
    if (it == quests.end()) {
        return false;
    }
    
    it->second.active = true;
    return true;
}

bool QuestComponent::completeObjective(const std::string& questId, const std::string& objectiveId) {
    auto questIt = quests.find(questId);
    if (questIt == quests.end()) {
        return false;
    }
    
    Quest& quest = questIt->second;
    for (auto& objective : quest.objectives) {
        if (objective.id == objectiveId) {
            objective.completed = true;
            updateQuestProgress(quest);
            
            bool allCompleted = true;
            for (const auto& obj : quest.objectives) {
                if (!obj.completed) {
                    allCompleted = false;
                    break;
                }
            }
            
            if (allCompleted) {
                quest.completed = true;
            }
            
            return true;
        }
    }
    
    return false;
}

const std::map<std::string, QuestComponent::Quest>& QuestComponent::getAllQuests() const {
    return quests;
}

std::vector<QuestComponent::Quest> QuestComponent::getActiveQuests() const {
    std::vector<Quest> active;
    for (const auto& pair : quests) {
        if (pair.second.active && !pair.second.completed) {
            active.push_back(pair.second);
        }
    }
    return active;
}

void QuestComponent::updateQuestProgress(Quest& quest) {
    if (quest.objectives.empty()) {
        quest.progress = quest.completed ? 1.0f : 0.0f;
        return;
    }
    
    size_t completedCount = 0;
    for (const auto& objective : quest.objectives) {
        if (objective.completed) {
            completedCount++;
        }
    }
    
    quest.progress = static_cast<float>(completedCount) / quest.objectives.size();
}

} // namespace FusionQuest