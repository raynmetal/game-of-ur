#include <vector>
#include <cassert>
#include <cstdint>

#include "ecs_world.hpp"

WorldID ECSWorld::s_nextWorld { 0 };

Entity::~Entity() {
    mWorld.get().destroyEntity(mID);
}

Entity::Entity(const Entity& other): Entity { other.mWorld.get().privateCreateEntity() } {
    mWorld.get().copyComponents(mID, other.mID, other.mWorld);
}

Entity::Entity(Entity&& other) noexcept: mID{ other.mID }, mWorld{ other.mWorld } {
    other.mID = kMaxEntities;
}


void Entity::disableSystems() {
    mWorld.get().disableEntity(mID);
}

void Entity::enableSystems(Signature systemMask) {
    mWorld.get().enableEntity(mID, systemMask);
}

Entity& Entity::operator=(const Entity& other) {
    if(&other == this) return *this;
    assert(mID < kMaxEntities && "This entity does not have a valid entity ID and cannot be copied to");

    // Clear existing components
    mWorld.get().removeComponentsAll(mID);

    // Copy components from other, if it is a valid entity
    if(other.mID != kMaxEntities) {
        mWorld.get().copyComponents(mID, other.mID, other.mWorld);
    }
    return *this;
}

Entity& Entity::operator=(Entity&& other) noexcept {
    if(&other == this) return *this;

    mWorld.get().destroyEntity(mID);

    mWorld = other.mWorld;
    mID = other.mID;
    other.mID = kMaxEntities;

    return *this;
}

void Entity::joinWorld(ECSWorld& world) {
    world.relocateEntity(*this);
}

void ECSWorld::relocateEntity(Entity& entity) {
    if(this == &entity.mWorld.get()) return; // nothing to be done, it's the same world

    assert((mNextEntity < kMaxEntities || !mDeletedIDs.empty()) && "Max number of entities reached");
    EntityID nextID;
    if(!mDeletedIDs.empty()){
        nextID = mDeletedIDs.back();
        mDeletedIDs.pop_back();
    } else {
        nextID = mNextEntity++;
    }

    copyComponents(nextID, entity.mID, entity.mWorld.get());
    entity.mWorld.get().destroyEntity(entity.mID);

    entity.mID = nextID;
    entity.mWorld = *this;
}

void BaseSystem::enableEntity(EntityID entityID) {
    if(isSingleton()) return;
    assert(
        (
            mEnabledEntities.find(entityID) != mEnabledEntities.end()
            || mDisabledEntities.find(entityID) != mDisabledEntities.end()
        ) && "This system does not apply to this entity"
    );
    mDisabledEntities.erase(entityID);
    mEnabledEntities.insert(entityID);
}

void Entity::copy(const Entity& other) {
    assert(mID < kMaxEntities && "This entity does not have a valid entity ID and cannot be copied to");
    if(other.mID == kMaxEntities) return;

    mWorld.get().copyComponents(mID, other.mID, other.mWorld);
}

void BaseSystem::disableEntity(EntityID entityID) {
    if(isSingleton()) return;
    assert(
        (
            mEnabledEntities.find(entityID) != mEnabledEntities.end()
            || mDisabledEntities.find(entityID) != mDisabledEntities.end()
        ) && "This system does not apply to this entity"
    );
    mEnabledEntities.erase(entityID);
    mDisabledEntities.insert(entityID);
}

void BaseSystem::addEntity(EntityID entityID, bool enabled) {
    if(
        isSingleton() 
        || (
            mEnabledEntities.find(entityID) != mEnabledEntities.end()
            || mDisabledEntities.find(entityID) != mDisabledEntities.end()
        )
    ) return;

    if(enabled){
        mEnabledEntities.insert(entityID);
    } else {
        mDisabledEntities.insert(entityID);
    }
}

void BaseSystem::removeEntity(EntityID entityID) {
    mEnabledEntities.erase(entityID);
    mDisabledEntities.erase(entityID);
}

const std::set<EntityID>& BaseSystem::getEnabledEntities() {
    return mEnabledEntities;
}

bool BaseSystem::isEnabled(EntityID entityID) const {
    return !isSingleton() && mEnabledEntities.find(entityID) != mEnabledEntities.end();
}

