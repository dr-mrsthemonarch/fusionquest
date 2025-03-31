#include "equipment_component.h"

namespace FusionQuest {

    EquipmentComponent::EquipmentComponent(GameObject* owner) : Component(owner) {}

    bool EquipmentComponent::equip(SlotType slot, const InventoryComponent::Item& item) {
        // Use emplace to avoid requiring default construction
        auto [it, inserted] = equipped.emplace(slot, item);
        if (!inserted) {
            // Slot was already occupied, replace the item
            it->second = item;
        }
        return true;
    }

    bool EquipmentComponent::unequip(SlotType slot) {
        return equipped.erase(slot) > 0;
    }

    const InventoryComponent::Item* EquipmentComponent::getEquipped(SlotType slot) const {
        auto it = equipped.find(slot);
        return (it != equipped.end()) ? &it->second : nullptr;
    }

    const std::map<EquipmentComponent::SlotType, InventoryComponent::Item>&
    EquipmentComponent::getAllEquipped() const {
        return equipped;
    }

} // namespace FusionQuest