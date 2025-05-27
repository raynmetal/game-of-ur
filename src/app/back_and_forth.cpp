#include <iostream>

#include <glm/glm.hpp>

#include "../engine/scene_system.hpp"

#include "back_and_forth.hpp"

void BackAndForth::variableUpdate(uint32_t variableStepMillis) {
    mElapsedTime += variableStepMillis;
    ToyMakersEngine::Placement placement  { getComponent<ToyMakersEngine::Placement>() };
    const float prevZ { placement.mPosition.z };
    placement.mPosition.z = glm::sin(glm::radians(mElapsedTime/10.f + getEntityID()*45.f));

    const float prevDelta { mDelta };
    mDelta = placement.mPosition.z - prevZ;
    if(std::signbit(prevDelta) != std::signbit(mDelta)) {
        const float direction { mDelta < 0.f? -1.f: 1.f };
        mSigDirectionChanged.emit(direction);
    }

    updateComponent<ToyMakersEngine::Placement>(placement);
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> BackAndForth::clone() const {
    return std::shared_ptr<BackAndForth>(new BackAndForth{});
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> BackAndForth::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<BackAndForth>(new BackAndForth{});
}
