#ifndef ZOECSWORLD_H
#define ZOECSWORLD_H

#include <cstdint>
#include <typeinfo>
#include <iostream>
#include <memory>
#include <vector>
#include <set>
#include <bitset>
#include <unordered_map>
#include <type_traits>
#include <set>

#include <nlohmann/json.hpp>

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
using WorldID = std::uint64_t;
using UniversalEntityID = std::pair<WorldID, EntityID>;
using ECSType = std::uint8_t;
using ComponentType = ECSType;
using SystemType = ECSType;
constexpr EntityID kMaxEntities { 1000000 };
/*
  A simple integer, used as an index into the Signature bitset
to determine what components are available for an Entity, and what
components are required by a System
*/
constexpr ECSType kMaxECSTypes { 255 };
constexpr ComponentType kMaxComponents { kMaxECSTypes };
constexpr SystemType kMaxSystems { kMaxECSTypes };
using Signature = std::bitset<kMaxComponents>;

class BaseSystem;
class SystemManager;
class ComponentManager;
class Entity;
class ECSWorld;

class BaseComponentArray {
public:
    BaseComponentArray(std::weak_ptr<ECSWorld> world): mWorld{world} {}
    virtual ~BaseComponentArray()=default;
    virtual void handleEntityDestroyed(EntityID entityID)=0;
    virtual void handlePreSimulationStep() = 0;
    virtual void copyComponent(EntityID to, EntityID from)=0;
    virtual void copyComponent(EntityID to, EntityID from, BaseComponentArray& other) = 0;
    virtual void addComponent(EntityID to, const nlohmann::json& jsonComponent)=0;
    virtual bool hasComponent(EntityID entityID) const=0;
    virtual std::shared_ptr<BaseComponentArray> instantiate(std::weak_ptr<ECSWorld> world) const = 0;

protected:
    std::weak_ptr<ECSWorld> mWorld {};
};

/*
Specialization example:
```c++
template <typename TComponent>
struct ComponentFromJSON<
    std::shared_ptr<TComponent>,
    typename std::enable_if<
        std::is_base_of<BaseClass, TComponent>::value
    >::type> {
    
    static std::shared_ptr<TComponent> get(const nlohmann::json& jsonComponent) {
        // your custom way for constructing/retrieving a component
        // based on json
        return component;
    }
};
```
*/
template <typename TComponent, typename Enable=void>
struct ComponentFromJSON {
    static TComponent get(const nlohmann::json& jsonComponent){
        // in the regular case, just invoke the from_json method that the
        // author of the component has presumably implemented
        TComponent component = jsonComponent;
        return component;
    }
};

template <typename TComponent, typename Enable>
struct ComponentFromJSON<std::shared_ptr<TComponent>, Enable> {
    static std::shared_ptr<TComponent> get(const nlohmann::json& jsonComponent) {
        // assume once again that the author of the component has provided
        // a from_json function that will be invoked here
        std::shared_ptr<TComponent> component { new TComponent{} = jsonComponent };
        return component;
    }
};

template<typename T>
class Interpolator {
public:
    T operator() (const T& previousState, const T& nextState, float simulationProgress=1.f) const;
private:
    RangeMapperLinear mProgressLimits {0.f, 1.f, 0.f, 1.f};
};

template<typename TComponent>
class ComponentArray : public BaseComponentArray {
public:
    ComponentArray(std::weak_ptr<ECSWorld> world): BaseComponentArray{ world } {}
private:
    std::shared_ptr<BaseComponentArray> instantiate(std::weak_ptr<ECSWorld> world) const override;
    void addComponent(EntityID entityID, const TComponent& component);
    void addComponent(EntityID entityID, const nlohmann::json& componentJSON) override;
    /**
     * Removes the component associated with a specific entity, maintaining
     * packing but not order.
     */
    void removeComponent(EntityID entityID);
    TComponent getComponent(EntityID entityID, float simulationProgress=1.f) const;
    bool hasComponent(EntityID entityID) const override;
    void updateComponent(EntityID entityID, const TComponent& newValue);
    virtual void handleEntityDestroyed(EntityID entityID) override;
    virtual void handlePreSimulationStep() override;
    virtual void copyComponent(EntityID to, EntityID from) override;
    virtual void copyComponent(EntityID to, EntityID from, BaseComponentArray& other) override;

