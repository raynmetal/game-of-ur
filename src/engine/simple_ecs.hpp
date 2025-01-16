#ifndef ZOSIMPLEECS_H
#define ZOSIMPLEECS_H

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
constexpr EntityID kMaxEntities { 300000 };

/*
  A simple integer, used as an index into the Signature bitset
to determine what components are available for an Entity, and what
components are required by a System
*/
using ECSType = std::uint8_t;
constexpr ECSType kMaxECSTypes { 255 };

using ComponentType = ECSType;
using SystemType = ECSType;
constexpr ComponentType kMaxComponents { kMaxECSTypes };
constexpr SystemType kMaxSystems { kMaxECSTypes };

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
    virtual void handleFrameBegin() = 0;
    virtual void copyComponent(EntityID to, EntityID from)=0;
    virtual void addComponent(EntityID to, const nlohmann::json& jsonComponent)=0;
    virtual bool hasComponent(EntityID entityID) const=0;
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
class ComponentArray : public IComponentArray {
public:
private:
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
    virtual void handleFrameBegin() override;
    virtual void copyComponent(EntityID to, EntityID from) override;

    std::vector<TComponent> mComponentsNext {};
    std::vector<TComponent> mComponentsPrevious {};
    std::unordered_map<EntityID, std::size_t> mEntityToComponentIndex {};
    std::unordered_map<std::size_t, EntityID> mComponentToEntity {};
friend class ComponentManager;
};

class ComponentManager {
public:
private:
    ComponentManager() = default;

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

    void handleEntityDestroyed(EntityID entityID);

    void handleFrameBegin();

    void unregisterAll();

    std::unordered_map<std::string, std::size_t> mNameToComponentHash {};
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

    void enableEntity(EntityID entityID, Signature entitySignature, Signature systemMask = Signature{}.set());

    template<typename TSystem>
    void disableEntity(EntityID entityID);

    void disableEntity(EntityID entityID, Signature entitySignature);

    template<typename TSystem>
    SystemType getSystemType() const;

    template<typename TSystem>
    bool isEnabled(EntityID entityID);

    void handleEntitySignatureChanged(EntityID entityID, Signature signature);

    void handleEntityDestroyed(EntityID entityID);

    void handleEntityUpdated(EntityID entityID, Signature signature);

    template<typename TSystem>
    void handleEntityUpdatedBySystem(EntityID entityID, Signature signature);

    std::unordered_map<std::size_t, Signature> mHashToSignature {};
    std::unordered_map<std::size_t, SystemType> mHashToSystemType {};
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

    template <typename TSystem>
    static SystemType getSystemType();

    template <typename TSystem>
    static bool isEnabled(EntityID entityID);

    template<typename ...TComponents>
    static Entity createEntity(TComponents...components);

    static void initialize();

    static void cleanup();

    static void beginFrame();

private:
    static SimpleECS& getInstance();

    SimpleECS() = default;

    void copyComponents(EntityID to, EntityID from);

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

    template<typename TSystem>
    void enableSystem();

    template<typename TSystem>
    void disableSystem();

