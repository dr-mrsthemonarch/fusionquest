#include "inventory_component.h"

namespace FusionQuest {

    // Implement Item constructor
    InventoryComponent::Item::Item(const std::string& id, const std::string& name,
                                 const std::string& description)
        : id(id), name(name), description(description) {}

    InventoryComponent::InventoryComponent(GameObject* owner, size_t capacity)
        : Component(owner), capacity(capacity) {}

    bool InventoryComponent::addItem(const Item& item) {
        if (items.size() >= capacity) {
            return false;
        }

        items.push_back(item);
        return true;
    }

    bool InventoryComponent::removeItem(size_t index) {
        if (index >= items.size()) {
            return false;
        }

        items.erase(items.begin() + index);
        return true;
    }

    const std::vector<InventoryComponent::Item>& InventoryComponent::getItems() const {
        return items;
    }

    size_t InventoryComponent::getCapacity() const {
        return capacity;
    }

    void InventoryComponent::setCapacity(size_t newCapacity) {
        capacity = newCapacity;
    }

} // namespace FusionQuest