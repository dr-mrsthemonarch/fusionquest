#ifndef EQUIPMENT_COMPONENT_H
#define EQUIPMENT_COMPONENT_H

#include "inventory_component.h"
#include "../engine_core.h"
#include <map>

namespace FusionQuest {

    class EquipmentComponent : public Component {
    public:
        enum class SlotType {
            Head,
            Body,
            Legs,
            Feet,
            Hands,
            Weapon,
            Shield,
            Accessory1,
            Accessory2,
        };

        EquipmentComponent(GameObject* owner);

        bool equip(SlotType slot, const InventoryComponent::Item& item);
        bool unequip(SlotType slot);
        const InventoryComponent::Item* getEquipped(SlotType slot) const;
        const std::map<SlotType, InventoryComponent::Item>& getAllEquipped() const;

    private:
        std::map<SlotType, InventoryComponent::Item> equipped;
    };

} // namespace FusionQuest

#endif // EQUIPMENT_COMPONENT_H