bool BaseSystem::isRegistered(EntityID entityID) const {
    return (
        !isSingleton() && (
            mEnabledEntities.find(entityID) != mEnabledEntities.end()
            || mDisabledEntities.find(entityID) != mDisabledEntities.end()
        )
    );
}

void SystemManager::initialize() {
    for(auto& pair: mNameToSystem) {
        pair.second->onCreated();
    }
}

SystemManager SystemManager::instantiate(ECSWorld& world) const {
    SystemManager newSystemManager { world };
    newSystemManager.mNameToSignature = mNameToSignature;
    newSystemManager.mNameToSystemType = mNameToSystemType;
    for(auto pair: mNameToSystem) {
        newSystemManager.mNameToSystem.insert({
            pair.first,
            pair.second->instantiate(world)
        });
    }
    return newSystemManager;
}

void SystemManager::enableEntity(EntityID entityID, Signature entitySignature, Signature systemMask) {
    for(const auto& signaturePair: mNameToSignature) {
        auto& system { mNameToSystem[signaturePair.first] };
        if(
            // entity and system signatures match
            (signaturePair.second&entitySignature) == signaturePair.second
            // mask allows this system to be enabled for this entity
            && systemMask.test(mNameToSystemType[signaturePair.first])
            // entity isn't already enabled for this system
            && (!system->isSingleton() && !system->isEnabled(entityID))
        ) {
            system->enableEntity(entityID);
            system->onEntityEnabled(entityID);
        }
    }
}

void SystemManager::disableEntity(EntityID entityID, Signature entitySignature) {
    // disable all systems which match this entity's signature
    for(const auto& signaturePair: mNameToSignature) {
        if(
            // signatures match
            (signaturePair.second&entitySignature) == signaturePair.second
            // and entity is enabled
            && (
                !mNameToSystem[signaturePair.first]->isSingleton() 
                && mNameToSystem[signaturePair.first]->isEnabled(entityID)
            )
        ) {
            mNameToSystem[signaturePair.first]->disableEntity(entityID);
            mNameToSystem[signaturePair.first]->onEntityDisabled(entityID);
        }
    }
}

void SystemManager::handleEntitySignatureChanged(EntityID entityID, Signature signature) {
    for(auto& pair: mNameToSignature) {
        const std::string systemTypeName { pair.first };
        const Signature& systemSignature { pair.second };
        BaseSystem& system { *(mNameToSystem[systemTypeName].get()) };
        if(system.isSingleton()) continue;

        if(
            (signature&systemSignature) == systemSignature 
            && !system.isRegistered(entityID)
        ) {
            system.addEntity(entityID, false);

        } else if(
            (signature&systemSignature) != systemSignature 
            && system.isRegistered(entityID) 
        ) {
            system.disableEntity(entityID);
            system.onEntityDisabled(entityID);
            system.removeEntity(entityID);
        }
    }
}

void SystemManager::handleEntityDestroyed(EntityID entityID) {
    for(auto& pair: mNameToSystem) {
        if(!pair.second->isSingleton() && pair.second->isRegistered(entityID)) {
            if(pair.second->isEnabled(entityID)){
                pair.second->disableEntity(entityID);
                pair.second->onEntityDisabled(entityID);
            }
            pair.second->removeEntity(entityID);
        }
    }
}

void SystemManager::handleEntityUpdated(EntityID entityID, Signature signature) {
    for(auto& pair: mNameToSignature) {
        Signature& systemSignature { pair.second };
        if((systemSignature&signature) != systemSignature) continue;

        BaseSystem& system { *(mNameToSystem[pair.first]).get() };
        if(system.isSingleton() || !system.isEnabled(entityID)) continue;

        system.onEntityUpdated(entityID);
    }
}

ComponentManager ComponentManager::instantiate(ECSWorld& world) const {
    ComponentManager newComponentManager {world};
    newComponentManager.mNameToComponentHash = mNameToComponentHash;
    newComponentManager.mHashToComponentType = mHashToComponentType;
    for(auto pair: mHashToComponentArray) {
        newComponentManager.mHashToComponentArray.insert({
            pair.first,
            pair.second->instantiate(world)
        });
    }
    return newComponentManager;
}

void ComponentManager::addComponent(EntityID entityID, const nlohmann::json& jsonComponent) {
    std::string componentTypeName { jsonComponent.at("type").get<std::string>() };
    std::size_t componentHash { mNameToComponentHash.at(componentTypeName) };
    ComponentType componentType { mHashToComponentType.at(componentHash) };
    mHashToComponentArray.at(componentHash)->addComponent(entityID, jsonComponent);
    mEntityToSignature.at(entityID).set(componentType, true);
}

