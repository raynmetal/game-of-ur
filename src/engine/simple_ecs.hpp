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

#include "util.hpp"
#include "registrator.hpp"

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
constexpr EntityID kMaxEntities { 300000 };

/*
  A simple integer, used as an index into the Signature bitset
to determine what components are available for an Entity, and what
components are required by a System
*/
using ComponentType = std::uint8_t;
constexpr ComponentType kMaxComponents { 255 };
using Signature = std::bitset<kMaxComponents>;

class BaseSystem;
class SystemManager;
class ComponentManager;
class Entity;
class SimpleECS;

class IComponentArray {
public:
    virtual ~IComponentArray()=default;
    virtual void handleEntityDestroyed(EntityID entityID)=0;
    virtual void handleFrameEnd() = 0;
    virtual void copyComponent(EntityID to, EntityID from)=0;
};

template<typename T>
class Interpolator {
public:
    T operator() (const T& previousState, const T& nextState, float simulationProgress=1.f) const;
private:
    RangeMapperLinear mProgressLimits {0.f, 1.f, 0.f, 1.f};
};

template<typename T>
class ComponentArray : public IComponentArray {
public:
private:
    void addComponent(EntityID entityID, const T& component);
    /**
     * Removes the component associated with a specific entity, maintaining
     * packing but not order.
     */
    void removeComponent(EntityID entityID);
    T getComponent(EntityID entityID, float simulationProgress=1.f) const;
    void updateComponent(EntityID entityID, const T& newValue);
    virtual void handleEntityDestroyed(EntityID entityID) override;
    virtual void handleFrameEnd() override;
    virtual void copyComponent(EntityID to, EntityID from) override;

    std::vector<T> mComponentsNext {};
    std::vector<T> mComponentsPrevious {};
    std::unordered_map<EntityID, std::size_t> mEntityToComponentIndex {};
    std::unordered_map<std::size_t, EntityID> mComponentToEntity {};
friend class ComponentManager;
};

class ComponentManager {
public:
private:
    ComponentManager() = default;

    template<typename T> 
    void registerComponentArray();

    template<typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray() const {
        const std::size_t componentHash { typeid(T).hash_code() };
        assert(mHashToComponentType.find(componentHash) != mHashToComponentType.end() && "This component type has not been registered");
        return std::dynamic_pointer_cast<ComponentArray<T>>(mHashToComponentArray.at(componentHash));
    }

    template<typename T>
    ComponentType getComponentType() const; 

    Signature getSignature(EntityID entityID);

    template<typename T>
    void addComponent(EntityID entityID, const T& component);

    template<typename T>
    void removeComponent(EntityID entityID);

    template<typename T>
    T getComponent(EntityID entityID, float simulationProgress=1.f) const;

    template<typename T>
    void updateComponent(EntityID entityID, const T& newValue);

    template<typename T>
    void copyComponent(EntityID to, EntityID from);

    void copyComponents(EntityID to, EntityID from);

    void handleEntityDestroyed(EntityID entityID);

    void handleFrameEnd();

    void unregisterAll();

    std::unordered_map<std::size_t, ComponentType> mHashToComponentType {};
    std::unordered_map<std::size_t, std::shared_ptr<IComponentArray>> mHashToComponentArray {};
    std::unordered_map<EntityID, Signature> mEntityToSignature {};

friend class SimpleECS;
};

class BaseSystem {
public:
    virtual ~BaseSystem() = default;

protected:
    BaseSystem() = default;
    const std::set<EntityID>& getEnabledEntities();

    template <typename TComponent, typename TSystem>
    TComponent getComponent(EntityID entityID, float progress=1.f) const;

    template <typename TComponent, typename TSystem>
    void updateComponent(EntityID entityID, const TComponent& component);

    bool isEnabled(EntityID entityID) const;
    bool isRegistered(EntityID entityID) const;

private:
    void addEntity(EntityID entityID, bool enabled=true);
    void removeEntity(EntityID entityID);

    void enableEntity(EntityID entityID);
    void disableEntity(EntityID entityID);

    virtual void onCreated(){};
    virtual void onDestroyed(){};
    virtual void onEntityEnabled(EntityID entityID){};
    virtual void onEntityDisabled(EntityID entityID){};
    virtual void onEntityUpdated(EntityID entityID){};

