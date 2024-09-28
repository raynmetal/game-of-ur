#ifndef ZOAPPLOOPEVENTS_H
#define ZOAPPLOOPEVENTS_H

#include <vector>
#include <memory>
#include <set>
#include <stdint.h>

class InternalApploopEventHandler;

class ApploopEventDispatcher {
public:
    static void simulationStep(uint32_t simulationTimestep);
    static void preRenderStep(float simulationProgress);
    static void postRenderStep(float simulationProgress);
    static void registerTimelineEventHandler(std::weak_ptr<InternalApploopEventHandler> timelineEventHandler);
private:
    ApploopEventDispatcher(){};
    static ApploopEventDispatcher& getInstance();

    std::set<std::weak_ptr<InternalApploopEventHandler>, std::owner_less<std::weak_ptr<InternalApploopEventHandler>>> mHandlers {};

friend class InternalApploopEventHandler;
};

class InternalApploopEventHandler {
private:
    struct Private {};

public:
    ~InternalApploopEventHandler() = default;
    InternalApploopEventHandler(Private) {}
private:
    virtual void onSimulationStep(uint32_t simulationTimestep) {}
    virtual void onPreRenderStep(float simulationProgress) {}
    virtual void onPostRenderStep(float simulationProgress) {}

friend class ApploopEventDispatcher;
};

template<class TDerived>
class IApploopEventHandler : public InternalApploopEventHandler{
public:
    IApploopEventHandler(): InternalApploopEventHandler{{}} {}

    template<typename ...TArgs>
    static std::shared_ptr<TDerived> registerHandler(TArgs &&... args);

private:
friend class ApploopEventDispatcher;
friend class InternalApploopEventHandler;
};

template<typename TDerived>
template<typename ...TArgs>
std::shared_ptr<TDerived> IApploopEventHandler<TDerived>::registerHandler(TArgs &&... args) {
    // Create and initialize the handler ...
    std::shared_ptr<TDerived> sharedPointer { new TDerived };

    // ... (with whatever it asks to be initialized with) ...
    sharedPointer->initializeEventHandler(args...);

    // ... and then register a weak pointer to this handler with the event dispatcher
    ApploopEventDispatcher::registerTimelineEventHandler(std::static_pointer_cast<InternalApploopEventHandler>(sharedPointer));

    return sharedPointer;
}

#endif
