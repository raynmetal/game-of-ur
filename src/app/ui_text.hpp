#ifndef ZOAPPUITEXT_H
#define ZOAPPUITEXT_H

#include "../engine/sim_system.hpp"
#include "../engine/text_render.hpp"

class UIText: public ToyMakersEngine::SimObjectAspect<UIText> {
public:
    UIText(): SimObjectAspect<UIText>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UIText"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;

    void updateText(const std::string& newText);
    void updateColor(glm::u8vec4 newColour);
    void updateScale(float scale);
    void updateFont(const std::string& textResourceName);
    void updateAnchor(glm::vec2 anchor);

private:
    void recomputeTexture();

    glm::u8vec4 mColor {0x00, 0x00, 0x00, 0xFF};
    std::shared_ptr<ToyMakersEngine::TextFont> mFont {};
    std::string mText {};
    float mScale { 1e-2 };

    glm::vec2 mAnchor {0.f, 0.f};
};

#endif