    std::vector<TComponent> mComponentsNext {};
    std::vector<TComponent> mComponentsPrevious {};
    std::unordered_map<EntityID, std::size_t> mEntityToComponentIndex {};
    std::unordered_map<std::size_t, EntityID> mComponentToEntity {};
friend class ComponentManager;
};

class ComponentManager {
public:
    ComponentManager(std::weak_ptr<ECSWorld> world): mWorld { world } {};
private:
    ComponentManager instantiate(std::weak_ptr<ECSWorld> world) const;

    template<typename TComponent> 
    void registerComponentArray();

    template <typename TComponent>
    struct getComponentTypeName {
        std::string operator()() {
            return TComponent::getComponentTypeName();
        }
    };

    template <typename TComponent>
    struct getComponentTypeName<std::shared_ptr<TComponent>> {
        std::string operator()() {
            return TComponent::getComponentTypeName();
        }
    };

    template<typename TComponent>
    std::shared_ptr<ComponentArray<TComponent>> getComponentArray() const {
        const std::size_t componentHash { typeid(TComponent).hash_code() };
        assert(mHashToComponentType.find(componentHash) != mHashToComponentType.end() && "This component type has not been registered");
        return std::dynamic_pointer_cast<ComponentArray<TComponent>>(mHashToComponentArray.at(componentHash));
    }

    template<typename TComponent>
    ComponentType getComponentType() const; 

    Signature getSignature(EntityID entityID);

    template<typename TComponent>
    void addComponent(EntityID entityID, const TComponent& component);

    void addComponent(EntityID entityID, const nlohmann::json& jsonComponent);

    template<typename TComponent>
    void removeComponent(EntityID entityID);

    template<typename TComponent>
    bool hasComponent(EntityID entityID) const;

    template<typename TComponent>
    TComponent getComponent(EntityID entityID, float simulationProgress=1.f) const;

    template<typename TComponent>
    void updateComponent(EntityID entityID, const TComponent& newValue);

    template<typename TComponent>
    void copyComponent(EntityID to, EntityID from);

    void copyComponents(EntityID to, EntityID from);
    void copyComponents(EntityID to, EntityID from, ComponentManager& other);

    void handleEntityDestroyed(EntityID entityID);

    void handlePreSimulationStep();

    void unregisterAll();

    std::unordered_map<std::string, std::size_t> mNameToComponentHash {};
    std::unordered_map<std::size_t, ComponentType> mHashToComponentType {};
    std::unordered_map<std::size_t, std::shared_ptr<BaseComponentArray>> mHashToComponentArray {};
    std::unordered_map<EntityID, Signature> mEntityToSignature {};
    std::weak_ptr<ECSWorld> mWorld;

friend class ECSWorld;
};

class BaseSystem : public std::enable_shared_from_this<BaseSystem> {
public:
    BaseSystem(std::weak_ptr<ECSWorld> world): mWorld { world } {}
    virtual ~BaseSystem() = default;
    virtual bool isSingleton() const { return false; }
protected:
    const std::set<EntityID>& getEnabledEntities();

    template <typename TComponent, typename TSystem>
    TComponent getComponent(EntityID entityID, float progress=1.f) const;

    template <typename TComponent, typename TSystem>
    void updateComponent(EntityID entityID, const TComponent& component);

    bool isEnabled(EntityID entityID) const;
    bool isRegistered(EntityID entityID) const;


    virtual std::shared_ptr<BaseSystem> instantiate(std::weak_ptr<ECSWorld> world) = 0;

    std::weak_ptr<ECSWorld> mWorld;
private:
    void addEntity(EntityID entityID, bool enabled=true);
    void removeEntity(EntityID entityID);

    void enableEntity(EntityID entityID);
    void disableEntity(EntityID entityID);

    virtual void onEntityEnabled(EntityID entityID) {}
    virtual void onEntityDisabled(EntityID entityID) {}
    virtual void onEntityUpdated(EntityID entityID) {}

