#include "nine_slice_panel.hpp"
#include "ui_panel.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UIPanel::create(const nlohmann::json& jsonAspectProperties) {
    const std::string panelResourceName {
        jsonAspectProperties.at("panel_resource_name").get<std::string>()
    };
    const glm::vec2 anchor {
        jsonAspectProperties.find("anchor") != jsonAspectProperties.end()?
        glm::vec2{
            jsonAspectProperties.at("anchor")[0].get<float>(),
            jsonAspectProperties.at("anchor")[1].get<float>()
        }:
        glm::vec2{.5f, .5f}
    };
    const glm::vec2 contentSize {
        jsonAspectProperties.at("content_size")[0].get<float>(),
        jsonAspectProperties.at("content_size")[1].get<float>(),
    };

    std::shared_ptr<NineSlicePanel> panel { ToyMakersEngine::ResourceDatabase::GetRegisteredResource<NineSlicePanel>(panelResourceName) };
    std::shared_ptr<UIPanel> panelAspect { std::make_shared<UIPanel>() };

    panelAspect->mBasePanel = panel;
    panelAspect->mContentSize = contentSize;
    panelAspect->mAnchor = anchor;

    return panelAspect;
}

void UIPanel::updateContentSize(glm::vec2 contentSize) {
    if(contentSize == mContentSize) return;
    mContentSize = contentSize;
    recomputeTexture();
}
void UIPanel::updateAnchor(glm::vec2 anchor) {
    if(anchor == mAnchor) return;
    mAnchor = anchor;
    recomputeTexture();
}
void UIPanel::updateBasePanel(std::shared_ptr<NineSlicePanel> newPanel) {
    if(newPanel == mBasePanel) return;
    mBasePanel = newPanel;
    recomputeTexture();
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UIPanel::clone() const {
    std::shared_ptr<UIPanel> panelAspect { std::make_shared<UIPanel>() };

    panelAspect->mBasePanel = mBasePanel;
    panelAspect->mContentSize = mContentSize;
    panelAspect->mAnchor = mAnchor;

    return panelAspect;
}

void UIPanel::onActivated() {
    recomputeTexture();
}

void UIPanel::recomputeTexture() {
    std::shared_ptr<ToyMakersEngine::Texture> panelTexture { mBasePanel->generateTexture(mContentSize) };
    const glm::vec2 panelDimensions { panelTexture->getWidth(), panelTexture->getHeight() };
    const nlohmann::json rectangleParameters = {
        {"type", ToyMakersEngine::StaticModel::getResourceTypeName()},
        {"method", ToyMakersEngine::StaticModelRectangleDimensions::getResourceConstructorName()},
        {"parameters", {
            {"width", panelDimensions.x}, {"height", panelDimensions.y},
            {"flip_texture_y", true},
            {"material_properties", nlohmann::json::array()}
        }}
    };

    if(!hasComponent<std::shared_ptr<ToyMakersEngine::StaticModel>>()) {
        addComponent<std::shared_ptr<ToyMakersEngine::StaticModel>>(
            ToyMakersEngine::ResourceDatabase::ConstructAnonymousResource<ToyMakersEngine::StaticModel>(rectangleParameters)
        );
    } else {
        updateComponent<std::shared_ptr<ToyMakersEngine::StaticModel>>(
            ToyMakersEngine::ResourceDatabase::ConstructAnonymousResource<ToyMakersEngine::StaticModel>(rectangleParameters)
        );
    }
    std::shared_ptr<ToyMakersEngine::StaticModel> rectangle { getComponent<std::shared_ptr<ToyMakersEngine::StaticModel>>() };
    for(auto mesh: rectangle->getMeshHandles()) {
        for(auto iVertex{ mesh->getVertexListBegin() }, end {mesh->getVertexListEnd()}; iVertex != end; ++iVertex) {
            iVertex->mPosition += glm::vec4{
                panelDimensions.x * (.5f - mAnchor.x),
                panelDimensions.y * (mAnchor.y - .5f),
                0.f,
                0.f,
            };
        }
    }
    updateComponent<std::shared_ptr<ToyMakersEngine::StaticModel>>(rectangle);
    std::shared_ptr<ToyMakersEngine::Material> material { getComponent<std::shared_ptr<ToyMakersEngine::StaticModel>>()->getMaterialHandles()[0] };
    material->updateTextureProperty(
        "textureAlbedo",
        panelTexture
    );
    material->updateIntProperty("usesTextureAlbedo", true);
}
