#include "sim_system.hpp"

std::shared_ptr<SimObject> SimObject::create(const nlohmann::json& jsonSimObject) {
    std::shared_ptr<SceneNode> newSimObject {new SimObject{ jsonSimObject }};
    std::shared_ptr<SimObject> downcastSimObject { std::static_pointer_cast<SimObject, SceneNode>(newSimObject) };

    for(const nlohmann::json& aspectDescription: jsonSimObject.at("aspects")) {
        downcastSimObject->addAspect(aspectDescription);
    }

    return downcastSimObject;
}

std::shared_ptr<SceneNode> SimObject::clone() const {
    // since SceneNode enables shared from this, we must ensure that the
    // associated SceneNode control block is created
    std::shared_ptr<SceneNode> newSimObject { new SimObject{ *this } };
    std::shared_ptr<SimObject> downcastSimObject { std::static_pointer_cast<SimObject>(newSimObject) };

    downcastSimObject->copyAspects(*this);

    return newSimObject;
}

SimObject::SimObject(const nlohmann::json& jsonSimObject):
SceneNode{jsonSimObject},
Resource<SimObject>{0}
{
    addComponent<SimCore>({this}, true);
}

SimObject::SimObject(const SceneNode& sceneNode):
SceneNode { sceneNode },
Resource<SimObject>{0}
{
    addComponent<SimCore>({this}, true);
}

SimObject::SimObject(const SimObject& simObject):
SceneNode { simObject },
Resource<SimObject>{0}
{
    updateComponent<SimCore>({this});
}

// Save this for whenever I actually find a use for it
// SimObject& SimObject::operator=(const SimObject& other) {
//     if(this == &other) return *this;

//     SceneNode::operator=(other);
//     updateComponent<SimCore>({this});

//     // with assignment, we can assume that a shared pointer
//     // to this already exists, and so aspects which depend on
//     // it being present are safe
//     copyAspects(other);

//     return *this;
// }

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

void SimObject::copyAspects(const SimObject& other) {
    mSimObjectAspects.clear();
    for(auto& aspectPair: other.mSimObjectAspects) {
        mSimObjectAspects[aspectPair.first] = aspectPair.second->makeCopy();
        mSimObjectAspects[aspectPair.first]->mSimObject = this;
    }
}

EntityID BaseSimObjectAspect::getEntityID() const {
    return mSimObject->getEntityID();
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

std::shared_ptr<IResource> SimObjectFromDescription::createResource(const nlohmann::json& methodParameters) {
    // SceneNode shared pointer control block created through SimObject::create
    std::shared_ptr<SimObject> simObjectPtr { SimObject::create(methodParameters) };

    // Resource<SimObject> shared pointer shares reference count with SceneNode shared pointer
    // control block
    return std::static_pointer_cast<Resource<SimObject>, SimObject>(simObjectPtr);
}
