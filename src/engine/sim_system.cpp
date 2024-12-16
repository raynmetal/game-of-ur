#include "sim_system.hpp"

SimObject::SimObject(SceneNode&& sceneNode):
SceneNode{std::move(sceneNode)}
{
    addComponent<SimCore>({this});
}

SimObject::SimObject(const SceneNode& sceneNode):
SceneNode {sceneNode}
{
    addComponent<SimCore>({this});
}

SimObject::SimObject(SimObject&& simObject):
SceneNode { std::move(simObject) },
mSimObjectAspects { std::move(simObject.mSimObjectAspects) }
{
    updateComponent<SimCore>({this});
    for(auto& aspectPair: mSimObjectAspects) {
        aspectPair.second->mSimObject = this;
    }
}

SimObject::SimObject(const SimObject& simObject):
SceneNode { simObject }
{
    updateComponent<SimCore>({this});
    for(auto& aspectPair: simObject.mSimObjectAspects) {
        mSimObjectAspects[aspectPair.first] = aspectPair.second->makeCopy();
        mSimObjectAspects[aspectPair.first]->mSimObject = this;
    }
}

SimObject& SimObject::operator=(SimObject&& simObject) {
    if(this == &simObject) return *this;

    SceneNode::operator=(std::move(simObject));
    mSimObjectAspects = std::move(simObject.mSimObjectAspects);

    // update pointers to self for visibility to the sim system
    // as well as by attached aspects
    updateComponent<SimCore>({this});
    for(auto& aspectPair: mSimObjectAspects) {
        mSimObjectAspects[aspectPair.first]->mSimObject = this;
    }

    return *this;
}

SimObject& SimObject::operator=(const SimObject& other) {
    if(this == &other) return *this;

    SceneNode::operator=(other);
    updateComponent<SimCore>({this});
    for(auto& aspectPair: other.mSimObjectAspects) {
        mSimObjectAspects[aspectPair.first] = aspectPair.second->makeCopy();
        mSimObjectAspects[aspectPair.first]->mSimObject = this;
    }

    return *this;
}

std::unique_ptr<BaseSimObjectAspect> SimSystem::constructAspect(const nlohmann::json& jsonAspectProperties) {
    return mAspectConstructors.at(jsonAspectProperties.at("type").get<std::string>())(jsonAspectProperties);
}

void SimSystem::ApploopEventHandler::onSimulationStep(uint32_t deltaSimTimeMillis) {
    for(EntityID entity: mSystem->getEnabledEntities()) {
        mSystem->getComponent<SimCore>(entity).mSimObject->update(deltaSimTimeMillis);
    }
}

void SimObject::update(uint32_t deltaSimTimeMillis) {
    for(auto& pair: mSimObjectAspects) {
        pair.second->update(deltaSimTimeMillis);
    }
}

EntityID BaseSimObjectAspect::getEntityID() const {
    return mSimObject->getEntityID();
}

std::shared_ptr<SimObject> SimObject::copy(const std::shared_ptr<SimObject> simObject) {
    return std::shared_ptr<SimObject>(new SimObject{ *simObject });
}

void SimObject::addAspect(const BaseSimObjectAspect& aspect) {
    mSimObjectAspects.try_emplace(aspect.getAspectTypeName(), aspect.makeCopy());
    mSimObjectAspects.at(aspect.getAspectTypeName())->mSimObject = this;
}

void SimObject::addAspect(const nlohmann::json& jsonAspectProperties) {
    mSimObjectAspects.try_emplace(
        jsonAspectProperties.at("type").get<std::string>(),
        SimpleECS::getSystem<SimSystem>()->constructAspect(jsonAspectProperties)
    );
    mSimObjectAspects.at(jsonAspectProperties.at("type").get<std::string>())->mSimObject = this;
}

void BaseSimObjectAspect::addAspect(const BaseSimObjectAspect& aspect) {
    mSimObject->addAspect(aspect);
}

void BaseSimObjectAspect::addAspect(const nlohmann::json& jsonAspectProperties) {
    mSimObject->addAspect(jsonAspectProperties);
}
