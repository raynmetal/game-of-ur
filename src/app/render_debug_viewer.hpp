#ifndef ZOAPPRENDERDEBUGVIEWER_H
#define ZOAPPRENDERDEBUGVIEWER_H

#include "../engine/sim_system.hpp"

class RenderDebugViewer: public SimObjectAspect<RenderDebugViewer> {
public:
    inline static std::string getSimObjectAspectTypeName() { return "RenderDebugViewer"; }
    std::shared_ptr<BaseSimObjectAspect> makeCopy() const override;
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);

protected:
    void onUpdateGamma(const ActionData& actionData, const ActionDefinition& actionDefinition);
    void onUpdateExposure(const ActionData& actionData, const ActionDefinition& actionDefinition);
    void onRenderNextTexture(const ActionData& actionData, const ActionDefinition& actionDefinition);

    std::weak_ptr<FixedActionBinding> handleUpdateGamma{ declareFixedActionBinding (
        "Graphics", "UpdateGamma", [this](const ActionData& actionData, const ActionDefinition& actionDefinition) {
            this->onUpdateGamma(actionData, actionDefinition);
        }
    )};
    std::weak_ptr<FixedActionBinding> handleUpdateExposure { declareFixedActionBinding (
        "Graphics", "UpdateExposure", [this](const ActionData& actionData, const ActionDefinition& actionDefinition) {
            this->onUpdateExposure(actionData, actionDefinition);
        }
    )};
    std::weak_ptr<FixedActionBinding> handleRenderNextTexture { declareFixedActionBinding(
        "Graphics", "RenderNextTexture", [this](const ActionData& actionData, const ActionDefinition& actionDefinition) {
            this->onRenderNextTexture(actionData, actionDefinition);
        }
    )};

private:
    RenderDebugViewer() : SimObjectAspect<RenderDebugViewer>{0} {}
    float mGammaStep { .1f };
    float mExposureStep { .1f };
};

#endif
