#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include "../engine_core.h"
#include "save_system.h"

namespace FusionQuest {

    class SaveSystem : public System {
    public:
        SaveSystem(Engine* engine);

        void initialize() override;
        bool saveCharacter(GameObject* character, const std::string& filename);
        GameObject* loadCharacter(const std::string& filename);

    private:
        Engine* engine;
    };

} // namespace FusionQuest

#endif // SAVE_SYSTEM_H