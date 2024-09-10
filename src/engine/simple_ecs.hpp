#ifndef ZOSIMPLEECS_H
#define ZOSIMPLEECS_H

#include <cstdint>
#include <typeinfo>
#include <memory>
#include <vector>
#include <set>
#include <bitset>
#include <unordered_map>
#include <set>

/* 
  See Austin Morlan's implementation of a simple ECS, where entities are
simple indices into various component arrays, each of which are kept tightly
packed:
    https://austinmorlan.com/posts/entity_component_system/
*/

/*
  An entity ID. An entity class takes the place of the coordinator
class from Morlan's implementation
*/
using EntityID = std::uint64_t;
constexpr EntityID kMaxEntities { 30000 };

/*
  A simple integer, used as an index into the Signature bitset
to determine what components are available for an Entity, and what
components are required by a System
*/
using ComponentType = std::uint8_t;
constexpr ComponentType kMaxComponents { 255 };
using Signature = std::bitset<kMaxComponents>;

class System;
class SystemManager;
class ComponentManager;
class Entity;

extern ComponentManager gComponentManager;
extern SystemManager gSystemManager;

class IComponentArray {
public:
    virtual void handleEntityDestroyed(EntityID entityID) = 0;
    virtual void copyComponent(EntityID to, EntityID from) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray {
public:
    void addComponent(EntityID entityID, T component);
    void removeComponent(EntityID entityID);
    T& getComponent(EntityID entityID);
    virtual void handleEntityDestroyed(EntityID entityID) override;
    virtual void copyComponent(EntityID to, EntityID from) override;

private:
    std::vector<T> mComponents {};
    std::unordered_map<EntityID, std::size_t> mEntityToComponentIndex {};
    std::unordered_map<std::size_t, EntityID> mComponentToEntity {};
};

class ComponentManager {
public:
    template<typename T> 
    void registerComponentArray();

    template<typename T>
    ComponentType getComponentType();

    Signature getSignature(EntityID entityID);

    template<typename T>
    void addComponent(EntityID entityID, T component);

    template<typename T>
    void removeComponent(EntityID entityID);

    template<typename T>
    T& getComponent(EntityID entityID);

    template<typename T>
    void copyComponent(EntityID to, EntityID from);

    void copyComponents(EntityID to, EntityID from);

    void handleEntityDestroyed(EntityID entityID);

    void unregisterAll();

private:
    std::unordered_map<std::size_t, ComponentType> mHashToComponentType {};
    std::unordered_map<std::size_t, std::shared_ptr<IComponentArray>> mHashToComponentArray {};
    std::unordered_map<EntityID, Signature> mEntityToSignature {};

    template<typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray() {
        const std::size_t componentHash { typeid(T).hash_code() };
        assert(mHashToComponentType.find(componentHash) != mHashToComponentType.end() && "This component type has not been registered");
        return std::dynamic_pointer_cast<ComponentArray<T>>(mHashToComponentArray[componentHash]);
    }
};

class System {
public:
    virtual ~System() = default;

    void enableEntity(EntityID entityID);
    void disableEntity(EntityID entityID);
    void addEntity(EntityID entityID, bool enabled=true);
    void removeEntity(EntityID entityID);

    bool isEnabled(EntityID entityID);

protected:
    const std::set<EntityID>& getEnabledEntities();

private:
    std::set<EntityID> mEnabledEntities {};
    std::set<EntityID> mDisabledEntities {};
};

class SystemManager {
public:
    template<typename TSystem>
    void registerSystem(Signature signature);

    void unregisterAll();

    template<typename TSystem>
    std::shared_ptr<TSystem> getSystem();

    template<typename TSystem>
    void enableEntity(EntityID entityID);

    template<typename TSystem>
    void disableEntity(EntityID entityID);

    template<typename TSystem>
    bool isEnabled(EntityID entityID);

    void handleEntitySignatureChanged(EntityID entityID, Signature signature);

    void handleEntityDestroyed(EntityID entityID);

private:
    std::unordered_map<std::size_t, Signature> mHashToSignature {};
    std::unordered_map<std::size_t, std::shared_ptr<System>> mHashToSystem {};
};

class Entity {
public:

    Entity();

    Entity(const Entity& other);
    Entity(Entity&& other);
    Entity& operator=(const Entity& other);
    Entity& operator=(Entity&& other);

    ~Entity();

    inline EntityID getID() { return mID; };

    Entity& copy(const Entity& other);

    template<typename T> 
    void addComponent(const T& component);

    template<typename T> 
    void removeComponent();

    template<typename T> 
    T& getComponent();

    template<typename T>
    void enableSystem();

    template<typename T>
    void disableSystem();

private:
    EntityID mID;

    static std::vector<EntityID> sDeletedIDs;
    static EntityID sNextEntity;

