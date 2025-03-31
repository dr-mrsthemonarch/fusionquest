#ifndef UI_SYSTEM_H
#define UI_SYSTEM_H

#include "../engine_core.h"
#include "../../character.h"
#include "ui_system.h"

namespace FusionQuest {

    class UISystem : public System {
    public:
        UISystem(Engine* engine);

        void initialize() override;
        void updateUI();
        GameObject* createCharacterFromUI(const Character& uiCharacter);
        void updateUICharacter(GameObject* gameObject, Character& uiCharacter);

    private:
        Engine* engine;
    };

} // namespace FusionQuest

#endif // UI_SYSTEM_H