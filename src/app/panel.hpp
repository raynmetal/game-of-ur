#ifndef ZOAPPPANEL_H
#define ZOAPPPANEL_H

#include "../engine/sim_system.hpp"

#include "nine_slice_panel.hpp"

class Panel: public ToyMakersEngine::SimObjectAspect<Panel> {
public:
    Panel(): SimObjectAspect<Panel>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "Panel"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;

    void updateContentSize(glm::vec2 contentSize);
    void updateAnchor(glm::vec2 anchor);
    void updateBasePanel(std::shared_ptr<NineSlicePanel> newPanel);

private:
    void recomputeTexture();

    std::shared_ptr<NineSlicePanel> mBasePanel {};
    glm::vec2 mContentSize {0.f, 0.f};
    glm::vec2 mAnchor {0.f, 0.f};
};

#endif
