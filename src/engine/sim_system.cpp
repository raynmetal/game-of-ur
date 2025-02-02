#include "application.hpp"

#include "sim_system.hpp"

SimObject::~SimObject() {
    for(const auto& aspectPair: mSimObjectAspects) {
        aspectPair.second->detach();
    }
}

std::shared_ptr<SimObject> SimObject::create(const nlohmann::json& jsonSimObject) {
    std::shared_ptr<SimObject> newSimObject { BaseSceneNode<SimObject>::create(jsonSimObject) };
    for(const nlohmann::json& aspectDescription: jsonSimObject.at("aspects")) {
        newSimObject->addAspect(aspectDescription);
    }
    return newSimObject;
}

std::shared_ptr<SimObject> SimObject::copy(const std::shared_ptr<const SimObject> simObject) {
    return BaseSceneNode<SimObject>::copy(simObject);
}

std::shared_ptr<SceneNodeCore> SimObject::clone() const {
    // since SceneNode enables shared from this, we must ensure that the
    // associated SceneNode control block is created
    std::shared_ptr<SceneNodeCore> newSimObject { new SimObject{ *this }, &SceneNodeCore_del_ };
    std::shared_ptr<SimObject> downcastSimObject { std::static_pointer_cast<SimObject>(newSimObject) };

    downcastSimObject->copyAspects(*this);

    return newSimObject;
}

SimObject::SimObject(const nlohmann::json& jsonSimObject):
BaseSceneNode<SimObject>{jsonSimObject},
Resource<SimObject>{0}
{
    addComponent<SimCore>({this}, true);
}

SimObject::SimObject(const SimObject& simObject):
BaseSceneNode<SimObject> { simObject },
Resource<SimObject>{0}
{
    updateComponent<SimCore>({this});
}

void SimObject::onActivated() {
    for(auto& aspectPair: mSimObjectAspects) {
        aspectPair.second->onActivated_();
    }
}
void SimObject::onDeactivated() {
    for(auto& aspectPair: mSimObjectAspects) {
        aspectPair.second->onDeactivated_();
    }
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

bool SimSystem::aspectRegistered(const std::string& aspectName) const {
    return mAspectConstructors.find(aspectName) != mAspectConstructors.end();
}

std::shared_ptr<BaseSystem> SimSystem::instantiate(ECSWorld& world) {
    std::shared_ptr<SimSystem> newSimSystem { std::static_pointer_cast<SimSystem>(System<SimSystem, SimCore>::instantiate(world)) };
    newSimSystem->mAspectConstructors = mAspectConstructors;
    return newSimSystem;
}

std::shared_ptr<BaseSimObjectAspect> SimSystem::constructAspect(const nlohmann::json& jsonAspectProperties) {
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
    const std::string& aspectType { aspect.getAspectTypeName() };
    mSimObjectAspects.try_emplace(aspect.getAspectTypeName(), aspect.makeCopy());
    mSimObjectAspects.at(aspectType)->attach(this);
}

void SimObject::addAspect(const nlohmann::json& jsonAspectProperties) {
    const std::string& aspectType { jsonAspectProperties.at("type").get<std::string>() };
    mSimObjectAspects.try_emplace(
        aspectType,
        getWorld().getSystem<SimSystem>()->constructAspect(jsonAspectProperties)
    );
    mSimObjectAspects.at(aspectType)->attach(this);
}

bool SimObject::hasAspect(const std::string& aspectType) const {
    return mSimObjectAspects.find(aspectType) != mSimObjectAspects.end();
}

void SimObject::removeAspect(const std::string& aspectType) {
    mSimObjectAspects.erase(aspectType);
}

BaseSimObjectAspect& SimObject::getAspect(const std::string& aspectType) {
    return *mSimObjectAspects.at(aspectType);
}

void BaseSimObjectAspect::attach(SimObject* newOwner) {
    detach();

    mSimObject = newOwner;
    onAttached_();
    if(mSimObject->isActive()) {
        onActivated_();
    }
}

void BaseSimObjectAspect::detach() {
    if(!mSimObject) return;

    if(mSimObject->isActive()) { 
        onDeactivated_();
    }
    onDetached_();
    mSimObject = nullptr;
}

void BaseSimObjectAspect::addAspect(const BaseSimObjectAspect& aspect) {
    mSimObject->addAspect(aspect);
}

void BaseSimObjectAspect::addAspect(const nlohmann::json& jsonAspectProperties) {
    mSimObject->addAspect(jsonAspectProperties);
}
BaseSimObjectAspect& BaseSimObjectAspect::getAspect(const std::string& aspectType) {
    return mSimObject->getAspect(aspectType);
}

bool BaseSimObjectAspect::hasAspect(const std::string& aspectType) const {
    return mSimObject->hasAspect(aspectType);
}

std::weak_ptr<FixedActionBinding> BaseSimObjectAspect::declareFixedActionBinding(
    const std::string& context,
    const std::string& action,
    std::function<void(const ActionData&, const ActionDefinition&)> handler
) {
    assert(!(mState & AspectState::ACTIVE) && "Cannot add or remove fixed action bindings while aspect is active.");
    assert(mFixedActionBindings.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(context, action),
        std::forward_as_tuple(new FixedActionBinding{context, action, handler})
    ).second == true && "Could not create this binding; perhaps a similar binding has already been registered");
    return mFixedActionBindings.at({context, action});
}

void BaseSimObjectAspect::activateFixedActionBindings(){
    assert((mState == (AspectState::ATTACHED|AspectState::ACTIVE)) && "Action bindings may only be activated or deactivated if an aspect is attached to an active SimObject");
    for(const auto& contextActionHandlerPair: mFixedActionBindings) {
        const std::string& context{ contextActionHandlerPair.first.first };
        const std::string& action { contextActionHandlerPair.first.second };
        Application::getInstance()
            .getObject<InputManager&>()[context]
            .registerActionHandler(
                action,
                shared_from_this()
        );
    }
}

void BaseSimObjectAspect::handleAction(const ActionData& actionData, const ActionDefinition& actionDefinition) {
    mFixedActionBindings
        .at({actionDefinition.mContext, actionDefinition.mName})
        ->call(actionData, actionDefinition);
}

void BaseSimObjectAspect::deactivateFixedActionBindings() {
    assert((mState == (AspectState::ATTACHED|AspectState::ACTIVE)) && "Action bindings may only be activated or deactivated if an aspect is attached to an active SimObject");
    for(const auto& contextActionHandlerPair: mFixedActionBindings) {
        const std::string& context{ contextActionHandlerPair.first.first };
        const std::string& action { contextActionHandlerPair.first.second };
        Application::getInstance()
            .getObject<InputManager&>()[context]
            .unregisterActionHandler(
                action,
                shared_from_this()
        );
    }
}

ECSWorld& BaseSimObjectAspect::getWorld() const {
    assert((mState&AspectState::ACTIVE) && "This aspect is not active, and therefore does not have access to its SimObject's world");
    return mSimObject->getWorld();
}

void BaseSimObjectAspect::onAttached_() {
    mState |= AspectState::ATTACHED;
    onAttached();
}

void BaseSimObjectAspect::onDetached_() {
    onDetached();
    mState &= ~AspectState::ATTACHED;
}

void BaseSimObjectAspect::onActivated_() {
    mState |= AspectState::ACTIVE;
    activateFixedActionBindings();
    onActivated();
}

void BaseSimObjectAspect::onDeactivated_() {
    onDeactivated();
    deactivateFixedActionBindings();
    mState &= ~AspectState::ACTIVE;
}