Signature ComponentManager::getSignature(EntityID entityID) {
    return mEntityToSignature[entityID];
}

void ComponentManager::copyComponents(EntityID to, EntityID from) {
    copyComponents(to, from, *this);
}

void ComponentManager::copyComponents(EntityID to, EntityID from, ComponentManager& other) {
    for(auto& pair: mHashToComponentArray) {
        pair.second->copyComponent(to, from, *other.mHashToComponentArray[pair.first]);
    }
    mEntityToSignature[to] |= other.mEntityToSignature[from];
}


void ComponentManager::handleEntityDestroyed(EntityID entityID) {
    const Signature entitySignature { mEntityToSignature[entityID] };

    for(const auto& pair: mHashToComponentType) {
        const std::size_t componentHash { pair.first };
        const ComponentType componentType { pair.second };

        if(entitySignature.test(componentType)) {
            mHashToComponentArray[componentHash]->handleEntityDestroyed(entityID);
        }
    }

    mEntityToSignature.erase(entityID);
}

void SystemManager::unregisterAll() {
    for(auto& pair: mNameToSystem) {
        pair.second->onDestroyed();
    }
    mNameToSystem.clear();
    mNameToSignature.clear();
}

void ComponentManager::unregisterAll() {
    mNameToComponentHash.clear();
    mHashToComponentArray.clear();
    mHashToComponentType.clear();
    mEntityToSignature.clear();
}

void ECSWorld::addComponent(EntityID entityID, const nlohmann::json& jsonComponent) {
    assert(entityID < kMaxEntities && "Cannot add a component to an entity that does not exist");
    mComponentManager.addComponent(entityID, jsonComponent);
    Signature signature { mComponentManager.getSignature(entityID) };
    mSystemManager.handleEntitySignatureChanged(entityID, signature);
}

void Entity::addComponent(const nlohmann::json& jsonComponent) {
    mWorld.get().addComponent(mID, jsonComponent);
}

const ECSWorld& ECSWorld::getPrototype() {
    return getInstance();
}
ECSWorld& ECSWorld::getInstance() {
    static ECSWorld instance {};
    return instance;
}

ECSWorld ECSWorld::instantiate() const {
    ECSWorld newWorld{
        mSystemManager.instantiate(newWorld),
        mComponentManager.instantiate(newWorld),
        ++s_nextWorld
    };
    return newWorld;
}

void ECSWorld::initialize() {
    mSystemManager.initialize();
}

void ECSWorld::disableEntity(EntityID entityID) {
    Signature entitySignature {mComponentManager.getSignature(entityID)};
    mSystemManager.disableEntity(entityID, entitySignature);
}
void ECSWorld::enableEntity(EntityID entityID, Signature systemMask) {
    Signature entitySignature {mComponentManager.getSignature(entityID)};
    mSystemManager.enableEntity(entityID, entitySignature, systemMask);
}

void ECSWorld::removeComponentsAll(EntityID entityID) {
    mSystemManager.handleEntityDestroyed(entityID);
    mComponentManager.handleEntityDestroyed(entityID);
}

void ECSWorld::destroyEntity(EntityID entityID) {
    if(entityID == kMaxEntities) return;

    removeComponentsAll(entityID);
    mDeletedIDs.push_back(entityID);
}

void ComponentManager::handleFrameBegin() {
    for(auto& pair: mHashToComponentArray) {
        // copy "Next" buffer into "Previous" buffer
        pair.second->handleFrameBegin();
    }
}

void ECSWorld::copyComponents(EntityID to, EntityID from) {
    copyComponents(to, from, *this);
}

void ECSWorld::copyComponents(EntityID to, EntityID from, ECSWorld& other) {
    if(from == kMaxEntities) return;
    mComponentManager.copyComponents(to, from, other.mComponentManager);
    const Signature& signature { mComponentManager.getSignature(to) };
    mSystemManager.handleEntitySignatureChanged(to, signature);
}

void ECSWorld::beginFrame() {
    mComponentManager.handleFrameBegin();
}

void ECSWorld::cleanup() {
    mComponentManager.unregisterAll();
    mSystemManager.unregisterAll();
}
