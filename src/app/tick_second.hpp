#ifndef ZOTICKSECOND_H
#define ZOTICKSECOND_H

#include "../engine/sim_system.hpp"

class TickSecond: public SimObjectAspect<TickSecond> {
public:
    inline static std::string getSimObjectAspectTypeName() { return "TickSecond"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    void update(uint32_t deltaSimtimeMillis) override;
    std::shared_ptr<BaseSimObjectAspect> makeCopy() const override;

    Signal<> mSigSecondPassed { *this, "secondPassed" };
private:
    TickSecond(): SimObjectAspect<TickSecond>{ 0 } {}
    uint32_t mTimeSinceLastTick { 0 };
};

#endif
