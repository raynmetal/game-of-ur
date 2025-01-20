#include "tick_second.hpp"

std::shared_ptr<BaseSimObjectAspect> TickSecond::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<TickSecond>(new TickSecond {});
}

std::shared_ptr<BaseSimObjectAspect> TickSecond::makeCopy() const {
    return std::shared_ptr<TickSecond>(new TickSecond {});
}

void TickSecond::update(uint32_t deltaSimTimeMillis) {
    mTimeSinceLastTick += deltaSimTimeMillis;
    while(mTimeSinceLastTick >= 1000) {
        mSigSecondPassed.emit();
        mTimeSinceLastTick -= 1000;
    }
}
