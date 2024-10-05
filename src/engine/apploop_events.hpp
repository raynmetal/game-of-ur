#ifndef ZOAPPLOOPEVENTS_H
#define ZOAPPLOOPEVENTS_H

#include <vector>
#include <memory>
#include <set>
#include <stdint.h>

class INTERNAL_ApploopEventHandler_;

class ApploopEventDispatcher {
public:
    static void simulationStep(uint32_t simulationTimestep);
    static void postSimulationStep(float simulationProgress);
    static void preRenderStep(float simulationProgress);
    static void postRenderStep(float simulationProgress);
    static void INTERNAL_registerTimelineEventHandler_(std::weak_ptr<INTERNAL_ApploopEventHandler_> timelineEventHandler);
private:
    ApploopEventDispatcher(){};
    static ApploopEventDispatcher& getInstance();

    std::set<std::weak_ptr<INTERNAL_ApploopEventHandler_>, std::owner_less<std::weak_ptr<INTERNAL_ApploopEventHandler_>>> mHandlers {};

friend class INTERNAL_ApploopEventHandler_;
};

class INTERNAL_ApploopEventHandler_ {
private:
    struct Private {};

public:
    ~INTERNAL_ApploopEventHandler_() = default;
    INTERNAL_ApploopEventHandler_(Private) {}
private:
    virtual void onSimulationStep(uint32_t simulationTimestep) {}
    virtual void onPostSimulationStep(float simulationProgress) {}
    virtual void onPreRenderStep(float simulationProgress) {}
    virtual void onPostRenderStep(float simulationProgress) {}

friend class ApploopEventDispatcher;
};

template<class TDerived>
class IApploopEventHandler : public INTERNAL_ApploopEventHandler_{
public:
    IApploopEventHandler(): INTERNAL_ApploopEventHandler_{{}} {}

    template<typename ...TArgs>
    static std::shared_ptr<TDerived> registerHandler(TArgs &&... args);

private:
friend class ApploopEventDispatcher;
friend class INTERNAL_ApploopEventHandler_;
};

template<typename TDerived>
template<typename ...TArgs>
std::shared_ptr<TDerived> IApploopEventHandler<TDerived>::registerHandler(TArgs &&... args) {
    // Create and initialize the handler ...
    std::shared_ptr<TDerived> sharedPointer { new TDerived };

    // ... (with whatever it asks to be initialized with) ...
    sharedPointer->initializeEventHandler(args...);

    // ... and then register a weak pointer to this handler with the event dispatcher ...
    ApploopEventDispatcher::INTERNAL_registerTimelineEventHandler_(std::static_pointer_cast<INTERNAL_ApploopEventHandler_>(sharedPointer));

    // ... finally giving the pointer to its (first) owner
    return sharedPointer;
}

#endif
