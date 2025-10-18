#include "engine_core.h"
#include <algorithm>
#include <iostream>

namespace FusionQuest {

// ----------------------------------------------------
// GameEvent
// ----------------------------------------------------
GameEvent::GameEvent(Type type) : type(type) {}
GameEvent::~GameEvent() = default;
GameEvent::Type GameEvent::getType() const { return type; }

// ----------------------------------------------------
// EventManager
// ----------------------------------------------------
void EventManager::subscribe(GameEvent::Type type, EventCallback callback) {
    callbacks[type].push_back(std::move(callback));
}

void EventManager::unsubscribe(GameEvent::Type type, size_t callbackId) {
    // Simple placeholder - actual unsubscribe logic can be implemented if needed
    if (callbacks.find(type) != callbacks.end() && callbackId < callbacks[type].size()) {
        callbacks[type].erase(callbacks[type].begin() + callbackId);
    }
}

void EventManager::emit(const GameEvent& event) {
    auto it = callbacks.find(event.getType());
    if (it != callbacks.end()) {
        for (auto& cb : it->second) {
            cb(event);
        }
    }
}

// ----------------------------------------------------
// Component
// ----------------------------------------------------
Component::Component(GameObject* owner) : owner(owner) {}
Component::~Component() = default;

void Component::initialize() {}
void Component::update(float deltaTime) {}
GameObject* Component::getOwner() const { return owner; }

// ----------------------------------------------------
// GameObject
// ----------------------------------------------------
GameObject::GameObject(ObjectID id, const std::string& name)
    : id(id), name(name) {}

GameObject::~GameObject() = default;

void GameObject::update(float deltaTime) {
    for (auto& component : components) {
        component->update(deltaTime);
    }
}

ObjectID GameObject::getID() const { return id; }
const std::string& GameObject::getName() const { return name; }
void GameObject::setName(const std::string& newName) { name = newName; }

// ----------------------------------------------------
// System
// ----------------------------------------------------
void System::initialize() {}
void System::update(float deltaTime) {}

// ----------------------------------------------------
// Engine
// ----------------------------------------------------
Engine::Engine() = default;
Engine::~Engine() = default;

void Engine::initialize() {
    running = true;
    for (auto& system : systems) {
        system->initialize();
    }
}

void Engine::update(float deltaTime) {
    if (!running) return;

    for (auto& system : systems) {
        system->update(deltaTime);
    }

    for (auto& [id, object] : gameObjects) {
        object->update(deltaTime);
    }
}

void Engine::shutdown() {
    running = false;
    gameObjects.clear();
    systems.clear();
}

GameObject* Engine::createGameObject(const std::string& name) {
    ObjectID id = nextObjectID++;
    auto gameObject = std::make_unique<GameObject>(id, name);
    GameObject* objPtr = gameObject.get();
    gameObjects[id] = std::move(gameObject);
    return objPtr;
}

void Engine::destroyGameObject(ObjectID id) {
    gameObjects.erase(id);
}

GameObject* Engine::findGameObject(ObjectID id) const {
    auto it = gameObjects.find(id);
    return it != gameObjects.end() ? it->second.get() : nullptr;
}

EventManager& Engine::getEventManager() { return eventManager; }

} // namespace FusionQuest

