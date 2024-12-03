#ifndef ZOSIMSYSTEM_H
#define ZOSIMSYSTEM_H

#include <memory>
#include <set>
#include <cassert>
#include <typeinfo>
#include <nlohmann/json.hpp>

#include "apploop_events.hpp" 
#include "simple_ecs.hpp"
#include "scene_system.hpp"

class SimObjectAspect;
class SimObject;
class SimSystem;

struct SimCore {
    SimObject* mSimObject;
};

class SimSystem: public System<SimSystem, SimCore> {
public:
    SimSystem():
    System<SimSystem, SimCore>{0}
    {}

    class ApploopEventHandler : public IApploopEventHandler<ApploopEventHandler> {
    public:
        ApploopEventHandler(){}
        inline void initializeEventHandler(SimSystem* pSystem) {mSystem = pSystem;}

    private:
        void onSimulationStep(uint32_t simulationTicks) override;
        SimSystem* mSystem;
    friend class SimSystem;
    };

private:
    std::shared_ptr<SimSystem::ApploopEventHandler> mApploopEventHandler { SimSystem::ApploopEventHandler::registerHandler(this) };
friend class SimSystem::ApploopEventHandler;
};

/**
 * A wrapper on entity that provides access to sim components and
 * sim logic
 */
class SimObject: public SceneNode {
public:
    template <typename ...TComponents>
    static std::shared_ptr<SimObject> create(const Placement& placement, const std::string& name, TComponents...components);

    static std::shared_ptr<SimObject> copy(const std::shared_ptr<SimObject> simObject);

    template <typename ...TComponents>
    SimObject(const Placement& placement, const std::string& name, TComponents...components);
    SimObject(const SceneNode& sceneNode);
    SimObject(SceneNode&& sceneNode);
    SimObject(const SimObject& other);
    SimObject(SimObject&& other);

    SimObject& operator=(const SimObject& other);
    SimObject& operator=(SimObject&& other);

    template <typename TSimObjectAspect, typename ...TSimObjectAspectArgs>
    void addAspect(TSimObjectAspectArgs ... simObjectAspectArgs);
    template <typename TSimObjectAspect>
    void addAspect(const TSimObjectAspect& simObjectAspect);
    template <typename TSimObjectAspect>
    bool hasAspect();
    template <typename TSimObjectAspect>
    TSimObjectAspect& getAspect();
    template <typename TSimObjectAspect>
    void removeAspect();

private:
    void update(uint32_t deltaSimTimeMillis);

    std::unordered_map<std::size_t, std::unique_ptr<SimObjectAspect>> mSimObjectAspects { };

friend class SimSystem;
friend class SimObjectAspect;
};

class SimObjectAspect {
public:
    virtual ~SimObjectAspect()=default;

    virtual void update(uint32_t deltaSimTimeMillis) {};
    virtual void onCreate() {};
    virtual void onDestroy() {};
    
protected:
    SimObjectAspect(SimObject* simObject): mSimObject { simObject } {};

    template <typename TComponent>
    void addComponent(const TComponent& component);
    template <typename TComponent>
    bool hasComponent();
    template <typename TComponent>
    void updateComponent(const TComponent& component);
    template <typename TComponent>
    TComponent getComponent();
    template <typename TComponent>
    void removeComponent();


    template <typename TSimObjectAspect, typename ... TSimObjectAspectArgs>
    void addAspect(TSimObjectAspectArgs ... simObjectAspectArgs);
    template <typename TSimObjectAspect>
    void addAspect(const TSimObjectAspect& aspect);
    template <typename TSimObjectAspect>
    bool hasAspect();
    template <typename TSimObjectAspect>
    TSimObjectAspect& getAspect();
    template <typename TSimObjectAspect>
    void removeAspect();

    EntityID getEntityID() const;

private:
    virtual std::unique_ptr<SimObjectAspect> makeCopy() const = 0;
    SimObject* mSimObject { nullptr };
friend class SimObject;
};

template <typename ...TComponents>
SimObject::SimObject(const Placement& placement, const std::string& name, TComponents ... components) :
SceneNode { placement, name, SimCore{this}, components... }
{}