    void disableSystems();
    void enableSystems(Signature systemMask);

private:
    Entity(EntityID entityID): mID{ entityID } {};
    EntityID mID;

friend class SimpleECS;
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
void ComponentArray<TComponent>::handleFrameBegin() {
    std::copy<typename std::vector<TComponent>::iterator, typename std::vector<TComponent>::iterator>(
        mComponentsNext.begin(), mComponentsNext.end(), 
        mComponentsPrevious.begin()
    );
}

template <typename TComponent>
void ComponentArray<TComponent>::copyComponent(EntityID to, EntityID from) {
    if(mEntityToComponentIndex.find(from) != mEntityToComponentIndex.end()) {
        if(mEntityToComponentIndex.find(to) == mEntityToComponentIndex.end()) {
            // create a local copy of the component as container may be reallocated
            TComponent componentValue { mComponentsNext[mEntityToComponentIndex[from]] };
            addComponent(to, componentValue); 
        } else {
            mComponentsNext[mEntityToComponentIndex[to]] = mComponentsNext[mEntityToComponentIndex[from]]; // overwriting existing values is fine
        }
        mComponentsPrevious[mEntityToComponentIndex[to]] = mComponentsPrevious[mEntityToComponentIndex[from]];
    }
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
        componentHash, std::static_pointer_cast<IComponentArray>(std::make_shared<ComponentArray<TComponent>>())
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
    const std::size_t systemHash { typeid(TSystem).hash_code() };
    assert(mHashToSystemType.find(systemHash) != mHashToSystemType.end() && "Component type has not been registered");
    return mHashToSystemType.at(systemHash);
}

template <typename TSystem>
bool SimpleECS::isEnabled(EntityID entityID) {
    return SimpleECS::getInstance().mSystemManager.isEnabled<TSystem>(entityID);
}

template <typename TComponent>
bool SimpleECS::hasComponent(EntityID entityID) const {
    return mComponentManager.hasComponent<TComponent>(entityID);
}


template <typename TSystemDerived, typename ...TRequiredComponents>
void System<TSystemDerived, TRequiredComponents...>::registerSelf() {
    SimpleECS::registerComponentTypes<TRequiredComponents...>();
    SimpleECS::registerSystem<TSystemDerived, TRequiredComponents...>();
}


template<typename TSystem>
void SystemManager::registerSystem(const Signature& signature) {
    std::size_t systemHash {typeid(TSystem).hash_code()};

    assert(mHashToSignature.find(systemHash) == mHashToSignature.end() && "System has already been registered");
    assert(mHashToSystemType.size() + 1 < kMaxSystems && "System type limit reached");

    mHashToSignature[systemHash] = signature;
    mHashToSystem.insert_or_assign(systemHash, std::make_shared<TSystem>());
    mHashToSystemType[systemHash] = mHashToSystemType.size();
}

template<typename TSystem>
std::shared_ptr<TSystem> SystemManager::getSystem() {
    std::size_t systemHash {typeid(TSystem).hash_code()};
    assert(mHashToSignature.find(systemHash) != mHashToSignature.end() && "System has not yet been registered");
    return std::dynamic_pointer_cast<TSystem>(mHashToSystem[systemHash]);
}

template<typename TSystem>
void SystemManager::enableEntity(EntityID entityID) {
    std::size_t systemHash {typeid(TSystem).hash_code()};
    mHashToSystem[systemHash]->enableEntity(entityID);
}

template<typename TSystem>
void SystemManager::disableEntity(EntityID entityID) {
    std::size_t systemHash{ typeid(TSystem).hash_code() };
    mHashToSystem[systemHash]->disableEntity(entityID);
}

template <typename TComponent>
void Entity::addComponent(const TComponent& component) {
    SimpleECS::getInstance().addComponent<TComponent>(mID, component);
}

template <typename TComponent>
bool Entity::hasComponent() const {
    return SimpleECS::getInstance().hasComponent<TComponent>(mID);
}

template<typename TComponent>
TComponent Entity::getComponent(float simulationProgress) const {
    return SimpleECS::getInstance().getComponent<TComponent>(mID, simulationProgress);
}

template<typename TComponent>
void Entity::updateComponent(const TComponent& newValue) {
    SimpleECS::getInstance().updateComponent<TComponent>(mID, newValue);
}

template <typename TComponent>
void Entity::removeComponent() {
    SimpleECS::getInstance().removeComponent<TComponent>(mID);
}

template <typename TSystem>
void Entity::enableSystem() {
    SimpleECS::getInstance().enableEntity<TSystem>(mID);
}

template <typename TSystem>
bool Entity::isEnabled() const {
    return SimpleECS::isEnabled<TSystem>(mID);
}

template <typename TSystem>
void Entity::disableSystem() {
    SimpleECS::getInstance().disableEntity<TSystem>(mID);
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

template<typename TSystem>
SystemType SimpleECS::getSystemType() {
    return SimpleECS::getInstance().mSystemManager.getSystemType<TSystem>();
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
        // expands into `1<<componentType1 | 1<<componentType2 | 1<<componentType3 | ...` during static
        // initialization to build this system's signature
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
