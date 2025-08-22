#ifndef ZOAPPUIIMAGE_H
#define ZOAPPUIIMAGE_H

#include "../engine/sim_system.hpp"
#include "../engine/texture.hpp"

class UIImage: public ToyMakersEngine::SimObjectAspect<UIImage> {
public:
    UIImage(): SimObjectAspect<UIImage>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UIImage"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;

    void updateImage(const std::string& imageFilepath);
    void updateDimensions(const glm::uvec2& dimensions);
    void updateAnchor(const glm::vec2& anchor);

private:
    void recomputeTexture();

    std::string mImageFilepath {};
    glm::vec2 mAnchor {0.f, 0.f};
    glm::uvec2 mDimensions { 0.f, 0.f };
};

#endif