    std::set<EntityID> mEnabledEntities {};
    std::set<EntityID> mDisabledEntities {};

friend class SystemManager;
friend class SimpleECS;
};

template<typename TSystemDerived, typename ...TRequiredComponents>
class System: public BaseSystem {
    static void registerSelf();
protected:
    System(int explicitlyInitializeMe) {
        s_registrator.emptyFunc();
    }
    template<typename TComponent>
    TComponent getComponent(EntityID entityID, float progress=1.f) {
        return BaseSystem::getComponent<TComponent, TSystemDerived>(entityID, progress);
    }
    template<typename TComponent>
    void updateComponent(EntityID entityID, const TComponent& component) {
        BaseSystem::updateComponent<TComponent, TSystemDerived>(entityID, component);
    }
private:
    inline static Registrator<System<TSystemDerived, TRequiredComponents...>>& s_registrator {
        Registrator<System<TSystemDerived, TRequiredComponents...>>::getRegistrator()
    };
friend class Registrator<System<TSystemDerived, TRequiredComponents...>>;
};

class SystemManager {
public:
private: 
    SystemManager() = default;

    template<typename TSystem>
    void registerSystem(const Signature& signature);

    void unregisterAll();

    void initialize();

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

    void handleEntityUpdated(EntityID entityID, Signature signature);

    template<typename TSystem>
    void handleEntityUpdatedBySystem(EntityID entityID, Signature signature);

    std::unordered_map<std::size_t, Signature> mHashToSignature {};
    std::unordered_map<std::size_t, std::shared_ptr<BaseSystem>> mHashToSystem {};

friend class SimpleECS;
friend class BaseSystem;
};

class SimpleECS {
public:
    template<typename ...TComponent>
    static void registerComponentTypes();

    template<typename TSystem, typename ...TComponents>
    static void registerSystem();

    template<typename TSystem>
    static std::shared_ptr<TSystem> getSystem();

    template<typename ...TComponents>
    static Entity createEntity(TComponents...components);

    static void initialize();

    static void cleanup();

    static void endFrame();

private:
    static SimpleECS& getInstance();

    SimpleECS() = default;

    void copyComponents(EntityID to, EntityID from);

    template<typename ...TComponents>
    Entity privateCreateEntity(TComponents...components);

    void destroyEntity(EntityID entityID);

    template<typename TSystem>
    void enableEntity(EntityID entityID);

    template<typename TSystem>
    void disableEntity(EntityID entityID);

    template<typename TComponent>
    void addComponent(EntityID entityID, const TComponent& component);

    template<typename TComponent>
    TComponent getComponent(EntityID entityID, float simulationProgress=1.f) const;

    template<typename TComponent, typename TSystem>
    TComponent getComponent(EntityID entityID, float simulationProgress=1.f) const;

    template<typename TComponent>
    void updateComponent(EntityID entityID, const TComponent& newValue);

    template<typename TComponent, typename TSystem>
    void updateComponent(EntityID entityID, const TComponent& newValue);

    template<typename TComponent>
    void removeComponent(EntityID entityID);

    void removeComponentsAll(EntityID entityID);

    ComponentManager mComponentManager {};
    SystemManager mSystemManager {};

    std::vector<EntityID> mDeletedIDs {};
    EntityID mNextEntity {};

friend class Entity;
friend class BaseSystem;
};

class Entity {
public:
    Entity(const Entity& other);
    Entity(Entity&& other) noexcept;
    Entity& operator=(const Entity& other);
    Entity& operator=(Entity&& other) noexcept;

    ~Entity();

    inline EntityID getID() { return mID; };

    void copy(const Entity& other);

    template<typename TComponent> 
    void addComponent(const TComponent& component);

    template<typename TComponent> 
    void removeComponent();

    template<typename TComponent> 
    TComponent getComponent(float simulationProgress=1.f) const;

    template<typename T>
    void updateComponent(const T& newValue);

    template<typename T>
    void enableSystem();

    template<typename T>
    void disableSystem();

private:
    Entity(EntityID entityID): mID{ entityID } {};
    EntityID mID;

friend class SimpleECS;
};

