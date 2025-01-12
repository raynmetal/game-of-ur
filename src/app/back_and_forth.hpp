#ifndef ZOBACKANDFORTH_H
#define ZOBACKANDFORTH_H

#include "../engine/sim_system.hpp"
#include "../engine/signals.hpp"

class BackAndForth: public SimObjectAspect<BackAndForth> {
public:
    inline static std::string getSimObjectAspectTypeName() { return "BackAndForth"; }
    static std::unique_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);

    void update(uint32_t deltaSimtimeMillis) override;

    std::unique_ptr<BaseSimObjectAspect> makeCopy() const override;

    Signal<float> mSigDirectionChanged {*this, "directionChanged"};

private:
    void onAttached() override;
    void onActivated() override;
    void onDeactivated() override;
    void onDetached() override;
    BackAndForth() : SimObjectAspect<BackAndForth>{0} {}
    float mDelta { 0.f };
    uint32_t mElapsedTime { 0 };
};

#endif
