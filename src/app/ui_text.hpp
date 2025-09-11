#ifndef ZOAPPUITEXT_H
#define ZOAPPUITEXT_H

#include "toymaker/sim_system.hpp"
#include "toymaker/text_render.hpp"

class UIText: public ToyMaker::SimObjectAspect<UIText> {
public:
    UIText(): SimObjectAspect<UIText>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UIText"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;

    inline std::string getText() const { return mText; }

    void updateText(const std::string& newText);
    void updateColor(glm::u8vec4 newColour);
    void updateScale(float scale);
    void updateFont(const std::string& textResourceName);
    void updateAnchor(glm::vec2 anchor);

private:
    void recomputeTexture();

    glm::u8vec4 mColor {0x00, 0x00, 0x00, 0xFF};
    std::shared_ptr<ToyMaker::TextFont> mFont {};
    std::string mText {};
    float mScale { 1e-2 };
    uint32_t mMaxWidthPixels { 0 };

    glm::vec2 mAnchor {0.f, 0.f};
};

#endif
