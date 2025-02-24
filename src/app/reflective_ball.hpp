#ifndef ZOREFLECTIVEBALL_H
#define ZOREFLECTIVEBALL_H

#include "../engine/sim_system.hpp"

class ReflectiveBall: public SimObjectAspect<ReflectiveBall> {
public:
    inline static std::string getSimObjectAspectTypeName() { return "ReflectiveBall"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;
    void onDeactivated() override;
    void variableUpdate(uint32_t variableStepMillis) override;

private:
    ReflectiveBall(): SimObjectAspect<ReflectiveBall>{0} {}

    std::weak_ptr<ViewportNode> mReflectionViewport {};
    std::weak_ptr<Material> mBallMaterial {};
};

#endif
