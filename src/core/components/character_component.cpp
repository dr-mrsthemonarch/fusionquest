#include "character_component.h"
#include "stats_component.h"

namespace FusionQuest {

CharacterComponent::CharacterComponent(GameObject* owner) : Component(owner) {
    raceIndex = 0;
    classIndex = 0;
    fusionIndex = 0;
}

void CharacterComponent::setRaceIndex(int index) { raceIndex = index; }
int CharacterComponent::getRaceIndex() const { return raceIndex; }

void CharacterComponent::setClassIndex(int index) { classIndex = index; }
int CharacterComponent::getClassIndex() const { return classIndex; }

void CharacterComponent::setFusionIndex(int index) { fusionIndex = index; }
int CharacterComponent::getFusionIndex() const { return fusionIndex; }

void CharacterComponent::syncWithCharacter(const Character& character) {
    auto* statsComponent = getOwner()->getComponent<StatsComponent>();
    if (!statsComponent) {
        return;
    }
    
    getOwner()->setName(character.name);
    raceIndex = character.race_index;
    classIndex = character.class_index;
    fusionIndex = character.fusion_index;
    
    statsComponent->setStat("hustle", character.hustle);
    statsComponent->setStat("agility", character.agility);
    statsComponent->setStat("busfactor", character.busfactor);
    statsComponent->setStat("techdebt", character.techdebt);
    statsComponent->setStat("ndas", character.ndas);
    statsComponent->setStat("luck", character.luck);
    statsComponent->setStat("synergy", character.Synergy);
    statsComponent->setStat("runway", character.runway);
    statsComponent->setStat("hype_energy", character.hype_energy);
    statsComponent->setStat("experience", character.experience);
    statsComponent->setStat("progress", character.progress);
    statsComponent->setStat("series", character.Series);
}

Character CharacterComponent::toCharacter() const {
    Character character;
    auto* statsComponent = getOwner()->getComponent<StatsComponent>();
    
    character.name = getOwner()->getName();
    character.race_index = raceIndex;
    character.class_index = classIndex;
    character.fusion_index = fusionIndex;
    
    if (statsComponent) {
        character.hustle = static_cast<int>(statsComponent->getStat("hustle"));
        character.agility = static_cast<int>(statsComponent->getStat("agility"));
        character.busfactor = statsComponent->getStat("busfactor");
        character.techdebt = static_cast<int>(statsComponent->getStat("techdebt"));
        character.ndas = static_cast<int>(statsComponent->getStat("ndas"));
        character.luck = static_cast<int>(statsComponent->getStat("luck"));
        character.Synergy = static_cast<int>(statsComponent->getStat("synergy"));
        character.runway = statsComponent->getStat("runway");
        character.hype_energy = statsComponent->getStat("hype_energy");
        character.experience = statsComponent->getStat("experience");
        character.progress = statsComponent->getStat("progress");
        character.Series = static_cast<int>(statsComponent->getStat("series"));
    }
    
    return character;
}

} // namespace FusionQuest