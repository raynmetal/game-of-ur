#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "../engine/window_context_manager.hpp"
#include "../engine/render_system.hpp"

#include "render_debug_viewer.hpp"


bool RenderDebugViewer::onUpdateGamma(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    getLocalViewport().updateGamma(
        getLocalViewport().getGamma() 
        + actionData.mOneAxisActionData.mValue*mGammaStep
    );
    return true;
}
bool RenderDebugViewer::onUpdateExposure(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    getLocalViewport().updateExposure(
        getLocalViewport().getExposure()
        + actionData.mOneAxisActionData.mValue*mExposureStep
    );
    return true;
}
bool RenderDebugViewer::onRenderNextTexture(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    getLocalViewport().viewNextDebugTexture();
    return true;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> RenderDebugViewer::clone() const {
    return std::shared_ptr<RenderDebugViewer>(new RenderDebugViewer {});
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> RenderDebugViewer::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<RenderDebugViewer>(new RenderDebugViewer {});
}

void RenderDebugViewer::printWindowProps() {
    ToyMakersEngine::WindowContext& windowCtxManager { ToyMakersEngine::WindowContext::getInstance() };
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
