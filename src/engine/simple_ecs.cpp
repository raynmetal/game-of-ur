#include <vector>
#include <cassert>
#include <cstdint>

#include "simple_ecs.hpp"

SystemManager gSystemManager {};
ComponentManager gComponentManager {};

std::vector<EntityID> Entity::sDeletedIDs{};
EntityID Entity::sNextEntity{ 0 };

Entity::Entity() {
    createEntity();
}

Entity::~Entity() {
    if(mID == kMaxEntities) return;

    sDeletedIDs.push_back(mID);
    gSystemManager.handleEntityDestroyed(mID);
    gComponentManager.handleEntityDestroyed(mID);
}

Entity::Entity(const Entity& other) {
    createEntity();
    if(other.mID == kMaxEntities) return;

    gComponentManager.copyComponents(mID, other.mID);
    const Signature& signature { gComponentManager.getSignature(mID) };
    gSystemManager.handleEntitySignatureChanged(mID, signature);
}

Entity::Entity(Entity&& other) {
    mID = other.mID;
    other.mID = kMaxEntities;
}


Entity& Entity::operator=(const Entity& other) {
    if(&other == this) return *this;

    sDeletedIDs.push_back(mID);
    gSystemManager.handleEntityDestroyed(mID);
    gComponentManager.handleEntityDestroyed(mID);

    createEntity();
    if(other.mID == kMaxEntities) return *this;
    gComponentManager.copyComponents(mID, other.mID);
    const Signature& signature { gComponentManager.getSignature(mID) };
    gSystemManager.handleEntitySignatureChanged(mID, signature);

    return *this;
}

Entity& Entity::operator=(Entity&& other) {
    if(&other == this) return *this;

    sDeletedIDs.push_back(mID);
    gSystemManager.handleEntityDestroyed(mID);
    gComponentManager.handleEntityDestroyed(mID);

    mID = other.mID;
    other.mID = kMaxEntities;

    return *this;
}

void Entity::createEntity() {
   assert((sNextEntity < kMaxEntities || !sDeletedIDs.empty()) && "Max number of entities reached");

    std::uint32_t nextID;
    if(!sDeletedIDs.empty()){
        nextID = sDeletedIDs.back();
        sDeletedIDs.pop_back();
    } else {
        nextID = sNextEntity++;
    }

    mID = nextID;
}

void System::enableEntity(EntityID entityID) {
    assert(
        (
            mEnabledEntities.find(entityID) != mEnabledEntities.end()
            || mDisabledEntities.find(entityID) != mDisabledEntities.end()
        ) && "This system does not apply to this entity"
    );
    mDisabledEntities.erase(entityID);
    mEnabledEntities.insert(entityID);
}

void System::disableEntity(EntityID entityID) {
    assert(
        (
            mEnabledEntities.find(entityID) != mEnabledEntities.end()
            || mDisabledEntities.find(entityID) != mDisabledEntities.end()
        ) && "This system does not apply to this entity"
    );
    mEnabledEntities.erase(entityID);
    mDisabledEntities.insert(entityID);
}

void System::addEntity(EntityID entityID, bool enabled) {
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

void System::removeEntity(EntityID entityID) {
    mEnabledEntities.erase(entityID);
    mDisabledEntities.erase(entityID);
}

const std::set<EntityID>& System::getEnabledEntities() {
    return mEnabledEntities;
}

bool System::isEnabled(EntityID entityID) {
    assert(
        (
            mEnabledEntities.find(entityID) != mEnabledEntities.end()
            || mDisabledEntities.find(entityID) != mDisabledEntities.end()
        ) && "This system does not apply to this entity"
    );

    return mEnabledEntities.find(entityID) != mEnabledEntities.end();
}

void SystemManager::handleEntitySignatureChanged(EntityID entityID, Signature signature) {
    for(auto& pair: mHashToSignature) {
        const Signature& systemSignature { pair.second };
        if((signature & systemSignature) == systemSignature) {
            mHashToSystem[pair.first]->addEntity(entityID);
        } else {
            mHashToSystem[pair.first]->removeEntity(entityID);
        }
    }
}

void SystemManager::handleEntityDestroyed(EntityID entityID) {
    for(auto& pair: mHashToSystem) {
        pair.second->removeEntity(entityID);
    }
}

Signature ComponentManager::getSignature(EntityID entityID) {
    return mEntityToSignature[entityID];
}

void ComponentManager::copyComponents(EntityID to, EntityID from) {
    for(auto& pair: mHashToComponentArray) {
        pair.second->copyComponent(to, from);
    }
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
    mHashToSystem.clear();
    mHashToSignature.clear();
}

void ComponentManager::unregisterAll() {
    mHashToComponentArray.clear();
    mHashToComponentType.clear();
    mEntityToSignature.clear();
}
