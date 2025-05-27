#include "tick_second.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> TickSecond::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<TickSecond>(new TickSecond {});
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> TickSecond::clone() const {
    return std::shared_ptr<TickSecond>(new TickSecond {});
}

void TickSecond::variableUpdate(uint32_t variableStepMillis) {
    mTimeSinceLastTick += variableStepMillis;
    while(mTimeSinceLastTick >= 1000) {
        mSigSecondPassed.emit();
        mTimeSinceLastTick -= 1000;
    }
}
