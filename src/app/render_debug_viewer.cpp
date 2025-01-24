#include <glm/gtx/string_cast.hpp>

#include "../engine/window_context_manager.hpp"
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

void RenderDebugViewer::printWindowProps() {
    WindowContext& windowCtxManager { WindowContext::getInstance() };
    std::cout << "Window State:\n"
        << "\tdisplay index: " << windowCtxManager.getDisplayID() << "\n"
        << "\ttitle: " << windowCtxManager.getTitle() << "\n"
        << "\tmaximized: " << windowCtxManager.isMaximized() << "\n"
        << "\tminimized: " << windowCtxManager.isMinimized() << "\n"
        << "\tresizable: " << windowCtxManager.isResizable() << "\n"
        << "\thidden: " << windowCtxManager.isHidden() << "\n"
        << "\tshown: " << windowCtxManager.isShown() << "\n"
        << "\tmouse focus: " << windowCtxManager.hasMouseFocus() << "\n"
        << "\tmouse capture: " << windowCtxManager.hasCapturedMouse() << "\n"
        << "\tkey focus: " << windowCtxManager.hasKeyFocus() << "\n"
        << "\tfullscreen: " << windowCtxManager.isFullscreen() << "\n"
        << "\tborderless: " << windowCtxManager.isBorderless() << "\n"
        << "\texclusive fullscreen: " << windowCtxManager.isExclusiveFullscreen() << "\n"
        << "\twindow position: " << glm::to_string(windowCtxManager.getPosition()) << "\n"
        << "\twindow dimensions: " << glm::to_string(windowCtxManager.getDimensions()) << "\n"
        << "\tmaximum window dimensions: " << glm::to_string(windowCtxManager.getDimensionsMaximum()) << "\n"
        << "\tminimum window dimensions: " << glm::to_string(windowCtxManager.getDimensionsMinimum()) << "\n"
    ;
}
