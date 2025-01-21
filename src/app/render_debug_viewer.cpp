#include "../engine/application.hpp"
#include "../engine/render_system.hpp"

#include "render_debug_viewer.hpp"


void RenderDebugViewer::onUpdateGamma(const ActionData& actionData, const ActionDefinition& actionDefinition) {
    std::shared_ptr<RenderSystem> renderSystem {
        Application::getInstance().getObject<std::shared_ptr<RenderSystem>>()
    };
    renderSystem->setGamma(
        renderSystem->getGamma() 
        + actionData.mOneAxisActionData.mValue*mGammaStep
    );
}
void RenderDebugViewer::onUpdateExposure(const ActionData& actionData, const ActionDefinition& actionDefinition) {
    std::shared_ptr<RenderSystem> renderSystem {
        Application::getInstance().getObject<std::shared_ptr<RenderSystem>>()
    };
    renderSystem->setExposure(
        renderSystem->getExposure()
        + actionData.mOneAxisActionData.mValue*mExposureStep
    );
}
void RenderDebugViewer::onRenderNextTexture(const ActionData& actionData, const ActionDefinition& actionDefinition) {
    std::shared_ptr<RenderSystem> renderSystem {
        Application::getInstance().getObject<std::shared_ptr<RenderSystem>>()
    };
    renderSystem->renderNextTexture();
}

std::shared_ptr<BaseSimObjectAspect> RenderDebugViewer::makeCopy() const {
    return std::shared_ptr<RenderDebugViewer>(new RenderDebugViewer {});
}

std::shared_ptr<BaseSimObjectAspect> RenderDebugViewer::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<RenderDebugViewer>(new RenderDebugViewer {});
}
