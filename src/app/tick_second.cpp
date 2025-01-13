#include "tick_second.hpp"

std::unique_ptr<BaseSimObjectAspect> TickSecond::create(const nlohmann::json& jsonAspectProperties) {
    return std::unique_ptr<TickSecond>(new TickSecond {});
}

std::unique_ptr<BaseSimObjectAspect> TickSecond::makeCopy() const {
    return std::unique_ptr<TickSecond>(new TickSecond {});
}

void TickSecond::update(uint32_t deltaSimTimeMillis) {
    mTimeSinceLastTick += deltaSimTimeMillis;
    while(mTimeSinceLastTick >= 1000) {
        mSigSecondPassed.emit();
        mTimeSinceLastTick -= 1000;
    }
}