    virtual void onInitialize() {}
    virtual void onSimulationActivated() {}
    virtual void onSimulationPreStep(uint32_t simStepMillis) {}
    virtual void onSimulationStep(uint32_t simStepMillis) {}
    virtual void onVariableStep(float simulationProgress, uint32_t variableStepMillis) {}
    virtual void onPreRenderStep(float simulationProgress) {}
    virtual void onPostRenderStep(float simulationProgress) {}
    virtual void onSimulationDeactivated() {}
    virtual void onDestroyed()  {}

    std::set<EntityID> mEnabledEntities {};
    std::set<EntityID> mDisabledEntities {};

friend class SystemManager;
friend class ECSWorld;
};

template<typename TSystemDerived, typename ...TRequiredComponents>
class System: public BaseSystem {
    static void registerSelf();
protected:
    System(std::weak_ptr<ECSWorld> world): BaseSystem { world } { s_registrator.emptyFunc(); }
    template<typename TComponent>
    TComponent getComponent(EntityID entityID, float progress=1.f) {
        assert(!isSingleton() && "Singletons cannot retrieve components by EntityID alone");
        return BaseSystem::getComponent<TComponent, TSystemDerived>(entityID, progress);
    }
    template<typename TComponent>
    void updateComponent(EntityID entityID, const TComponent& component) {
        assert(!isSingleton() && "Singletons cannot retrieve components by EntityID alone");
        BaseSystem::updateComponent<TComponent, TSystemDerived>(entityID, component);
    }
    std::shared_ptr<BaseSystem> instantiate(std::weak_ptr<ECSWorld> world) override;
private:
    inline static Registrator<System<TSystemDerived, TRequiredComponents...>>& s_registrator {
        Registrator<System<TSystemDerived, TRequiredComponents...>>::getRegistrator()
    };

friend class Registrator<System<TSystemDerived, TRequiredComponents...>>;
};

class SystemManager {
public:
    SystemManager(std::weak_ptr<ECSWorld> world): mWorld{ world } {}
private: 
    SystemManager instantiate(std::weak_ptr<ECSWorld> world) const;

    template<typename TSystem>
    void registerSystem(const Signature& signature);

    void unregisterAll();


    template<typename TSystem>
    std::shared_ptr<TSystem> getSystem();

    template<typename TSystem>
    void enableEntity(EntityID entityID);

    void enableEntity(EntityID entityID, Signature entitySignature, Signature systemMask = Signature{}.set());

    template<typename TSystem>
    void disableEntity(EntityID entityID);

    void disableEntity(EntityID entityID, Signature entitySignature);

    template<typename TSystem>
    SystemType getSystemType() const;

    template<typename TSystem>
    bool isEnabled(EntityID entityID);

    template <typename TSystem>
    bool isRegistered(EntityID entityID);

    void handleEntitySignatureChanged(EntityID entityID, Signature signature);
    void handleEntityDestroyed(EntityID entityID);
    void handleEntityUpdated(EntityID entityID, Signature signature);
    template<typename TSystem>
    void handleEntityUpdatedBySystem(EntityID entityID, Signature signature);

    void handleInitialize();
    void handleSimulationActivated();
    void handleSimulationPreStep(uint32_t simStepMillis);
    void handleSimulationStep(uint32_t simStepMillis);
    void handleVariableStep(float simulationProgress, uint32_t variableStepMillis);
    void handlePreRenderStep(float simulationProgress);
    void handlePostRenderStep(float simulationProgress);
    void handleSimulationDeactivated();

    std::unordered_map<std::string, Signature> mNameToSignature {};
    std::unordered_map<std::string, SystemType> mNameToSystemType {};
    std::unordered_map<std::string, std::shared_ptr<BaseSystem>> mNameToSystem {};
    std::weak_ptr<ECSWorld> mWorld;

friend class ECSWorld;
friend class BaseSystem;
};

class ECSWorld: public std::enable_shared_from_this<ECSWorld> {
public:
    static std::weak_ptr<const ECSWorld> getPrototype();
    std::shared_ptr<ECSWorld> instantiate() const;

    template<typename ...TComponent>
    static void registerComponentTypes();

    template<typename TSystem, typename ...TComponents>
    static void registerSystem();

    template<typename TSystem>
    std::shared_ptr<TSystem> getSystem();

    template<typename TSystem>
    static std::shared_ptr<TSystem> getSystemPrototype();

