#ifndef ZOSIMSYSTEM_H
#define ZOSIMSYSTEM_H

#include <memory>
#include <set>
#include <cassert>
#include <functional>
#include <typeinfo>
#include <type_traits>

#include <nlohmann/json.hpp>

#include "core/resource_database.hpp"
#include "core/ecs_world.hpp"

#include "spatial_query_system.hpp"
#include "registrator.hpp"
#include "input_system/input_system.hpp"
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

class SimSystem: public System<SimSystem, std::tuple<>, std::tuple<SimCore>> {
public:
    explicit SimSystem(std::weak_ptr<ECSWorld> world):
    System<SimSystem, std::tuple<>, std::tuple<SimCore>>{world}
    {}

    static std::string getSystemTypeName() { return "SimSystem"; }

    bool aspectRegistered(const std::string& aspectName) const;

protected:
    virtual std::shared_ptr<BaseSystem> instantiate(std::weak_ptr<ECSWorld> world) override;

private:
    template <typename TSimObjectAspect>
    void registerAspect();

    std::shared_ptr<BaseSimObjectAspect> constructAspect(const nlohmann::json& jsonAspectProperties);
    void onSimulationStep(uint32_t simulationStepMillis) override;
    void onVariableStep(float simulationProgress, uint32_t variableStepMillis) override;

    std::unordered_map<std::string, std::shared_ptr<BaseSimObjectAspect> (*)(const nlohmann::json& jsonAspectProperties)> mAspectConstructors {};
friend class BaseSimObjectAspect;
friend class SimObject;
};

/**
 * A wrapper on entity that provides access to sim components and
 * sim logic
 */
class SimObject: public BaseSceneNode<SimObject>, public Resource<SimObject> {
public:
    ~SimObject() override;
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
    template <typename TInterface>
    bool hasAspectWithInterface() const;

    template <typename TSimObjectAspect>
    TSimObjectAspect& getAspect();
    BaseSimObjectAspect& getAspect(const std::string& aspectType);
    template <typename TInterface>
    std::vector<std::reference_wrapper<TInterface>> getAspectsWithInterface();

    template <typename TSimObjectAspect>
    void removeAspect();
    void removeAspect(const std::string& aspectType);

protected:
    template <typename ...TComponents>
    SimObject(const Placement& placement, const std::string& name, TComponents...components);
    SimObject(const nlohmann::json& jsonSimObject);
    SimObject(const SimObject& other);

    // // TODO: sit down and figure out whether this operator will ever actually be useful
    // SimObject& operator=(const SimObject& other);

private:
    void simulationUpdate(uint32_t simStepMillis);
    void variableUpdate(uint32_t variableStepMillis);

    void onActivated() override;
    void onDeactivated() override;

    void copyAspects(const SimObject& other);
    std::shared_ptr<SceneNodeCore> clone() const override;

    std::unordered_map<std::string, std::shared_ptr<BaseSimObjectAspect>> mSimObjectAspects {};

friend class SimSystem;
friend class BaseSimObjectAspect;
friend class BaseSceneNode<SimObject>;
};

class FixedActionBinding {
private:
    inline bool call(const ActionData& actionData, const ActionDefinition& actionDefinition) {
        return mHandler(actionData, actionDefinition);
    }
    FixedActionBinding(const std::string& context, const std::string& name, std::function<bool(const ActionData&, const ActionDefinition&)> handler):
    mContext { context },
    mName { name },
    mHandler { handler }
    {}
    std::string mContext;
    std::string mName;
    std::function<bool(const ActionData&, const ActionDefinition&)> mHandler;
friend class BaseSimObjectAspect;
};

class BaseSimObjectAspect : public std::enable_shared_from_this<BaseSimObjectAspect>, public SignalTracker, public IActionHandler {
public:
    virtual ~BaseSimObjectAspect()=default;
    virtual void simulationUpdate(uint32_t simStepMillis) {}
    virtual void variableUpdate(uint32_t variableStepMillis) {}

    bool handleAction(const ActionData& actionData, const ActionDefinition& actionDefinition) override final;
    ViewportNode& getLocalViewport();

protected:
    BaseSimObjectAspect()=default;

    BaseSimObjectAspect(const BaseSimObjectAspect& other)=delete;
    BaseSimObjectAspect(BaseSimObjectAspect&& other)=delete;

    template <typename TSimObjectAspectDerived>
    static inline void registerAspect() {
        //ensure registration of SimSystem before trying to register
        //this aspect
        auto& simSystemRegistrator { 
            Registrator<System<SimSystem, std::tuple<>, std::tuple<SimCore>>>::getRegistrator()
        };
        simSystemRegistrator.emptyFunc();

        // Let SimSystem know that this type of aspect exists
        ECSWorld::getSystemPrototype<SimSystem>()->registerAspect<TSimObjectAspectDerived>();
    }

    SimObject& getSimObject();

