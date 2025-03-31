#ifndef INVENTORY_COMPONENT_H
#define INVENTORY_COMPONENT_H

#include "../engine_core.h"
#include <map>
#include <string>
#include <vector>

namespace FusionQuest {

    class InventoryComponent : public Component {
    public:
        struct Item {
            std::string id;
            std::string name;
            std::string description;
            std::map<std::string, float> stats;

            // Declaration only
            Item(const std::string& id, const std::string& name,
                 const std::string& description = "");
        };

        InventoryComponent(GameObject* owner, size_t capacity = 20);

        bool addItem(const Item& item);
        bool removeItem(size_t index);
        const std::vector<Item>& getItems() const;
        size_t getCapacity() const;
        void setCapacity(size_t newCapacity);

    private:
        std::vector<Item> items;
        size_t capacity;
    };

} // namespace FusionQuest

#endif // INVENTORY_COMPONENT_H