    template <typename TSingletonSystem>
    static std::shared_ptr<TSingletonSystem> getSingletonSystem();

    template <typename TSystem>
    SystemType getSystemType();

    template <typename TSystem>
    bool isEnabled(EntityID entityID);

    template <typename TSystem>
    bool isRegistered(EntityID entityID);

    template<typename ...TComponents>
    Entity createEntity(TComponents...components);
    template <typename ...TComponents>
    static Entity createEntityPrototype(TComponents...components);

    // Simulation lifecycle events
    void initialize();
    void activateSimulation();
    void deactivateSimulation();

    // Simulation loop events
    void preSimulationStep(uint32_t simStepMillis);
    void simulationStep(uint32_t simStepMillis);
    void variableStep(float simulationProgress, uint32_t variableStepMillis);
    void preRenderStep(float simulationProgress);
    void postRenderStep(float simulationProgress);


    void cleanup();
    inline WorldID getID() const { return mID; }

private:
    static std::shared_ptr<ECSWorld> createWorld();
    static std::weak_ptr<ECSWorld> getInstance();
    ECSWorld() = default;

    void copyComponents(EntityID to, EntityID from);
    void copyComponents(EntityID to, EntityID from, ECSWorld& other);

    void relocateEntity(Entity& entity);

    template<typename ...TComponents>
    Entity privateCreateEntity(TComponents...components);

    void destroyEntity(EntityID entityID);

    template<typename TSystem>
    void enableEntity(EntityID entityID);

    void enableEntity(EntityID entityID, Signature systemMask = Signature{}.set());

    template<typename TSystem>
    void disableEntity(EntityID entityID);

    void disableEntity(EntityID entityID);

    template<typename TComponent>
    void addComponent(EntityID entityID, const TComponent& component);

    void addComponent(EntityID entityID, const nlohmann::json& jsonComponent);

    template<typename TComponent>
    TComponent getComponent(EntityID entityID, float simulationProgress=1.f) const;

    template<typename TComponent, typename TSystem>
    TComponent getComponent(EntityID entityID, float simulationProgress=1.f) const;

    template <typename TComponent>
    bool hasComponent(EntityID entityID) const;

    template<typename TComponent>
    void updateComponent(EntityID entityID, const TComponent& newValue);

    template<typename TComponent, typename TSystem>
    void updateComponent(EntityID entityID, const TComponent& newValue);

    template<typename TComponent>
    void removeComponent(EntityID entityID);

    void removeComponentsAll(EntityID entityID);

    std::unique_ptr<ComponentManager> mComponentManager {nullptr};
    std::unique_ptr<SystemManager> mSystemManager {nullptr};

    std::vector<EntityID> mDeletedIDs {};
    EntityID mNextEntity {};
    WorldID mID {0};

    static WorldID s_nextWorld;

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

    void addComponent(const nlohmann::json& jsonComponent);

    template<typename TComponent> 
    void removeComponent();

    template<typename TComponent>
    bool hasComponent() const;

    template<typename TComponent> 
    TComponent getComponent(float simulationProgress=1.f) const;

    template<typename TComponent>
    void updateComponent(const TComponent& newValue);

    template<typename TSystem>
    bool isEnabled() const;

    template <typename TSystem>
    bool isRegistered() const;

    template<typename TSystem>
    void enableSystem();

    template<typename TSystem>
    void disableSystem();

    void disableSystems();
    void enableSystems(Signature systemMask);
    inline std::weak_ptr<ECSWorld> getWorld() { return mWorld; }
    void joinWorld(ECSWorld& world);

private:
    Entity(EntityID entityID, std::shared_ptr<ECSWorld> world): mID{ entityID }, mWorld{ world } {};
    EntityID mID;

    std::weak_ptr<ECSWorld> mWorld;
friend class ECSWorld;
};

template<typename T>
T Interpolator<T>::operator() (const T& previousState, const T& nextState, float simulationProgress) const {
    // Clamp progress to acceptable values
    simulationProgress = mProgressLimits(simulationProgress);
    return simulationProgress * nextState + (1.f - simulationProgress) * previousState;
}

