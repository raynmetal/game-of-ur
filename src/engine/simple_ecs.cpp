#include <vector>
#include <cassert>
#include <cstdint>

#include "simple_ecs.hpp"

Entity::~Entity() {
    SimpleECS::getInstance().destroyEntity(mID);
}

Entity::Entity(const Entity& other): Entity { SimpleECS::getInstance().privateCreateEntity() } {
    SimpleECS::getInstance().copyComponents(mID, other.mID);
}

void Entity::disableSystems() {
    SimpleECS::getInstance().disableEntity(mID);
}

void Entity::enableSystems(Signature systemMask) {
    SimpleECS::getInstance().enableEntity(mID, systemMask);
}

Entity::Entity(Entity&& other) noexcept {
    mID = other.mID;
    other.mID = kMaxEntities;
}

Entity& Entity::operator=(const Entity& other) {
    if(&other == this) return *this;
    assert(mID < kMaxEntities && "This entity does not have a valid entity ID and cannot be copied to");

    // Clear existing components
    SimpleECS::getInstance().removeComponentsAll(mID);

    // Copy components from other, if it is a valid entity
    if(other.mID != kMaxEntities) {
        SimpleECS::getInstance().copyComponents(mID, other.mID);
    }
    return *this;
}

Entity& Entity::operator=(Entity&& other) noexcept {
    if(&other == this) return *this;

    SimpleECS::getInstance().destroyEntity(mID);

    mID = other.mID;
    other.mID = kMaxEntities;

    return *this;
}

void BaseSystem::enableEntity(EntityID entityID) {
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

    SimpleECS::getInstance().copyComponents(mID, other.mID);
}

void BaseSystem::disableEntity(EntityID entityID) {
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
        mEnabledEntities.find(entityID) != mEnabledEntities.end()
        || mDisabledEntities.find(entityID) != mDisabledEntities.end()
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
    return mEnabledEntities.find(entityID) != mEnabledEntities.end();
}

bool BaseSystem::isRegistered(EntityID entityID) const {
    return (
        mEnabledEntities.find(entityID) != mEnabledEntities.end()
        || mDisabledEntities.find(entityID) != mDisabledEntities.end()
    );
}

void SystemManager::initialize() {
    for(auto& pair: mHashToSystem) {
        pair.second->onCreated();
    }
}

void SystemManager::enableEntity(EntityID entityID, Signature entitySignature, Signature systemMask) {
    for(const auto& signaturePair: mHashToSignature) {
        auto& system { mHashToSystem[signaturePair.first] };
        if(
            // entity and system signatures match
            (signaturePair.second&entitySignature) == signaturePair.second
            // mask allows this system to be enabled for this entity
            && systemMask.test(mHashToSystemType[signaturePair.first])
            // entity isn't already enabled for this system
            && !system->isEnabled(entityID)
        ) {
            system->enableEntity(entityID);
            system->onEntityEnabled(entityID);
        }
    }
}

void SystemManager::disableEntity(EntityID entityID, Signature entitySignature) {
    // disable all systems which match this entity's signature
    for(const auto& signaturePair: mHashToSignature) {
        if(
            // signatures match
            (signaturePair.second&entitySignature) == signaturePair.second
            // and entity is enabled
            && mHashToSystem[signaturePair.first]->isEnabled(entityID)
        ) {
            mHashToSystem[signaturePair.first]->disableEntity(entityID);
            mHashToSystem[signaturePair.first]->onEntityDisabled(entityID);
        }
    }
}

void SystemManager::handleEntitySignatureChanged(EntityID entityID, Signature signature) {
    for(auto& pair: mHashToSignature) {
        const std::size_t systemHash { pair.first };
        const Signature& systemSignature { pair.second };
        BaseSystem& system { *(mHashToSystem[systemHash].get()) };

        if(
            (signature&systemSignature) == systemSignature && !system.isRegistered(entityID)
        ) {
            system.addEntity(entityID, false);
        } else if(
            (signature&systemSignature) != systemSignature && system.isRegistered(entityID) 
        ) {
            system.disableEntity(entityID);
            system.onEntityDisabled(entityID);
            system.removeEntity(entityID);
        }
    }
}

void SystemManager::handleEntityDestroyed(EntityID entityID) {
    for(auto& pair: mHashToSystem) {
        if(pair.second->isRegistered(entityID)) {
            if(pair.second->isEnabled(entityID)){
                pair.second->disableEntity(entityID);
                pair.second->onEntityDisabled(entityID);
            }
            pair.second->removeEntity(entityID);
        }
    }
}

void SystemManager::handleEntityUpdated(EntityID entityID, Signature signature) {
    for(auto& pair: mHashToSignature) {
        Signature& systemSignature { pair.second };
        if((systemSignature & signature) == systemSignature){
            BaseSystem& system { *(mHashToSystem[pair.first]).get() };
            system.onEntityUpdated(entityID);
        }
    }
}

Signature ComponentManager::getSignature(EntityID entityID) {
    return mEntityToSignature[entityID];
}

void ComponentManager::copyComponents(EntityID to, EntityID from) {
    for(auto& pair: mHashToComponentArray) {
        pair.second->copyComponent(to, from);
    }
    mEntityToSignature[to] |= mEntityToSignature[from];
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
    for(auto& pair: mHashToSystem) {
        pair.second->onDestroyed();
    }
    mHashToSystem.clear();
    mHashToSignature.clear();
}

void ComponentManager::unregisterAll() {
    mHashToComponentArray.clear();
    mHashToComponentType.clear();
    mEntityToSignature.clear();
}

SimpleECS& SimpleECS::getInstance() {
    static SimpleECS instance {};
    return instance;
}

void SimpleECS::initialize() {
    getInstance().mSystemManager.initialize();
}

void SimpleECS::disableEntity(EntityID entityID) {
    Signature entitySignature {mComponentManager.getSignature(entityID)};
    mSystemManager.disableEntity(entityID, entitySignature);
}
void SimpleECS::enableEntity(EntityID entityID, Signature systemMask) {
    Signature entitySignature {mComponentManager.getSignature(entityID)};
    mSystemManager.enableEntity(entityID, entitySignature, systemMask);
}

void SimpleECS::removeComponentsAll(EntityID entityID) {
    mSystemManager.handleEntityDestroyed(entityID);
    mComponentManager.handleEntityDestroyed(entityID);
}

void SimpleECS::destroyEntity(EntityID entityID) {
    if(entityID == kMaxEntities) return;

    removeComponentsAll(entityID);
    mDeletedIDs.push_back(entityID);
}

void ComponentManager::handleFrameEnd() {
    for(auto& pair: mHashToComponentArray) {
        // copy "Next" buffer into "Previous" buffer
        pair.second->handleFrameEnd();
    }
}

void SimpleECS::copyComponents(EntityID to, EntityID from) {
    if(from == kMaxEntities) return;
    mComponentManager.copyComponents(to, from);
    const Signature& signature { mComponentManager.getSignature(to) };
    mSystemManager.handleEntitySignatureChanged(to, signature);
}

void SimpleECS::endFrame() {
    getInstance().mComponentManager.handleFrameEnd();
}

void SimpleECS::cleanup() {
    getInstance().mComponentManager.unregisterAll();
    getInstance().mSystemManager.unregisterAll();
}
