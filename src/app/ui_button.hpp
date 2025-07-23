#ifndef ZOAPPUIBUTTON_H
#define ZOAPPUIBUTTON_H

#include <nlohmann/json.hpp>

#include "../engine/sim_system.hpp"
#include "../engine/signals.hpp"

#include "interface_pointer_callback.hpp"
#include "nine_slice_panel.hpp"

class UIButton: public ToyMakersEngine::SimObjectAspect<UIButton>, public IHoverable, public ILeftClickable {
public:
    enum State: uint8_t {
        ACTIVE,
        HOVER,
        PRESSED,
        INACTIVE,

        //=============
        TOTAL,
    };
    UIButton(): SimObjectAspect<UIButton>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UIButton"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;

    void enableButton();
    void disableButton();

    void updateText(const std::string& newText);
    void updateTextScale(float scale);
    void updateTextFont(const std::string& textResourceName);
    void updateTextColor(glm::u8vec4 textColor);

    void updateButtonAnchor(glm::vec2 newAnchor);
    void updateHighlightColor(glm::vec4 newColor);

private:
    State mCurrentState { State::ACTIVE };
    bool mHovered { false };
    std::array<std::shared_ptr<NineSlicePanel>, State::TOTAL> mStatePanels {};
    glm::vec2 mAnchor {.5f, .5f};
    std::string mValue {""};

    std::string mTextOverride {""};
    float mTextScaleOverride {1.f};
    std::string mTextFontOverride {""};
    glm::u8vec4 mTextColorOverride { 0x00, 0x00, 0x00, 0xFF };

    std::shared_ptr<NineSlicePanel> mHighlightPanel {};
    glm::vec4 mHighlightColor {0.f, 0.f, 0.f, 0.f};

    void recomputeTexture();
    void updateButtonState(UIButton::State newState);
    void fireStateEvent();

    std::shared_ptr<ToyMakersEngine::SimObject> getTextObject();
public:
    ToyMakersEngine::Signal<std::string> mSigButtonPressed { *this, "ButtonPressed" };
    ToyMakersEngine::Signal<std::string> mSigButtonReleased { *this, "ButtonReleased" };
    ToyMakersEngine::Signal<std::string> mSigButtonHoveredOver { *this, "ButtonHoveredOver" };
    ToyMakersEngine::Signal<> mSigButtonActivated { *this, "ButtonActivated" };
    ToyMakersEngine::Signal<> mSigButtonDeactivated { *this, "ButtonDeactivated" };

private:
    bool onPointerEnter(glm::vec4 pointerLocation) override;
    bool onPointerLeave() override;
    bool onPointerLeftClick(glm::vec4 pointerLocation) override;
    bool onPointerLeftRelease(glm::vec4 pointerLocation) override;
};


NLOHMANN_JSON_SERIALIZE_ENUM(UIButton::State, {
    {UIButton::State::ACTIVE, "active"},
    {UIButton::State::HOVER, "hover"},
    {UIButton::State::PRESSED, "pressed"},
    {UIButton::State::INACTIVE, "inactive"},
});

#endif
