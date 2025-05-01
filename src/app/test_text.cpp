
#include <nlohmann/json.hpp>

#include "../engine/core/resource_database.hpp"
#include "../engine/shapegen.hpp"

#include "test_text.hpp"

std::shared_ptr<BaseSimObjectAspect> TestText::create(const nlohmann::json& jsonAspectProperties) {
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

    std::shared_ptr<TextFont> font { ResourceDatabase::GetRegisteredResource<TextFont>(fontResourceName) };
    std::shared_ptr<TestText> testTextAspect { std::make_shared<TestText>() };
    testTextAspect->mFont = font;
    testTextAspect->mText = text;
    testTextAspect->mScale = scale;

    return testTextAspect;
}

std::shared_ptr<BaseSimObjectAspect> TestText::clone() const {
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
    mFont = ResourceDatabase::GetRegisteredResource<TextFont>(fontResourceName);
    recomputeTexture();
}

void TestText::recomputeTexture() {
    std::shared_ptr<Texture> textTexture { mFont->renderText(mText, glm::u8vec3 {0x0}, glm::u8vec3 {0xFF}) };
    const nlohmann::json rectangleParameters = { 
        {"type", StaticModel::getResourceTypeName()},
        {"method", StaticModelRectangleDimensions::getResourceConstructorName()},
        {"parameters", {
            {"width", textTexture->getWidth() * mScale },
            {"height", textTexture->getHeight() * mScale },
            {"flip_texture_y", true}
        }}
    };

    if(!hasComponent<std::shared_ptr<StaticModel>>()) {
        addComponent<std::shared_ptr<StaticModel>>(
            ResourceDatabase::ConstructAnonymousResource<StaticModel>(rectangleParameters)
        );
    } else {
        updateComponent<std::shared_ptr<StaticModel>>(
            ResourceDatabase::ConstructAnonymousResource<StaticModel>(rectangleParameters)
        );
    }

    std::shared_ptr<Material> material { getComponent<std::shared_ptr<StaticModel>>()->getMaterialHandles()[0] };
    material->updateTextureProperty(
        "textureAlbedo",
        textTexture
    );
    material->updateIntProperty("usesTextureAlbedo", true);
}
