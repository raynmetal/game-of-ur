#include "back_and_forth.hpp"
#include "../engine/scene_system.hpp"

#include "glm/glm.hpp"

void BackAndForth::onAttach() {
    mSigDirectionChanged = std::make_unique<Signal<float>>(getSignalTrackerReference(), "directionChanged");
}

void BackAndForth::update(uint32_t deltaSimtimeMillis) {
    mElapsedTime += deltaSimtimeMillis;
    Placement placement  { getComponent<Placement>() };
    const float prevZ { placement.mPosition.z };
    placement.mPosition.z = glm::sin(glm::radians(mElapsedTime/10.f + getEntityID()*45.f));

    const float prevDelta { mDelta };
    mDelta = placement.mPosition.z - prevZ;
    if(std::signbit(prevDelta) != std::signbit(mDelta)) {
        const float direction { mDelta < 0.f? -1.f: 1.f };
        mSigDirectionChanged->emit(direction);
    }

    updateComponent<Placement>(placement);
}

std::unique_ptr<BaseSimObjectAspect> BackAndForth::makeCopy() const {
    return std::unique_ptr<BackAndForth>(new BackAndForth{});
}

std::unique_ptr<BaseSimObjectAspect> BackAndForth::create(const nlohmann::json& jsonAspectProperties) {
    return std::unique_ptr<BackAndForth>(new BackAndForth{});
}