    template <typename TComponent>
    void addComponent(const TComponent& component);
    template <typename TComponent>
    bool hasComponent();
    template <typename TComponent>
    void updateComponent(const TComponent& component);
    template <typename TComponent>
    TComponent getComponent(const float simulationProgress=1.f);
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

    std::weak_ptr<FixedActionBinding> declareFixedActionBinding(const std::string& context, const std::string& action, std::function<bool(const ActionData&, const ActionDefinition&)>);
    EntityID getEntityID() const;
    std::weak_ptr<ECSWorld> getWorld() const;
    virtual std::string getAspectTypeName() const = 0;

private:
    void activateFixedActionBindings();
    void deactivateFixedActionBindings();

    void onAttached_();
    void onDetached_();
    void onActivated_();
    void onDeactivated_();

    virtual void onAttached(){}
    virtual void onDetached(){}
    virtual void onActivated() {}
    virtual void onDeactivated() {}

    inline bool isAttached() { return mState&AspectState::ATTACHED; }
    inline bool isActive() { return mState&AspectState::ACTIVE; }

    void attach(SimObject* owner);
    void detach();
    virtual std::shared_ptr<BaseSimObjectAspect> clone() const = 0;

    std::map<
        std::pair<std::string, std::string>,
        std::shared_ptr<FixedActionBinding>,
        std::less<std::pair<std::string, std::string>>
    > mFixedActionBindings {};
    SimObject* mSimObject { nullptr };

    enum AspectState : uint8_t {
        ATTACHED=1,
        ACTIVE=2,
    };
    uint8_t mState { 0x0 };

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
    return BaseSceneNode<SimObject>::create<SimObject, TComponents...>(placement, name, components...);
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
TComponent BaseSimObjectAspect::getComponent(const float simulationProgress) {
    return mSimObject->getComponent<TComponent>(simulationProgress);
}
template <typename TComponent>
void BaseSimObjectAspect::removeComponent() {
    return mSimObject->removeComponent<TComponent>();
}

template <typename TSimObjectAspect>
bool SimObject::hasAspect() const {
    return mSimObjectAspects.find(TSimObjectAspect::getSimObjectAspectTypeName()) != mSimObjectAspects.end();
}

template <typename TInterface>
bool SimObject::hasAspectWithInterface() const {
    for(auto aspect: mSimObjectAspects) {
        if(auto interfaceReference = std::dynamic_pointer_cast<TInterface>(aspect.second)){
            return true;
        }
    }
    return false;
}

template <typename TSimObjectAspect>
TSimObjectAspect& SimObject::getAspect() {
    return *(static_cast<TSimObjectAspect*>(mSimObjectAspects.at(TSimObjectAspect::getSimObjectAspectTypeName()).get()));
}

template <typename TInterface>
std::vector<std::reference_wrapper<TInterface>> SimObject::getAspectsWithInterface() {
    std::vector<std::reference_wrapper<TInterface>> interfaceImplementations {};
    for(auto aspect: mSimObjectAspects) {
        if(auto interfaceReference = std::dynamic_pointer_cast<TInterface>(aspect.second)) {
            interfaceImplementations.push_back(*interfaceReference);
        }
    }
    return interfaceImplementations;
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

template <>
struct SceneNodeCore::getByPath_Helper<BaseSimObjectAspect&> {
    static BaseSimObjectAspect& get(std::shared_ptr<SceneNodeCore> rootNode, const std::string& where) {
        std::string::const_iterator div {std::find(where.begin(), where.end(), '@')};
        assert(div != where.end() && "Must contain @ to be a valid path to a scene node's aspect");

        // extract the node path from full path
        const std::string nodePath { where.substr(0, div - where.begin()) };
        const std::string aspectName { where.substr(1 + (div - where.begin())) };
        assert(rootNode->getWorld().lock()->getSystem<SimSystem>()->aspectRegistered(aspectName) && "No aspect of this type has been registered with the Sim System");

        std::shared_ptr<SimObject> node { rootNode->getByPath<std::shared_ptr<SimObject>>(nodePath) };
        return node->getAspect(aspectName);
    }

    static constexpr bool s_valid { true };
};

template <typename TAspect>
struct SceneNodeCore::getByPath_Helper<TAspect&, std::enable_if_t<std::is_base_of<BaseSimObjectAspect, TAspect>::value>> {
    static TAspect& get(std::shared_ptr<SceneNodeCore> rootNode, const std::string& where) {
        return static_cast<TAspect&>(SceneNodeCore::getByPath_Helper<BaseSimObjectAspect&>::get(rootNode, where));
    }
    static constexpr bool s_valid { true };
};

template<>
inline SimCore Interpolator<SimCore>::operator() (const SimCore&, const SimCore& next, float) const {
    // Never return the previous state, as that is (supposed to be)
    // an invalidated reference to this node
    return next;
}

template <>
inline void SceneNodeCore::removeComponent<SimCore>() {
    assert(false && "Cannot remove a sim object's sim core component.");
}

#endif
