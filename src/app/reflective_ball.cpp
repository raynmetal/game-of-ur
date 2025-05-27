#include "reflective_ball.hpp"


std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> ReflectiveBall::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<ReflectiveBall>{ new ReflectiveBall{} };
}
std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> ReflectiveBall::clone() const {
    return std::shared_ptr<ReflectiveBall> { new ReflectiveBall{} };
}

void ReflectiveBall::onActivated() {
    mReflectionViewport = std::static_pointer_cast<ToyMakersEngine::ViewportNode>(
        getSimObject().getNode("/viewport/")
    );
    mBallMaterial = getComponent<std::shared_ptr<ToyMakersEngine::StaticModel>>()->getMaterialHandles()[0];
}

void ReflectiveBall::variableUpdate(uint32_t variableStepMillis) {
    std::shared_ptr<ToyMakersEngine::Material> ballMaterial { mBallMaterial.lock() };
    if(std::shared_ptr<ToyMakersEngine::Texture> viewportRenderResult = mReflectionViewport.lock()->fetchRenderResult(1.f)){
        ballMaterial->updateTextureProperty(
            "textureAlbedo",
            viewportRenderResult
        );
        ballMaterial->updateIntProperty("usesTextureAlbedo", true);
    }
}

void ReflectiveBall::onDeactivated() {
    mBallMaterial.reset();
    mReflectionViewport.reset();
}
