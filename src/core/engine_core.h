#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace FusionQuest {

// Forward declarations
class GameObject;
class Component;
class System;
class EventManager;

// Unique identifier for game objects
using ObjectID = size_t;

// ----------------------------------------------------
// GameEvent
// ----------------------------------------------------
class GameEvent {
public:
    enum class Type {
        None,
        StatChanged,
        ItemAcquired,
        QuestUpdated,
        PlayerLevelUp,
        // Add more event types as needed
    };

    explicit GameEvent(Type type);
    virtual ~GameEvent();

    Type getType() const;

private:
    Type type;
};

// Event callback function type
using EventCallback = std::function<void(const GameEvent&)>;

// ----------------------------------------------------
// EventManager
// ----------------------------------------------------
class EventManager {
public:
    void subscribe(GameEvent::Type type, EventCallback callback);
    void unsubscribe(GameEvent::Type type, size_t callbackId);
    void emit(const GameEvent& event);

private:
    std::unordered_map<GameEvent::Type, std::vector<EventCallback>> callbacks;
};

// ----------------------------------------------------
// Component
// ----------------------------------------------------
class Component {
public:
    explicit Component(GameObject* owner);
    virtual ~Component();

    virtual void initialize();
    virtual void update(float deltaTime);

    GameObject* getOwner() const;

private:
    GameObject* owner;
};

// ----------------------------------------------------
// GameObject
// ----------------------------------------------------
class GameObject {
public:
    GameObject(ObjectID id, const std::string& name = "");
    ~GameObject();

    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        auto component = std::make_unique<T>(this, std::forward<Args>(args)...);
        T* componentPtr = component.get();
        components.push_back(std::move(component));
        return componentPtr;
    }

    template<typename T>
    T* getComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        for (const auto& component : components) {
            if (T* derived = dynamic_cast<T*>(component.get())) {
                return derived;
            }
        }
        return nullptr;
    }

    void update(float deltaTime);
    ObjectID getID() const;
    const std::string& getName() const;
    void setName(const std::string& newName);

private:
    ObjectID id;
    std::string name;
    std::vector<std::unique_ptr<Component>> components;
};

// ----------------------------------------------------
// System
// ----------------------------------------------------
class System {
public:
    virtual ~System() = default;
    virtual void initialize();
    virtual void update(float deltaTime);
};

// ----------------------------------------------------
// Engine
// ----------------------------------------------------
class Engine {
public:
    Engine();
    ~Engine();

    void initialize();
    void update(float deltaTime);
    void shutdown();

    template<typename T, typename... Args>
    T* addSystem(Args&&... args) {
        static_assert(std::is_base_of<System, T>::value, "T must derive from System");

        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* systemPtr = system.get();
        systems.push_back(std::move(system));
        return systemPtr;
    }

    GameObject* createGameObject(const std::string& name = "");
    void destroyGameObject(ObjectID id);
    GameObject* findGameObject(ObjectID id) const;

    EventManager& getEventManager();

private:
    std::vector<std::unique_ptr<System>> systems;
    std::unordered_map<ObjectID, std::unique_ptr<GameObject>> gameObjects;
    ObjectID nextObjectID = 1;
    EventManager eventManager;
    bool running = false;
};

} // namespace FusionQuest