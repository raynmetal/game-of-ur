#include "try_cubemap.hpp"

std::shared_ptr<BaseSimObjectAspect> TryCubemap::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<TryCubemap>{ new TryCubemap{} };
}
std::shared_ptr<BaseSimObjectAspect> TryCubemap::clone() const {
    return std::shared_ptr<TryCubemap>{ new TryCubemap{} };
}

void TryCubemap::onActivated() {
    mMaterial = getComponent<std::shared_ptr<StaticModel>>()->getMaterialHandles()[0];
    mMaterial.lock()->updateTextureProperty(
        "textureAlbedo",
        ResourceDatabase::GetRegisteredResource<Texture>("Skybox_Texture")
    );
    mMaterial.lock()->updateIntProperty("usesTextureAlbedo", true);
}
