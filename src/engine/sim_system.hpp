#ifndef ZOSIMSYSTEM_H
#define ZOSIMSYSTEM_H

#include <memory>
#include <set>
#include <cassert>
#include <typeinfo>
#include <type_traits>

#include <nlohmann/json.hpp>

#include "registrator.hpp"
#include "apploop_events.hpp" 
#include "simple_ecs.hpp"
#include "scene_system.hpp"
#include "signals.hpp"

class BaseSimObjectAspect;
class SimObject;
class SimSystem;

struct SimCore {
    inline static std::string getComponentTypeName() { return "SimCore"; }
    SimObject* mSimObject;
};

// never used, so make empty definitions for these
inline void from_json(const nlohmann::json& json, SimCore& simCore) {}
// never used, so make empty definitions for these
inline void to_json(nlohmann::json& json, const SimCore& simCore) {}

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
    template <typename TSimObjectAspect>
    void registerAspect();

    std::unique_ptr<BaseSimObjectAspect> constructAspect(const nlohmann::json& jsonAspectProperties);

    std::unordered_map<std::string, std::unique_ptr<BaseSimObjectAspect> (*)(const nlohmann::json& jsonAspectProperties)> mAspectConstructors {};
    std::shared_ptr<SimSystem::ApploopEventHandler> mApploopEventHandler { SimSystem::ApploopEventHandler::registerHandler(this) };
friend class SimSystem::ApploopEventHandler;
friend class BaseSimObjectAspect;
friend class SimObject;
};

/**
 * A wrapper on entity that provides access to sim components and
 * sim logic
 */
class SimObject: public BaseSceneNode<SimObject>, public Resource<SimObject> {
public:
    ~SimObject();
    inline static std::string getResourceTypeName() { return "SimObject"; }

    template <typename ...TComponents>
    static std::shared_ptr<SimObject> create(const Placement& placement, const std::string& name, TComponents...components);
    static std::shared_ptr<SimObject> create(const nlohmann::json& jsonSimObject);
    static std::shared_ptr<SimObject> copy(const std::shared_ptr<const SimObject> simObject);

    void addAspect(const nlohmann::json& jsonAspectProperties);
    void addAspect(const BaseSimObjectAspect& simObjectAspect);
    template <typename TSimObjectAspect>
    bool hasAspect() const;
    bool hasAspect(const std::string& aspectType) const;
    template <typename TSimObjectAspect>
    TSimObjectAspect& getAspect();
    BaseSimObjectAspect& getAspect(const std::string& aspectType);
    template <typename TSimObjectAspect>
    void removeAspect();
    void removeAspect(const std::string& aspectType);

protected:
    template <typename ...TComponents>
    SimObject(const Placement& placement, const std::string& name, TComponents...components);
    SimObject(const nlohmann::json& jsonSimObject);
    SimObject(const SceneNodeCore& sceneNode);
    SimObject(const SimObject& other);

    // // TODO: sit down and figure out whether this operator will ever actually be useful
    // SimObject& operator=(const SimObject& other);

private:
    void update(uint32_t deltaSimTimeMillis);

    void copyAspects(const SimObject& other);
    std::shared_ptr<SceneNodeCore> clone() const override;

    std::unordered_map<std::string, std::unique_ptr<BaseSimObjectAspect>> mSimObjectAspects { };
    SignalTracker mSignalTracker {};

friend class SimSystem;
friend class BaseSimObjectAspect;
friend class BaseSceneNode<SimObject>;
};

class BaseSimObjectAspect : public SignalTracker {
public:
    virtual ~BaseSimObjectAspect();

    virtual void update(uint32_t deltaSimTimeMillis) {}
    virtual void onAttached(){}
    virtual void onDetached(){}
    virtual void onActivated() {}
    virtual void onDeactivated() {}

protected:
    BaseSimObjectAspect()=default;

    template <typename TSimObjectAspectDerived>
    static inline void registerAspect() {
        //ensure registration of SimSystem before trying to register
        //this aspect
        auto& simSystemRegistrator { 
            Registrator<System<SimSystem, SimCore>>::getRegistrator()
        };
        simSystemRegistrator.emptyFunc();

        // Let SimSystem know that this type of aspect exists
        SimpleECS::getSystem<SimSystem>()->registerAspect<TSimObjectAspectDerived>();
    }

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

    void addAspect(const nlohmann::json& jsonAspectProperties);
    void addAspect(const BaseSimObjectAspect& aspect);
    template <typename TSimObjectAspect>
    bool hasAspect() const;
    bool hasAspect(const std::string& aspectType) const;
    template <typename TSimObjectAspect>
    TSimObjectAspect& getAspect();
    BaseSimObjectAspect& getAspect(const std::string& aspectType);
    template <typename TSimObjectAspect>
    void removeAspect();