template <typename ...TComponents>
std::shared_ptr<SimObject> SimObject::create(const Placement& placement, const std::string& name, TComponents ... components) {
    return std::make_shared<SimObject>(placement, name, components...);
}

template <typename TComponent>
void SimObjectAspect::addComponent(const TComponent& component) {
    mSimObject->addComponent<TComponent>(component);
}
template <typename TComponent>
bool SimObjectAspect::hasComponent() {
    return mSimObject->hasComponent<TComponent>();
}
template <typename TComponent>
void SimObjectAspect::updateComponent(const TComponent& component) {
    mSimObject->updateComponent<TComponent>(component);
}
template <typename TComponent>
TComponent SimObjectAspect::getComponent() {
    return mSimObject->getComponent<TComponent>();
}

template <typename TComponent>
void SimObjectAspect::removeComponent() {
    return mSimObject->removeComponent<TComponent>();
}


template <typename TSimObjectAspect>
bool SimObject::hasAspect() {
    std::size_t componentHash { typeid(TSimObjectAspect).hash_code() };
    return mSimObjectAspects.find(componentHash) != mSimObjectAspects.end();
}

template <typename TSimObjectAspect>
TSimObjectAspect& SimObject::getAspect() {
    std::size_t componentHash { typeid(TSimObjectAspect).hash_code() };
    return *(static_cast<TSimObjectAspect*>(mSimObjectAspects.at(componentHash).get()));
}

template <typename TSimObjectAspect>
void SimObject::removeAspect() {
    std::size_t componentHash { typeid(TSimObjectAspect).hash_code() };
    mSimObjectAspects.erase(componentHash);
}

template <typename TSimObjectAspect>
void SimObject::addAspect(const TSimObjectAspect& aspect) {
    std::size_t componentHash { typeid(TSimObjectAspect).hash_code() };
    constexpr bool isDerivedFromsimObjectAspect { std::is_base_of<SimObjectAspect, TSimObjectAspect>::value };
    static_assert(isDerivedFromsimObjectAspect && "Component object must be a subclass of SimObjectAspect");
    mSimObjectAspects.try_emplace(componentHash, aspect.makeCopy());
    mSimObjectAspects.at(componentHash)->mSimObject = this;
}

template <typename TSimObjectAspect, typename ...TSimObjectAspectArgs>
void SimObject::addAspect(TSimObjectAspectArgs ... simObjectAspectArgs) {
    std::size_t componentHash { typeid(TSimObjectAspect).hash_code() };
    constexpr bool isDerivedFromsimObjectAspect { std::is_base_of<SimObjectAspect, TSimObjectAspect>::value };
    static_assert(isDerivedFromsimObjectAspect && "Component object must be a subclass of SimObjectAspect");
    mSimObjectAspects.try_emplace(componentHash, std::make_unique<TSimObjectAspect>( this,  simObjectAspectArgs... ));
}

template <typename TSimObjectAspect>
void SimObjectAspect::addAspect(const TSimObjectAspect& aspect) {
    mSimObject->addAspect(aspect);
}

template <typename TSimObjectAspect, typename ...TSimObjectAspectArgs>
void SimObjectAspect::addAspect(TSimObjectAspectArgs ... simObjectAspectArgs) {
    mSimObject->addAspect<TSimObjectAspect>(simObjectAspectArgs...);
}

template <typename TSimObjectAspect>
void SimObjectAspect::removeAspect() {
    mSimObject->removeAspect<TSimObjectAspect>();
}

template <typename TSimObjectAspect>
TSimObjectAspect& SimObjectAspect::getAspect() {
    return mSimObject->getAspect<TSimObjectAspect>();
}

template <typename TSimObjectAspect>
bool SimObjectAspect::hasAspect() {
    return mSimObject->hasAspect<TSimObjectAspect>();
}

template<>
inline SimCore Interpolator<SimCore>::operator() (const SimCore& prev, const SimCore& next, float value) const {
    return next;
}

template <>
inline void SceneNode::removeComponent<SimCore>() {
    assert(false && "Cannot remove a sim object's sim core component.");
}

#endif
