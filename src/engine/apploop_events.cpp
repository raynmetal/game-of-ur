#include "stdint.h"
#include "apploop_events.hpp"

ApploopEventDispatcher& ApploopEventDispatcher::getInstance() {
    static ApploopEventDispatcher instance {};
    return instance;
}

void ApploopEventDispatcher::registerTimelineEventHandler(std::weak_ptr<InternalApploopEventHandler> timelineEventHandler) {
    getInstance().mHandlers.insert(timelineEventHandler);
}

void ApploopEventDispatcher::simulationStep(uint32_t simulationTimestep) {
    ApploopEventDispatcher& caller { getInstance() };
    std::set<std::weak_ptr<InternalApploopEventHandler>, std::owner_less<std::weak_ptr<InternalApploopEventHandler>>> eraseables {};
    for(auto& handler: caller.mHandlers) {
        if(handler.expired()) {
            eraseables.insert(handler);
            continue;
        }
        handler.lock()->onSimulationStep(simulationTimestep);
    }

    for(auto& eraseable: eraseables) {
        caller.mHandlers.erase(eraseable);
    }
}

void ApploopEventDispatcher::preRenderStep(float simulationProgress) {
    ApploopEventDispatcher& caller { getInstance() };
    std::set<std::weak_ptr<InternalApploopEventHandler>, std::owner_less<std::weak_ptr<InternalApploopEventHandler>>> eraseables {};
    for(auto& handler: caller.mHandlers) {
        if(handler.expired()) {
            eraseables.emplace(handler);
            continue;
        }
        handler.lock()->onPreRenderStep(simulationProgress);
    }

    for(auto& eraseable: eraseables) {
        caller.mHandlers.erase(eraseable);
    }
}

void ApploopEventDispatcher::postRenderStep(float simulationProgress) {
    ApploopEventDispatcher& caller { getInstance() };
    std::set<std::weak_ptr<InternalApploopEventHandler>, std::owner_less<std::weak_ptr<InternalApploopEventHandler>>> eraseables {};
    for(auto& handler: caller.mHandlers) {
        if(handler.expired()) {
            eraseables.emplace(handler);
            continue;
        }
        handler.lock()->onPostRenderStep(simulationProgress);
    }

    for(auto& eraseable: eraseables) {
        caller.mHandlers.erase(eraseable);
    }
}