    EntityID getEntityID() const;

    virtual std::string getAspectTypeName() const = 0;
private:
    void attach(SimObject* owner);
    void detach();
    virtual std::unique_ptr<BaseSimObjectAspect> makeCopy() const = 0;
    SimObject* mSimObject { nullptr };
friend class SimObject;
};

template <typename TSimObjectAspectDerived>
class SimObjectAspect: public BaseSimObjectAspect {
protected:
    SimObjectAspect(int explicitlyInitializeMe){
        s_registrator.emptyFunc();
    }
private:
    inline std::string getAspectTypeName() const override {
        return TSimObjectAspectDerived::getSimObjectAspectTypeName();
    }
    static inline void registerSelf() {
        BaseSimObjectAspect::registerAspect<TSimObjectAspectDerived>();
    }
    static Registrator<SimObjectAspect<TSimObjectAspectDerived>>& s_registrator;
friend class Registrator<SimObjectAspect<TSimObjectAspectDerived>>;
friend class SimObject;
};

template<typename TSimObjectAspectDerived>
Registrator<SimObjectAspect<TSimObjectAspectDerived>>& SimObjectAspect<TSimObjectAspectDerived>::s_registrator{ 
    Registrator<SimObjectAspect<TSimObjectAspectDerived>>::getRegistrator()
};

template <typename TSimObjectAspect>
void SimSystem::registerAspect() {
    assert((std::is_base_of<BaseSimObjectAspect, TSimObjectAspect>::value) && "Type being registered must be a subclass of BaseSimObjectAspect");
    mAspectConstructors.insert_or_assign(
        TSimObjectAspect::getSimObjectAspectTypeName(),
        &(TSimObjectAspect::create)
    );
}

template <typename ...TComponents>
std::shared_ptr<SimObject> SimObject::create(const Placement& placement, const std::string& name, TComponents...components) {
    return SceneNode::create<SimObject, TComponents...>(placement, name, components...);
}

template <typename ...TComponents>
SimObject::SimObject(const Placement& placement, const std::string& name, TComponents ... components) :
BaseSceneNode<SimObject> { placement, name, SimCore{this}, components... },
Resource<SimObject>{0}
{}

template <typename TComponent>
void BaseSimObjectAspect::addComponent(const TComponent& component) {
    mSimObject->addComponent<TComponent>(component);
}
template <typename TComponent>
bool BaseSimObjectAspect::hasComponent() {
    return mSimObject->hasComponent<TComponent>();
}
template <typename TComponent>
void BaseSimObjectAspect::updateComponent(const TComponent& component) {
    mSimObject->updateComponent<TComponent>(component);
}
template <typename TComponent>
TComponent BaseSimObjectAspect::getComponent() {
    return mSimObject->getComponent<TComponent>();
}
template <typename TComponent>
void BaseSimObjectAspect::removeComponent() {
    return mSimObject->removeComponent<TComponent>();
}

template <typename TSimObjectAspect>
bool SimObject::hasAspect() const {
    return mSimObjectAspects.find(TSimObjectAspect::getSimObjectAspectTypeName()) != mSimObjectAspects.end();
}

template <typename TSimObjectAspect>
TSimObjectAspect& SimObject::getAspect() {
    return *(static_cast<TSimObjectAspect*>(mSimObjectAspects.at(TSimObjectAspect::getSimObjectAspectTypeName()).get()));
}

template <typename TSimObjectAspect>
void SimObject::removeAspect() {
    mSimObjectAspects.erase(TSimObjectAspect::getSimObjectAspectTypeName());
}

template <typename TSimObjectAspect>
void BaseSimObjectAspect::removeAspect() {
    mSimObject->removeAspect<TSimObjectAspect>();
}

template <typename TSimObjectAspect>
TSimObjectAspect& BaseSimObjectAspect::getAspect() {
    return mSimObject->getAspect<TSimObjectAspect>();
}

template <typename TSimObjectAspect>
bool BaseSimObjectAspect::hasAspect() const {
    return mSimObject->hasAspect<TSimObjectAspect>();
}

template<>
inline SimCore Interpolator<SimCore>::operator() (const SimCore& prev, const SimCore& next, float value) const {
    return next;
}

template <>
inline void SceneNodeCore::removeComponent<SimCore>() {
    assert(false && "Cannot remove a sim object's sim core component.");
}

#endif
