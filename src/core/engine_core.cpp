#include "engine_core.h"
#include <algorithm>
#include <iostream>

namespace FusionQuest {

// Event Manager implementation
void EventManager::subscribe(Event::Type type, EventCallback callback) {
    callbacks[type].push_back(std::move(callback));
}

void EventManager::unsubscribe(Event::Type type, size_t callbackId) {
    auto it = callbacks.find(type);
    if (it != callbacks.end() && callbackId < it->second.size()) {
        it->second.erase(it->second.begin() + callbackId);
    }
}

void EventManager::emit(const Event& event) {
    auto it = callbacks.find(event.getType());
    if (it != callbacks.end()) {
        for (const auto& callback : it->second) {
            callback(event);
        }
    }
}

// Engine implementation
Engine::Engine() = default;

Engine::~Engine() {
    shutdown();
}

void Engine::initialize() {
    if (running) {
        return;
    }
    
    // Initialize all systems
    for (auto& system : systems) {
        system->initialize();
    }
    
    running = true;
}

void Engine::update(float deltaTime) {
    if (!running) {
        return;
    }
    
    // Update all systems
    for (auto& system : systems) {
        system->update(deltaTime);
    }
    
    // Update all game objects
    for (auto& pair : gameObjects) {
        pair.second->update(deltaTime);
    }
}

void Engine::shutdown() {
    if (!running) {
        return;
    }
    
    // Clear all game objects
    gameObjects.clear();
    
    // Clear all systems
    systems.clear();
    
    running = false;
}

GameObject* Engine::createGameObject(const std::string& name) {
    auto gameObject = std::make_unique<GameObject>(nextObjectID++, name);
    GameObject* ptr = gameObject.get();
    gameObjects[ptr->getID()] = std::move(gameObject);
    return ptr;
}

void Engine::destroyGameObject(ObjectID id) {
    gameObjects.erase(id);
}

GameObject* Engine::findGameObject(ObjectID id) const {
    auto it = gameObjects.find(id);
    return (it != gameObjects.end()) ? it->second.get() : nullptr;
}

} // namespace FusionQuest