template <typename TSystemDerived, typename ...TRequiredComponents>
void System<TSystemDerived, TRequiredComponents...>::registerSelf() {
    SimpleECS::registerComponentTypes<TRequiredComponents...>();
    SimpleECS::registerSystem<TSystemDerived, TRequiredComponents...>();
}

template<typename T>
T Interpolator<T>::operator() (const T& previousState, const T& nextState, float simulationProgress) const {
    // Clamp progress to acceptable values
    simulationProgress = mProgressLimits(simulationProgress);
    return simulationProgress * nextState + (1.f - simulationProgress) * previousState;
}

template<typename T>
void ComponentArray<T>::addComponent(EntityID entityID, const T& component) {
    assert(mEntityToComponentIndex.find(entityID) == mEntityToComponentIndex.end() && "Component already added for this entity");

    std::size_t newComponentID { mComponentsNext.size() };

    mComponentsNext.push_back(component);
    mComponentsPrevious.push_back(component);
    mEntityToComponentIndex[entityID] = newComponentID;
    mComponentToEntity[newComponentID] = entityID;
}

template<typename T>
void ComponentArray<T>::removeComponent(EntityID entityID) {
    assert(mEntityToComponentIndex.find(entityID) != mEntityToComponentIndex.end());

    const std::size_t removedComponentIndex { mEntityToComponentIndex[entityID] };
    const std::size_t lastComponentIndex { mComponentsNext.size() - 1 };
    const std::size_t lastComponentEntity { mComponentToEntity[lastComponentIndex] };

    // store last component in the removed components place
    mComponentsNext[removedComponentIndex] = mComponentsNext[lastComponentIndex];
    mComponentsPrevious[removedComponentIndex] = mComponentsPrevious[lastComponentIndex];
    // map the last component's entity to its new index
    mEntityToComponentIndex[lastComponentEntity] = removedComponentIndex;
    // map the removed component's index to the last entity
    mComponentToEntity[removedComponentIndex] = lastComponentEntity;

    // erase all traces of the removed entity's component and other
    // invalid references
    mComponentsNext.pop_back();
    mComponentsPrevious.pop_back();
    mEntityToComponentIndex.erase(entityID);
    mComponentToEntity.erase(lastComponentIndex);
}

template <typename T>
T ComponentArray<T>::getComponent(EntityID entityID, float simulationProgress) const {
    static Interpolator<T> interpolator{};
    assert(mEntityToComponentIndex.find(entityID) != mEntityToComponentIndex.end());
    std::size_t componentID { mEntityToComponentIndex.at(entityID) };
    return interpolator(mComponentsPrevious[componentID], mComponentsNext[componentID], simulationProgress);
}

template <typename T>
void ComponentArray<T>::updateComponent(EntityID entityID, const T& newComponent) {
    assert(mEntityToComponentIndex.find(entityID) != mEntityToComponentIndex.end());
    std::size_t componentID { mEntityToComponentIndex.at(entityID) };
    mComponentsPrevious[componentID] = mComponentsNext[componentID];
    mComponentsNext[componentID] = newComponent;
}

template <typename T>
void ComponentArray<T>::handleEntityDestroyed(EntityID entityID) {
    if(mEntityToComponentIndex.find(entityID) != mEntityToComponentIndex.end()) {
        removeComponent(entityID);
    }
}

template<typename TComponent>
void ComponentArray<TComponent>::handleFrameEnd() {
    for(std::size_t i{0}; i < mComponentsNext.size(); ++i) {
        mComponentsPrevious[i] = mComponentsNext[i];
    }
}

template <typename T>
void ComponentArray<T>::copyComponent(EntityID to, EntityID from) {
    if(mEntityToComponentIndex.find(from) != mEntityToComponentIndex.end()) {
        if(mEntityToComponentIndex.find(to) == mEntityToComponentIndex.end()) {
            // create a local copy of the component as container may be reallocated
            T componentValue { mComponentsNext[mEntityToComponentIndex[from]] };
            addComponent(to, componentValue); 
        } else {
            mComponentsNext[mEntityToComponentIndex[to]] = mComponentsNext[mEntityToComponentIndex[from]]; // overwriting existing values is fine
        }
        mComponentsPrevious[mEntityToComponentIndex[to]] = mComponentsPrevious[mEntityToComponentIndex[from]];
    }
}

