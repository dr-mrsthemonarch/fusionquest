#ifndef CHARACTER_COMPONENT_H
#define CHARACTER_COMPONENT_H

#include "../engine_core.h"
#include "../../classes/character.h"

namespace FusionQuest {

    class CharacterComponent : public Component {
    public:
        CharacterComponent(GameObject* owner);

        void setRaceIndex(int index);
        int getRaceIndex() const;

        void setClassIndex(int index);
        int getClassIndex() const;

        void setFusionIndex(int index);
        int getFusionIndex() const;

        void syncWithCharacter(const Character& character);
        Character toCharacter() const;

    private:
        int raceIndex;
        int classIndex;
        int fusionIndex;
    };

} // namespace FusionQuest

#endif // CHARACTER_COMPONENT_H