template<typename TComponent>
void ComponentArray<TComponent>::addComponent(EntityID entityID, const TComponent& component) {
    assert(mEntityToComponentIndex.find(entityID) == mEntityToComponentIndex.end() && "Component already added for this entity");

    std::size_t newComponentID { mComponentsNext.size() };

    mComponentsNext.push_back(component);
    mComponentsPrevious.push_back(component);
    mEntityToComponentIndex[entityID] = newComponentID;
    mComponentToEntity[newComponentID] = entityID;
}

template <typename TComponent>
void ComponentArray<TComponent>::addComponent(EntityID entityID, const nlohmann::json& jsonComponent) {
    addComponent(entityID, ComponentFromJSON<TComponent>::get(jsonComponent));
}

template<typename TComponent>
bool ComponentArray<TComponent>::hasComponent(EntityID entityID) const {
    return mEntityToComponentIndex.find(entityID) != mEntityToComponentIndex.end();
}

template<typename TComponent>
void ComponentArray<TComponent>::removeComponent(EntityID entityID) {
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

template <typename TComponent>
TComponent ComponentArray<TComponent>::getComponent(EntityID entityID, float simulationProgress) const {
    static Interpolator<TComponent> interpolator{};
    assert(mEntityToComponentIndex.find(entityID) != mEntityToComponentIndex.end());
    std::size_t componentID { mEntityToComponentIndex.at(entityID) };
    return interpolator(mComponentsPrevious[componentID], mComponentsNext[componentID], simulationProgress);
}

template <typename TComponent>
void ComponentArray<TComponent>::updateComponent(EntityID entityID, const TComponent& newComponent) {
    assert(mEntityToComponentIndex.find(entityID) != mEntityToComponentIndex.end());
    std::size_t componentID { mEntityToComponentIndex.at(entityID) };
    mComponentsNext[componentID] = newComponent;
}

template <typename TComponent>
void ComponentArray<TComponent>::handleEntityDestroyed(EntityID entityID) {
    if(mEntityToComponentIndex.find(entityID) != mEntityToComponentIndex.end()) {
        removeComponent(entityID);
    }
}

template<typename TComponent>
void ComponentArray<TComponent>::handlePreSimulationStep() {
    std::copy<typename std::vector<TComponent>::iterator, typename std::vector<TComponent>::iterator>(
        mComponentsNext.begin(), mComponentsNext.end(), 
        mComponentsPrevious.begin()
    );
}

template <typename TComponent>
void ComponentArray<TComponent>::copyComponent(EntityID to, EntityID from) {
    copyComponent(to, from, *this);
}

template <typename TComponent>
void ComponentArray<TComponent>::copyComponent(EntityID to, EntityID from, BaseComponentArray& other) {
    assert(to < kMaxEntities && "Cannot copy to an entity with an invalid entity ID");
    ComponentArray<TComponent>& downcastOther { static_cast<ComponentArray<TComponent>&>(other) };
    if(downcastOther.mEntityToComponentIndex.find(from) == downcastOther.mEntityToComponentIndex.end()) return;

    const TComponent& componentValueNext { downcastOther.mComponentsNext[downcastOther.mEntityToComponentIndex[from]] };
    const TComponent& componentValuePrevious { downcastOther.mComponentsPrevious[downcastOther.mEntityToComponentIndex[from]] };

    if(mEntityToComponentIndex.find(to) == mEntityToComponentIndex.end()) {
        addComponent(to, componentValueNext);
    } else {
        mComponentsNext[mEntityToComponentIndex[to]] = componentValueNext;
    }
    mComponentsPrevious[mEntityToComponentIndex[to]] = componentValuePrevious;
}

template<typename TComponent> 
void ComponentManager::registerComponentArray() {
    const std::size_t componentHash { typeid(TComponent).hash_code() };
    // nop when a component array for this type already exists
    if(mHashToComponentType.find(componentHash) != mHashToComponentType.end()) {
        return;
    }

    std::string componentTypeName { getComponentTypeName<TComponent>{}() };
    assert(mHashToComponentType.size() + 1 < kMaxComponents && "Component type limit reached");
    assert(mNameToComponentHash.find(componentTypeName) == mNameToComponentHash.end() && "Another component with this name\
        has already been registered");

    mNameToComponentHash.insert_or_assign(componentTypeName, componentHash);
    mHashToComponentArray.insert_or_assign(
        componentHash, std::static_pointer_cast<BaseComponentArray>(std::make_shared<ComponentArray<TComponent>>(mWorld))
    );
    mHashToComponentType[componentHash] = mHashToComponentType.size();
}

template<typename TComponent>
ComponentType ComponentManager::getComponentType() const {
    const std::size_t componentHash { typeid(TComponent).hash_code() };
    assert(mHashToComponentType.find(componentHash) != mHashToComponentType.end() && "Component type has not been registered");
    return mHashToComponentType.at(componentHash);
}

template<typename TComponent>
void ComponentManager::addComponent(EntityID entityID, const TComponent& component) {
    getComponentArray<TComponent>()->addComponent(entityID, component);
    mEntityToSignature[entityID].set(getComponentType<TComponent>(), true);
}

template <typename TComponent>
bool ComponentManager::hasComponent(EntityID entityID) const {
    return getComponentArray<TComponent>()->hasComponent(entityID);
}

template<typename TComponent>
void ComponentManager::removeComponent(EntityID entityID) {
    getComponentArray<TComponent>()->removeComponent(entityID);
    mEntityToSignature[entityID].set(getComponentType<TComponent>(), false);
}

template<typename TComponent>
TComponent ComponentManager::getComponent(EntityID entityID, float simulationProgress) const {
    return getComponentArray<TComponent>()->getComponent(entityID, simulationProgress);
}

template<typename TComponent>
void ComponentManager::updateComponent(EntityID entityID, const TComponent& newValue) {
    getComponentArray<TComponent>()->updateComponent(entityID, newValue);
}

template <typename TComponent>
void ComponentManager::copyComponent(EntityID to, EntityID from) {
    assert(mEntityToSignature[from].test(getComponentType<TComponent>()) && "The entity being copied from does not have this component");
    getComponentArray<TComponent>()->copyComponent(to, from);
    mEntityToSignature[to].set(getComponentType<TComponent>(), true);
}

template<typename TSystem>
SystemType SystemManager::getSystemType() const {
    const std::string systemTypeName{ TSystem::getSystemTypeName() };
    assert(mNameToSystemType.find(systemTypeName) != mNameToSystemType.end() && "Component type has not been registered");
    return mNameToSystemType.at(systemTypeName);
}

template <typename TSystem>
bool ECSWorld::isEnabled(EntityID entityID) {
    return mSystemManager->isEnabled<TSystem>(entityID);
}
template <typename TSystem>
bool ECSWorld::isRegistered(EntityID entityID) {
    return mSystemManager->isRegistered<TSystem>(entityID);
}

template <typename TComponent>
bool ECSWorld::hasComponent(EntityID entityID) const {
    return mComponentManager->hasComponent<TComponent>(entityID);
}


template <typename TSystemDerived, typename ...TRequiredComponents>
void System<TSystemDerived, TRequiredComponents...>::registerSelf() {
    ECSWorld::registerComponentTypes<TRequiredComponents...>();
    ECSWorld::registerSystem<TSystemDerived, TRequiredComponents...>();
}


template<typename TSystem>
void SystemManager::registerSystem(const Signature& signature) {
    std::string systemTypeName { TSystem::getSystemTypeName() };
    assert(mNameToSignature.find(systemTypeName) == mNameToSignature.end() && "System has already been registered");
    assert(mNameToSystemType.size() + 1 < kMaxSystems && "System type limit reached");

    mNameToSignature[systemTypeName] = signature;
    mNameToSystem.insert_or_assign(systemTypeName, std::make_shared<TSystem>(mWorld));
    mNameToSystemType[systemTypeName] = mNameToSystemType.size();
}

template<typename TSystem>
std::shared_ptr<TSystem> SystemManager::getSystem() {
    std::string systemTypeName { TSystem::getSystemTypeName() };
    assert(mNameToSignature.find(systemTypeName) != mNameToSignature.end() && "System has not yet been registered");
    return std::dynamic_pointer_cast<TSystem>(mNameToSystem[systemTypeName]);
}

template<typename TSystem>
void SystemManager::enableEntity(EntityID entityID) {
    std::string systemTypeName { TSystem::getSystemTypeName() };
    mNameToSystem[systemTypeName]->enableEntity(entityID);
}

template<typename TSystem>
void SystemManager::disableEntity(EntityID entityID) {
    std::string systemTypeName { TSystem::getSystemTypeName() };
    mNameToSystem[systemTypeName]->disableEntity(entityID);
}

template <typename TComponent>
void Entity::addComponent(const TComponent& component) {
    mWorld.lock()->addComponent<TComponent>(mID, component);
}

template <typename TComponent>
bool Entity::hasComponent() const {
    return mWorld.lock()->hasComponent<TComponent>(mID);
}

template<typename TComponent>
TComponent Entity::getComponent(float simulationProgress) const {
    return mWorld.lock()->getComponent<TComponent>(mID, simulationProgress);
}

template<typename TComponent>
void Entity::updateComponent(const TComponent& newValue) {
    mWorld.lock()->updateComponent<TComponent>(mID, newValue);
}

template <typename TComponent>
void Entity::removeComponent() {
    mWorld.lock()->removeComponent<TComponent>(mID);
}

template <typename TSystem>
void Entity::enableSystem() {
    mWorld.lock()->enableEntity<TSystem>(mID);
}

template <typename TSystem>
bool Entity::isEnabled() const {
    return mWorld.lock()->isEnabled<TSystem>(mID);
}

template <typename TSystem>
bool Entity::isRegistered() const {
    return mWorld.lock()->isRegistered<TSystem>(mID);
}

template <typename TSystem>
void Entity::disableSystem() {
    mWorld.lock()->disableEntity<TSystem>(mID);
}

template <typename TSystem>
bool SystemManager::isEnabled(EntityID entityID) {
    const std::string systemTypeName { TSystem::getSystemTypeName() };
    return mNameToSystem[systemTypeName]->isEnabled(entityID);
}

template <typename TSystem>
bool SystemManager::isRegistered(EntityID entityID) {
    const std::string systemTypeName { TSystem::getSystemTypeName() };
    return mNameToSystem[systemTypeName]->isRegistered(entityID);
}

template<typename ...TComponents>
Entity ECSWorld::privateCreateEntity(TComponents...components) {
    assert((mNextEntity < kMaxEntities || !mDeletedIDs.empty()) && "Max number of entities reached");

    EntityID nextID;
    if(!mDeletedIDs.empty()){
        nextID = mDeletedIDs.back();
        mDeletedIDs.pop_back();
    } else {
        nextID = mNextEntity++;
    }

    Entity entity { nextID, shared_from_this()};

    (addComponent<TComponents>(nextID, components), ...);
    return entity;
}

template<typename TSystem>
SystemType ECSWorld::getSystemType() {
    return mSystemManager->getSystemType<TSystem>();
}

template<typename TComponent>
void ECSWorld::addComponent(EntityID entityID, const TComponent& component) {
    assert(entityID < kMaxEntities && "Cannot add a component to an entity that does not exist");
    mComponentManager->addComponent<TComponent>(entityID, component);
    Signature signature { mComponentManager->getSignature(entityID) };
    mSystemManager->handleEntitySignatureChanged(entityID, signature);
}

template<typename TComponent>
void ECSWorld::removeComponent(EntityID entityID) {
    mComponentManager->removeComponent<TComponent>(entityID);
    Signature signature { mComponentManager->getSignature(entityID) };
    mSystemManager->handleEntitySignatureChanged(entityID, signature);
}

template<typename TSystem>
void ECSWorld::enableEntity(EntityID entityID) {
    mSystemManager->enableEntity<TSystem>(entityID);
}

template<typename TSystem>
void ECSWorld::disableEntity(EntityID entityID) {
    mSystemManager->disableEntity<TSystem>(entityID);
}

template<typename TComponent>
TComponent ECSWorld::getComponent(EntityID entityID, float progress) const {
    return mComponentManager->getComponent<TComponent>(entityID, progress);
}

template<typename TComponent, typename TSystem>
TComponent ECSWorld::getComponent(EntityID entityID, float progress) const {
    assert(
        (
            mSystemManager->mNameToSignature.at(TSystem::getSystemTypeName())
                .test(mComponentManager->getComponentType<TComponent>())
        )
        && "This system cannot access this kind of component"
    );
    return getComponent<TComponent>(entityID, progress);
}


template<typename TComponent>
void ECSWorld::updateComponent(EntityID entityID, const TComponent& newValue) {
    mComponentManager->updateComponent<TComponent>(entityID, newValue);
    mSystemManager->handleEntityUpdated(entityID, mComponentManager->getSignature(entityID));
}

template<typename TComponent, typename TSystem>
void ECSWorld::updateComponent(EntityID entityID, const TComponent& newValue) {
    assert(
        (
            mSystemManager->mNameToSignature.at(TSystem::getSystemTypeName())
                .test(mComponentManager->getComponentType<TComponent>())
        )
        && "This system cannot access this kind of component"
    );
    mComponentManager->updateComponent<TComponent>(entityID, newValue);
    mSystemManager->handleEntityUpdatedBySystem<TSystem>(entityID, mComponentManager->getSignature(entityID));
}

template<typename ...TComponents>
void ECSWorld::registerComponentTypes() {

    ((getInstance().lock()->mComponentManager->registerComponentArray<TComponents>()),...);
}

template<typename TSystem, typename ...TComponents>
void ECSWorld::registerSystem() {
    Signature signature {
        // expands into `1<<componentType1 | 1<<componentType2 | 1<<componentType3 | ...` during static
        // initialization to build this system's signature
        (
            (0x1u << getInstance().lock()->mComponentManager->getComponentType<TComponents>()) 
            | ... | 0x0u
        )
    };
    getInstance().lock()->mSystemManager->registerSystem<TSystem>(signature);
}

template<typename ...TComponents>
Entity ECSWorld::createEntity(TComponents...components) {
    return privateCreateEntity<TComponents...>(components...);
}

template <typename ...TComponents>
Entity ECSWorld::createEntityPrototype(TComponents...components) {
    return ECSWorld::getInstance().lock()->privateCreateEntity<TComponents...>(components...);
}

template<typename TSystem>
std::shared_ptr<TSystem> ECSWorld::getSystem() {
    return mSystemManager->getSystem<TSystem>();
}

template<typename TSystem>
std::shared_ptr<TSystem> ECSWorld::getSystemPrototype() {
    return getInstance().lock()->mSystemManager->getSystem<TSystem>();
}

template <typename TSingletonSystem>
std::shared_ptr<TSingletonSystem> ECSWorld::getSingletonSystem() {
    std::shared_ptr<TSingletonSystem> system { getInstance().lock()->getSystem<TSingletonSystem>() };
    assert(system->isSingleton() && "System specified is not an ECSWorld-aware singleton system");
    return system;
}

template<typename TComponent, typename TSystem>
TComponent BaseSystem::getComponent(EntityID entityID, float progress) const {
    assert(!isSingleton() && "Singletons cannot retrieve entity components through entity ID alone");
    return mWorld.lock()->getComponent<TComponent, TSystem>(entityID, progress);
}
template<typename TSystem, typename ...TRequiredComponents>
std::shared_ptr<BaseSystem> System<TSystem, TRequiredComponents...>::instantiate(std::weak_ptr<ECSWorld> world) {
    if(isSingleton()) return shared_from_this();
    return std::make_shared<TSystem>(world);
}

template <typename TComponent>
std::shared_ptr<BaseComponentArray> ComponentArray<TComponent>::instantiate(std::weak_ptr<ECSWorld> world) const {
    return std::make_shared<ComponentArray<TComponent>>(world);
}

template<typename TComponent, typename TSystem>
void BaseSystem::updateComponent(EntityID entityID, const TComponent& component) {
    assert(!isSingleton() && "Singletons cannot retrieve entity components through entity ID alone");
    mWorld.lock()->updateComponent<TComponent, TSystem>(entityID, component);
}

template<typename TSystem>
void SystemManager::handleEntityUpdatedBySystem(EntityID entityID, Signature signature) {
    std::string originatingSystemTypeName { TSystem::getSystemTypeName() };
    for(auto& pair: mNameToSignature) {
        // suppress update callback from the system that caused this update
        if(pair.first == originatingSystemTypeName) continue;

        Signature& systemSignature { pair.second };
        if((systemSignature & signature) == systemSignature){
            mNameToSystem[pair.first]->onEntityUpdated(entityID);
        }
    }
}

#endif
