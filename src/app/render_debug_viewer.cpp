#include "../engine/application.hpp"
#include "../engine/render_system.hpp"

#include "render_debug_viewer.hpp"

void RenderDebugViewer::handleAction(const ActionData& actionData, const ActionDefinition& actionDefinition) {
    std::shared_ptr<RenderSystem> renderSystem {
        Application::getInstance().getObject<std::shared_ptr<RenderSystem>>()
    };
    if(actionDefinition.mName == "UpdateGamma") {
        renderSystem->setGamma(
            renderSystem->getGamma() 
            + actionData.mOneAxisActionData.mValue*mGammaStep
        );
    } else if(actionDefinition.mName == "UpdateExposure") {
        renderSystem->setExposure(
            renderSystem->getExposure()
            + actionData.mOneAxisActionData.mValue*mExposureStep
        );
    } else if (actionDefinition.mName == "RenderNextTexture") {
        renderSystem->renderNextTexture();
    }
}

std::shared_ptr<BaseSimObjectAspect> RenderDebugViewer::makeCopy() const {
    return std::shared_ptr<RenderDebugViewer>(new RenderDebugViewer {});
}

std::shared_ptr<BaseSimObjectAspect> RenderDebugViewer::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<RenderDebugViewer>(new RenderDebugViewer {});
}