    void createEntity();
};

template<typename T>
void ComponentArray<T>::addComponent(EntityID entityID, T component) {
    assert(mEntityToComponentIndex.find(entityID) == mEntityToComponentIndex.end() && "Component already added for this entity");

    std::size_t newComponentID { mComponents.size() };

    mComponents.push_back(component);
    mEntityToComponentIndex[entityID] = newComponentID;
    mComponentToEntity[newComponentID] = entityID;
}

template<typename T>
void ComponentArray<T>::removeComponent(EntityID entityID) {
    assert(mEntityToComponentIndex.find(entityID) != mEntityToComponentIndex.end());

    const std::size_t removedComponentIndex { mEntityToComponentIndex[entityID] };
    const std::size_t lastComponentIndex { mComponents.size() - 1 };
    const std::size_t lastComponentEntity { mComponentToEntity[lastComponentIndex] };

    // store last component in the removed components place
    mComponents[removedComponentIndex] = mComponents[lastComponentIndex];
    // map the last component's entity to its new index
    mEntityToComponentIndex[lastComponentEntity] = removedComponentIndex;
    // map the removed component's index to the last entity
    mComponentToEntity[removedComponentIndex] = lastComponentEntity;

    // erase all traces of the removed entity's component and other
    // invalid references
    mComponents.pop_back();
    mEntityToComponentIndex.erase(entityID);
    mComponentToEntity.erase(lastComponentIndex);
}

template <typename T>
T& ComponentArray<T>::getComponent(EntityID entityID) {
    assert(mEntityToComponentIndex.find(entityID) != mEntityToComponentIndex.end());
    std::size_t componentID { mEntityToComponentIndex.at(entityID) };
    return mComponents[componentID];
}

template <typename T>
void ComponentArray<T>::handleEntityDestroyed(EntityID entityID) {
    if(mEntityToComponentIndex.find(entityID) != mEntityToComponentIndex.end()) {
        removeComponent(entityID);
    }
}

template <typename T>
void ComponentArray<T>::copyComponent(EntityID to, EntityID from) {
    if(mEntityToComponentIndex.find(from) != mEntityToComponentIndex.end()) {
        if(mEntityToComponentIndex.find(to) == mEntityToComponentIndex.end()) {
            addComponent(to, mComponents[mEntityToComponentIndex[from]]);
        } else {
            mComponents[mEntityToComponentIndex[to]] = mComponents[mEntityToComponentIndex[from]];
        }
    }
}

template<typename T> 
void ComponentManager::registerComponentArray() {
    const std::size_t componentHash { typeid(T).hash_code() };
    assert(mHashToComponentType.find(componentHash) == mHashToComponentType.end() && "Component type already registered");
    assert(mHashToComponentType.size() + 1 < kMaxComponents && "Component type limit reached");
    mHashToComponentArray.insert_or_assign(
        componentHash, std::static_pointer_cast<IComponentArray>(std::make_shared<ComponentArray<T>>())
    );
    mHashToComponentType[componentHash] = mHashToComponentType.size();
}

template<typename T>
ComponentType ComponentManager::getComponentType() {
    const std::size_t componentHash { typeid(T).hash_code() };
    assert(mHashToComponentType.find(componentHash) != mHashToComponentType.end() && "Component type has not been registered");
    return mHashToComponentType[componentHash];
}

template<typename T>
void ComponentManager::addComponent(EntityID entityID, T component) {
    getComponentArray<T>()->addComponent(entityID, component);
    mEntityToSignature[entityID].set(getComponentType<T>(), true);
}

template<typename T>
void ComponentManager::removeComponent(EntityID entityID) {
    getComponentArray<T>()->removeComponent(entityID);
    mEntityToSignature[entityID].set(getComponentType<T>(), false);
}

template<typename T>
T& ComponentManager::getComponent(EntityID entityID) {
    return getComponentArray<T>()->getComponent(entityID);
}

template <typename T>
void ComponentManager::copyComponent(EntityID to, EntityID from) {
    assert(mEntityToSignature[from].test(getComponentType<T>()) && "The entity being copied from does not have this component");
    getComponentArray<T>()->copyComponent(to, from);
    mEntityToSignature[to].set(getComponentType<T>(), true);
}

template<typename T>
void SystemManager::registerSystem(Signature signature) {
    std::size_t systemHash {typeid(T).hash_code()};
    assert(mHashToSignature.find(systemHash) == mHashToSignature.end() && "System has already been registered");
    mHashToSignature[systemHash] = signature;
    mHashToSystem.insert_or_assign(systemHash, std::make_shared<T>());
}

template<typename T>
std::shared_ptr<T> SystemManager::getSystem() {
    std::size_t systemHash {typeid(T).hash_code()};
    assert(mHashToSignature.find(systemHash) != mHashToSignature.end() && "System has not yet been registered");
    return std::dynamic_pointer_cast<T>(mHashToSystem[systemHash]);
}

template<typename T>
void SystemManager::enableEntity(EntityID entityID) {
    std::size_t systemHash {typeid(T).hash_code()};
    mHashToSystem[systemHash]->enableEntity(entityID);
}

template<typename T>
void SystemManager::disableEntity(EntityID entityID) {
    std::size_t systemHash{ typeid(T).hash_code() };
    mHashToSystem[systemHash]->disableEntity(entityID);
}

template <typename T>
void Entity::addComponent(const T& component) {
    gComponentManager.addComponent<T>(mID, component);
    Signature signature { gComponentManager.getSignature(mID) };
    gSystemManager.handleEntitySignatureChanged(mID, signature);
}

template<typename T>
T& Entity::getComponent() {
    return gComponentManager.getComponent<T>(mID);
}

template <typename T>
void Entity::removeComponent() {
    gComponentManager.removeComponent<T>(mID);
    Signature signature { gComponentManager.getSignature(mID) };
    gSystemManager.handleEntitySignatureChanged(mID, signature);
}

template <typename T>
void Entity::enableSystem() {
    gSystemManager.enableEntity<T>(mID);
}

template <typename T>
void Entity::disableSystem() {
    gSystemManager.disableEntity<T>(mID);
}

template <typename TSystem>
bool SystemManager::isEnabled(EntityID entityID) {
    const std::size_t systemHash { typeid(TSystem).hash_code() };
    return mHashToSystem[systemHash]->isEnabled(entityID);
}

#endif
