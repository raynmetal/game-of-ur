
#include <nlohmann/json.hpp>

#include "../engine/core/resource_database.hpp"
#include "../engine/shapegen.hpp"

#include "test_text.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> TestText::create(const nlohmann::json& jsonAspectProperties) {
    const std::string text { 
        jsonAspectProperties.find("text") != jsonAspectProperties.end()?
            jsonAspectProperties.at("text").get<std::string>():
            "Default Text"
    };
    const std::string fontResourceName {
        jsonAspectProperties.find("font_resource_name") != jsonAspectProperties.end()?
            jsonAspectProperties.at("font_resource_name").get<std::string>():
            "DefaultFont"
    };
    const float scale {
        jsonAspectProperties.find("scale") != jsonAspectProperties.end()?
            jsonAspectProperties.at("scale").get<float>():
            .01f
    };
    const glm::vec2 anchor {
        jsonAspectProperties.find("anchor") != jsonAspectProperties.end()?
        glm::vec2{
            jsonAspectProperties.at("anchor")[0].get<float>(),
            jsonAspectProperties.at("anchor")[1].get<float>()
        }:
        glm::vec2{.5f, .5f}
    };

    std::shared_ptr<ToyMakersEngine::TextFont> font { ToyMakersEngine::ResourceDatabase::GetRegisteredResource<ToyMakersEngine::TextFont>(fontResourceName) };
    std::shared_ptr<TestText> testTextAspect { std::make_shared<TestText>() };
    testTextAspect->mFont = font;
    testTextAspect->mText = text;
    testTextAspect->mScale = scale;
    testTextAspect->mAnchor;

    return testTextAspect;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> TestText::clone() const {
    std::shared_ptr<TestText> testTextAspect { std::make_shared<TestText>() };
    testTextAspect->mFont = mFont;
    testTextAspect->mText = mText;
    testTextAspect->mScale = mScale;

    return testTextAspect;
}

void TestText::onActivated() {
    recomputeTexture();
}

void TestText::updateScale(float scale) {
    mScale = scale;
    recomputeTexture();
}

void TestText::updateText(const std::string& text) {
    mText = text;
    recomputeTexture();
}

void TestText::updateFont(const std::string& fontResourceName) {
    mFont = ToyMakersEngine::ResourceDatabase::GetRegisteredResource<ToyMakersEngine::TextFont>(fontResourceName);
    recomputeTexture();
}

void TestText::recomputeTexture() {
    std::shared_ptr<ToyMakersEngine::Texture> textTexture { mFont->renderText(mText, glm::u8vec3 {0x0}, glm::u8vec3 {0xFF}) };
    const glm::vec2 textDimensions { textTexture->getWidth() * mScale, textTexture->getHeight() * mScale };
    const nlohmann::json rectangleParameters = { 
        {"type", ToyMakersEngine::StaticModel::getResourceTypeName()},
        {"method", ToyMakersEngine::StaticModelRectangleDimensions::getResourceConstructorName()},
        {"parameters", {
            {"width", textDimensions.x }, {"height", textDimensions.y },
            {"flip_texture_y", true},
            {"material_properties", nlohmann::json::array()},
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

    std::shared_ptr<ToyMakersEngine::StaticModel>  rectangle { getComponent<std::shared_ptr<ToyMakersEngine::StaticModel>>() };
    for(auto mesh: rectangle->getMeshHandles()) {
        for(auto iVertex{ mesh->getVertexListBegin() }, end {mesh->getVertexListEnd()}; iVertex != end; ++iVertex) {
            iVertex->mPosition += glm::vec4{
                textDimensions.x * (.5f - mAnchor.x),
                textDimensions.y * (mAnchor.y - .5f),
                0.f,
                0.f,
            };
        }
    }
    updateComponent<std::shared_ptr<ToyMakersEngine::StaticModel>>(rectangle);

    std::shared_ptr<ToyMakersEngine::Material> material { getComponent<std::shared_ptr<ToyMakersEngine::StaticModel>>()->getMaterialHandles()[0] };
    material->updateTextureProperty(
        "textureAlbedo",
        textTexture
    );
    material->updateIntProperty("usesTextureAlbedo", true);
}
