#ifndef ZOAPPTESTTEXT_H
#define ZOAPPTESTTEXT_H

#include "../engine/sim_system.hpp"
#include "../engine/text_render.hpp"

class TestText: public ToyMakersEngine::SimObjectAspect<TestText> {
public:
    TestText(): SimObjectAspect<TestText>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "TestText"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;

    void updateText(const std::string& newText);
    void updateScale(float scale);
    void updateFont(const std::string& textResourceName);

private:
    void recomputeTexture();

    std::shared_ptr<ToyMakersEngine::TextFont> mFont {};
    std::string mText {};
    float mScale { 1e-2 };

    glm::vec2 mAnchor {0.f, 0.f};
};

#endif
