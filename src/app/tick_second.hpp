#ifndef ZOTICKSECOND_H
#define ZOTICKSECOND_H

#include "../engine/sim_system.hpp"

class TickSecond: public ToyMakersEngine::SimObjectAspect<TickSecond> {
public:
    inline static std::string getSimObjectAspectTypeName() { return "TickSecond"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    void variableUpdate(uint32_t variableStepMillis) override;
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    ToyMakersEngine::Signal<> mSigSecondPassed { *this, "secondPassed" };
private:
    TickSecond(): SimObjectAspect<TickSecond>{ 0 } {}
    uint32_t mTimeSinceLastTick { 0 };
};

#endif
