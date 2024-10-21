#include "sim_system.hpp"

void SimSystem::ApploopEventHandler::onSimulationStep(uint32_t deltaSimTimeMillis) {
    for(EntityID entity: mSystem->getEnabledEntities()) {
        reinterpret_cast<SimObject*>(mSystem->getComponent<SimCore>(entity).mSimObject)->update(deltaSimTimeMillis);
    }
}

SimObject::SimObject(SimObject&& other) {
    mEntity = std::move(other.mEntity);
    mSimComponents = std::move(other.mSimComponents);

    // update associated pointers
    mEntity->updateComponent<SimSystem::SimCore>(SimSystem::SimCore{this});
    for(auto& pair : *mSimComponents) {
        pair.second->mSimObject = this;
    }
}

SimObject& SimObject::operator=(SimObject&& other) {
    if(&other == this) return *this;

    mEntity = std::move(other.mEntity);
    mSimComponents = std::move(other.mSimComponents);

    // update associated pointers
    mEntity->updateComponent<SimSystem::SimCore>(SimSystem::SimCore{this});
    for(auto& pair : *mSimComponents) {
        pair.second->mSimObject = this;
    }

    return *this;
}

Entity& SimObject::getEntity() {
    return *mEntity;
}

void SimObject::update(uint32_t deltaSimTimeMillis) {
    for(auto& pair: *mSimComponents) {
        pair.second->update(deltaSimTimeMillis);
    }
}

EntityID SimObject::getEntityID() const {
    return mEntity->getID();
}