template<typename T> 
void ComponentManager::registerComponentArray() {
    const std::size_t componentHash { typeid(T).hash_code() };
    if(mHashToComponentType.find(componentHash) != mHashToComponentType.end()) {
        return;
    }

    assert(mHashToComponentType.size() + 1 < kMaxComponents && "Component type limit reached");
    mHashToComponentArray.insert_or_assign(
        componentHash, std::static_pointer_cast<IComponentArray>(std::make_shared<ComponentArray<T>>())
    );
    mHashToComponentType[componentHash] = mHashToComponentType.size();
}

template<typename T>
ComponentType ComponentManager::getComponentType() const {
    const std::size_t componentHash { typeid(T).hash_code() };
    assert(mHashToComponentType.find(componentHash) != mHashToComponentType.end() && "Component type has not been registered");
    return mHashToComponentType.at(componentHash);
}

template<typename T>
void ComponentManager::addComponent(EntityID entityID, const T& component) {
    getComponentArray<T>()->addComponent(entityID, component);
    mEntityToSignature[entityID].set(getComponentType<T>(), true);
}

template<typename T>
void ComponentManager::removeComponent(EntityID entityID) {
    getComponentArray<T>()->removeComponent(entityID);
    mEntityToSignature[entityID].set(getComponentType<T>(), false);
}

template<typename T>
T ComponentManager::getComponent(EntityID entityID, float simulationProgress) const {
    return getComponentArray<T>()->getComponent(entityID, simulationProgress);
}

template<typename T>
void ComponentManager::updateComponent(EntityID entityID, const T& newValue) {
    getComponentArray<T>()->updateComponent(entityID, newValue);
}

template <typename T>
void ComponentManager::copyComponent(EntityID to, EntityID from) {
    assert(mEntityToSignature[from].test(getComponentType<T>()) && "The entity being copied from does not have this component");
    getComponentArray<T>()->copyComponent(to, from);
    mEntityToSignature[to].set(getComponentType<T>(), true);
}

