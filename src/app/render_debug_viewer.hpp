#ifndef ZOAPPRENDERDEBUGVIEWER_H
#define ZOAPPRENDERDEBUGVIEWER_H

#include "../engine/sim_system.hpp"

class RenderDebugViewer: public SimObjectAspect<RenderDebugViewer> {
public:
    inline static std::string getSimObjectAspectTypeName() { return "RenderDebugViewer"; }
    void handleAction(const ActionData& actionData, const ActionDefinition& actionDefinition) override;
    std::shared_ptr<BaseSimObjectAspect> makeCopy() const override;
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);

private:
    RenderDebugViewer(): SimObjectAspect<RenderDebugViewer>{0} {}
    float mGammaStep { .1f };
    float mExposureStep { .1f };
};

#endif
