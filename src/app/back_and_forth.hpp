#ifndef ZOBACKANDFORTH_H
#define ZOBACKANDFORTH_H

#include "../engine/sim_system.hpp"

class BackAndForth: public SimObjectAspect<BackAndForth> {
public:
    inline static std::string getSimObjectAspectTypeName() { return "BackAndForth"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);

    void variableUpdate(uint32_t variableStepMillis) override;

    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    Signal<float> mSigDirectionChanged {*this, "directionChanged"};

private:
    BackAndForth() : SimObjectAspect<BackAndForth>{0} {}
    float mDelta { 0.f };
    uint32_t mElapsedTime { 0 };
};

#endif
