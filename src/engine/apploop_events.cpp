#include "stdint.h"
#include "apploop_events.hpp"

ApploopEventDispatcher& ApploopEventDispatcher::getInstance() {
    static ApploopEventDispatcher instance {};
    return instance;
}

void ApploopEventDispatcher::INTERNAL_registerTimelineEventHandler_(std::weak_ptr<INTERNAL_ApploopEventHandler_> timelineEventHandler) {
    getInstance().mHandlers.insert(timelineEventHandler);
}

void ApploopEventDispatcher::applicationInitialize() {
    ApploopEventDispatcher& caller { getInstance() };
    std::set<std::weak_ptr<INTERNAL_ApploopEventHandler_>, std::owner_less<std::weak_ptr<INTERNAL_ApploopEventHandler_>>> eraseables {};
    for(auto& handler: caller.mHandlers) {
        if(handler.expired()) {
            eraseables.insert(handler);
            continue;
        }
        handler.lock()->onApplicationInitialize();
    }
    for(auto& eraseable: eraseables) {
        caller.mHandlers.erase(eraseable);
    }
}

void ApploopEventDispatcher::applicationStart() {
    ApploopEventDispatcher& caller { getInstance() };
    std::set<std::weak_ptr<INTERNAL_ApploopEventHandler_>, std::owner_less<std::weak_ptr<INTERNAL_ApploopEventHandler_>>> eraseables {};
    for(auto& handler: caller.mHandlers) {
        if(handler.expired()) {
            eraseables.insert(handler);
            continue;
        }
        handler.lock()->onApplicationStart();
    }
    for(auto& eraseable: eraseables) {
        caller.mHandlers.erase(eraseable);
    }
}

void ApploopEventDispatcher::applicationEnd() {
    ApploopEventDispatcher& caller { getInstance() };
    std::set<std::weak_ptr<INTERNAL_ApploopEventHandler_>, std::owner_less<std::weak_ptr<INTERNAL_ApploopEventHandler_>>> eraseables {};
    for(auto& handler: caller.mHandlers) {
        if(handler.expired()) {
            eraseables.insert(handler);
            continue;
        }
        handler.lock()->onApplicationEnd();
    }
    for(auto& eraseable: eraseables) {
        caller.mHandlers.erase(eraseable);
    }
}

void ApploopEventDispatcher::simulationStep(uint32_t simulationTimestep) {
    ApploopEventDispatcher& caller { getInstance() };
    std::set<std::weak_ptr<INTERNAL_ApploopEventHandler_>, std::owner_less<std::weak_ptr<INTERNAL_ApploopEventHandler_>>> eraseables {};
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

void ApploopEventDispatcher::postSimulationStep(float simulationProgress) {
    ApploopEventDispatcher& caller { getInstance() };
    std::set<std::weak_ptr<INTERNAL_ApploopEventHandler_>, std::owner_less<std::weak_ptr<INTERNAL_ApploopEventHandler_>>> eraseables {};   
    for(auto& handler: caller.mHandlers) {
        if(handler.expired()) {
            eraseables.insert(handler);
            continue;
        }
        handler.lock()->onPostSimulationStep(simulationProgress);
    }

    for(auto& eraseable: eraseables) {
        caller.mHandlers.erase(eraseable);
    }
}

void ApploopEventDispatcher::preRenderStep(float simulationProgress) {
    ApploopEventDispatcher& caller { getInstance() };
    std::set<std::weak_ptr<INTERNAL_ApploopEventHandler_>, std::owner_less<std::weak_ptr<INTERNAL_ApploopEventHandler_>>> eraseables {};
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
    std::set<std::weak_ptr<INTERNAL_ApploopEventHandler_>, std::owner_less<std::weak_ptr<INTERNAL_ApploopEventHandler_>>> eraseables {};
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
