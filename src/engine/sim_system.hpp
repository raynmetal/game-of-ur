#ifndef ZOSIMSYSTEM_H
#define ZOSIMSYSTEM_H

#include <memory>
#include <set>
#include <cassert>
#include <typeinfo>

#include "apploop_events.hpp" 
#include "simple_ecs.hpp"

class SimComponent;
class SimObject;


class SimSystem: public System<SimSystem> {
public:
    struct SimCore {
        void* mSimObject;
    };

    class ApploopEventHandler : public IApploopEventHandler<ApploopEventHandler> {
    public:
        ApploopEventHandler(){}
        inline void initializeEventHandler(SimSystem* pSystem) {mSystem = pSystem;}
    private:
        void onSimulationStep(uint32_t simulationTicks) override;
        SimSystem* mSystem;
        uint32_t mSimulationTicks {};
    friend class SimSystem;
    };

    template <typename ...TCoreComponentArgs>
    std::shared_ptr<SimObject> createSimObject(TCoreComponentArgs &&... coreComponentArgs);

private:
    std::shared_ptr<SimSystem::ApploopEventHandler> mApploopEventHandler { SimSystem::ApploopEventHandler::registerHandler(this) };
friend class SimSystem::ApploopEventHandler;
};

/**
 * A wrapper on entity that provides access to sim components and
 * sim logic
 */
class SimObject {
public:

    SimObject(const SimObject& other) = delete;
    SimObject(SimObject&& other);
    
    SimObject& operator=(const SimObject& other) = delete;
    SimObject& operator=(SimObject&& other);

    template <typename TSimComponent, typename ...TSimComponentArgs>
    void addComponent(TSimComponentArgs &&... simComponentArgs);
    template <typename TSimComponent>
    bool hasComponent();
    template <typename TSimComponent>
    TSimComponent& getComponent();
    template <typename TSimComponent>
    void removeComponent();

private:
    template <typename ...TCoreComponentArgs>
    SimObject(void* fake, TCoreComponentArgs &&... coreComponentArgs);

    Entity& getEntity();
    void update(uint32_t deltaSimTimeMillis);

    std::unique_ptr<Entity> mEntity { std::make_unique<Entity>(SimpleECS::createEntity( SimSystem::SimCore{this} )) };
    std::unique_ptr<std::unordered_map<std::size_t, std::unique_ptr<SimComponent>>> mSimComponents { nullptr };

friend class SimSystem;
friend class SimComponent;
};

class SimComponent {
public:
    virtual ~SimComponent()=default;

    virtual void update(uint32_t deltaSimTimeMillis) {};
    virtual void onCreate() {};
    virtual void onDestroy() {};

protected:
    SimComponent(SimObject* simObject): mSimObject { simObject } {};

    template <typename TCoreComponent>
    void addCoreComponent(const TCoreComponent& initialComponentValue);
    template <typename TCoreComponent>
    TCoreComponent getCoreComponent();
    template <typename TCoreComponent>
    void updateCoreComponent(const TCoreComponent& coreComponent);
    template <typename TCoreComponent>
    void removeCoreComponent();

    template <typename TSimComponent, typename ...TSimComponentArgs>
    void addComponent(TSimComponentArgs &&... simComponentArgs);
    template <typename TSimComponent>
    bool hasComponent();
    template <typename TSimComponent>
    TSimComponent& getComponent();
    template <typename TSimComponent>
    void removeComponent();

private:
    SimObject* mSimObject;
friend class SimObject;
};

template <typename ...TCoreComponentArgs>
std::shared_ptr<SimObject> SimSystem::createSimObject(TCoreComponentArgs &&... coreComponentArgs) {
    return std::make_shared<SimObject>(nullptr, coreComponentArgs...);
}

template <typename TSimComponent, typename ...TSimComponentArgs>
void SimObject::addComponent(TSimComponentArgs &&... simComponentArgs) {
    std::size_t componentHash { typeid(TSimComponent).hash_code() };
    constexpr bool isDerivedFromSimComponent { std::is_base_of<SimComponent, TSimComponent>::value };
    static_assert(isDerivedFromSimComponent && "Component object must be a subclass of SimComponent");
    mSimComponents->try_emplace(componentHash, std::make_unique<TSimComponent>( *this,  simComponentArgs... ));
}

template <typename TSimComponent>
bool SimObject::hasComponent() {
    std::size_t componentHash { typeid(TSimComponent).hash_code() };
    return mSimComponents->find(componentHash) != mSimComponents->end();
}

template <typename TSimComponent>
TSimComponent& SimObject::getComponent() {
    std::size_t componentHash { typeid(TSimComponent).hash_code() };
    return *(std::static_pointer_cast<TSimComponent>(mSimComponents->at(componentHash)));
}

template <typename TSimComponent>
void SimObject::removeComponent() {
    std::size_t componentHash { typeid(TSimComponent).hash_code() };
    mSimComponents->erase(componentHash);
}

template<typename ...TCoreComponents>
SimObject::SimObject(void*, TCoreComponents &&... coreComponents) :
    mEntity{ std::make_unique(SimpleECS::createEntity(SimSystem::SimCore { this }, coreComponents...)) }
{
    mSimComponents = std::make_unique<std::unordered_map<std::size_t, std::unique_ptr<SimComponent>>>();
}

template <typename TSimComponent, typename ...TSimComponentArgs>
void SimComponent::addComponent(TSimComponentArgs &&... simComponentArgs) {
    mSimObject->addComponent<TSimComponent>(simComponentArgs...);
}

template <typename TSimComponent>
void SimComponent::removeComponent() {
    mSimObject->removeComponent<TSimComponent>();
}

template <typename TSimComponent>
TSimComponent& SimComponent::getComponent() {
    return mSimObject->getComponent<TSimComponent>();
}

template <typename TSimComponent>
bool SimComponent::hasComponent() {
    return mSimObject->hasComponent<TSimComponent>();
}

template <typename TCoreComponent>
TCoreComponent SimComponent::getCoreComponent() {
    return mSimObject->getEntity().getComponent<TCoreComponent>();
}

template <typename TCoreComponent>
void SimComponent::addCoreComponent(const TCoreComponent& initialComponentValue) {
    mSimObject->getEntity().addComponent(initialComponentValue);
}

template <typename TCoreComponent>
void SimComponent::updateCoreComponent(const TCoreComponent& componentValue) {
    mSimObject->getEntity().updateComponent(componentValue);
}

template <typename TCoreComponent>
void SimComponent::removeCoreComponent() {
    mSimObject->getEntity().removeComponent<TCoreComponent>();
}

template<>
inline SimSystem::SimCore Interpolator<SimSystem::SimCore>::operator() (const SimSystem::SimCore& prev, const SimSystem::SimCore& next, float value) const {
    return next;
}

#endif