template<typename TSystem>
void SystemManager::registerSystem(const Signature& signature) {
    std::size_t systemHash {typeid(TSystem).hash_code()};
    assert(mHashToSignature.find(systemHash) == mHashToSignature.end() && "System has already been registered");

    mHashToSignature[systemHash] = signature;
    mHashToSystem.insert_or_assign(systemHash, std::make_shared<TSystem>());
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

template <typename TComponent>
void Entity::addComponent(const TComponent& component) {
    SimpleECS::getInstance().addComponent<TComponent>(mID, component);
}

template<typename TComponent>
TComponent Entity::getComponent(float simulationProgress) const {
    return SimpleECS::getInstance().getComponent<TComponent>(mID, simulationProgress);
}

template<typename T>
void Entity::updateComponent(const T& newValue) {
    SimpleECS::getInstance().updateComponent<T>(mID, newValue);
}

template <typename T>
void Entity::removeComponent() {
    SimpleECS::getInstance().removeComponent<T>(mID);
}

template <typename T>
void Entity::enableSystem() {
    SimpleECS::getInstance().enableEntity<T>(mID);
}

template <typename T>
void Entity::disableSystem() {
    SimpleECS::getInstance().disableEntity<T>(mID);
}

template <typename TSystem>
bool SystemManager::isEnabled(EntityID entityID) {
    const std::size_t systemHash { typeid(TSystem).hash_code() };
    return mHashToSystem[systemHash]->isEnabled(entityID);
}

template<typename ...TComponents>
Entity SimpleECS::privateCreateEntity(TComponents...components) {
    assert((mNextEntity < kMaxEntities || !mDeletedIDs.empty()) && "Max number of entities reached");

    EntityID nextID;
    if(!mDeletedIDs.empty()){
        nextID = mDeletedIDs.back();
        mDeletedIDs.pop_back();
    } else {
        nextID = mNextEntity++;
    }

    Entity entity { nextID };

    (addComponent<TComponents>(nextID, components), ...);
    return entity;
}

template<typename TComponent>
void SimpleECS::addComponent(EntityID entityID, const TComponent& component) {
    assert(entityID < kMaxEntities && "Cannot add a component to an entity that does not exist");
    mComponentManager.addComponent<TComponent>(entityID, component);
    Signature signature { mComponentManager.getSignature(entityID) };
    mSystemManager.handleEntitySignatureChanged(entityID, signature);
}

template<typename TComponent>
void SimpleECS::removeComponent(EntityID entityID) {
    mComponentManager.removeComponent<TComponent>(entityID);
    Signature signature { mComponentManager.getSignature(entityID) };
    mSystemManager.handleEntitySignatureChanged(entityID, signature);
}

template<typename TSystem>
void SimpleECS::enableEntity(EntityID entityID) {
    mSystemManager.enableEntity<TSystem>(entityID);
}

template<typename TSystem>
void SimpleECS::disableEntity(EntityID entityID) {
    mSystemManager.disableEntity<TSystem>(entityID);
}

template<typename TComponent>
TComponent SimpleECS::getComponent(EntityID entityID, float progress) const {
    return mComponentManager.getComponent<TComponent>(entityID, progress);
}

template<typename TComponent, typename TSystem>
TComponent SimpleECS::getComponent(EntityID entityID, float progress) const {
    assert(
        (
            mSystemManager.mHashToSignature.at(typeid(TSystem).hash_code())
                .test(mComponentManager.getComponentType<TComponent>())
        )
        && "This system cannot access this kind of component"
    );
    return getComponent<TComponent>(entityID, progress);
}


template<typename TComponent>
void SimpleECS::updateComponent(EntityID entityID, const TComponent& newValue) {
    mComponentManager.updateComponent<TComponent>(entityID, newValue);
    mSystemManager.handleEntityUpdated(entityID, mComponentManager.getSignature(entityID));
}

template<typename TComponent, typename TSystem>
void SimpleECS::updateComponent(EntityID entityID, const TComponent& newValue) {
     assert(
        (
            mSystemManager.mHashToSignature.at(typeid(TSystem).hash_code())
                .test(mComponentManager.getComponentType<TComponent>())
        )
        && "This system cannot access this kind of component"
    );
    mComponentManager.updateComponent<TComponent>(entityID, newValue);
    mSystemManager.handleEntityUpdatedBySystem<TSystem>(entityID, mComponentManager.getSignature(entityID));
}

template<typename ...TComponent>
void SimpleECS::registerComponentTypes() {
    ((getInstance().mComponentManager.registerComponentArray<TComponent>()),...);
}

template<typename TSystem, typename ...TComponents>
void SimpleECS::registerSystem() {
    Signature signature {
        (
            (0x1u << getInstance().mComponentManager.getComponentType<TComponents>()) 
            | ... | 0x0u
        )
    };
    getInstance().mSystemManager.registerSystem<TSystem>(signature);
}

template<typename ...TComponents>
Entity SimpleECS::createEntity(TComponents...components) {
    return getInstance().privateCreateEntity<TComponents...>(components...);
}

template<typename TSystem>
std::shared_ptr<TSystem> SimpleECS::getSystem() {
    return getInstance().mSystemManager.getSystem<TSystem>();
}

template<typename TComponent, typename TSystem>
TComponent BaseSystem::getComponent(EntityID entityID, float progress) const {
    return SimpleECS::getInstance().getComponent<TComponent, TSystem>(entityID, progress);
}

template<typename TComponent, typename TSystem>
void BaseSystem::updateComponent(EntityID entityID, const TComponent& component) {
    SimpleECS::getInstance().updateComponent<TComponent, TSystem>(entityID, component);
}

template<typename TSystem>
void SystemManager::handleEntityUpdatedBySystem(EntityID entityID, Signature signature) {
    std::size_t originatingSystemHash { typeid(TSystem).hash_code() };
    for(auto& pair: mHashToSignature) {
        // suppress update callback from the system that caused this update
        if(pair.first == originatingSystemHash) continue;

        Signature& systemSignature { pair.second };
        if((systemSignature & signature) == systemSignature){
            BaseSystem& system { *(mHashToSystem[pair.first]).get() };
            system.onEntityUpdated(entityID);
        }
    }
}

#endif
