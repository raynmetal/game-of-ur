#ifndef ZOAPPNINESLICE_H
#define ZOAPPNINESLICE_H

#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>

#include "../engine/shader_program.hpp"
#include "../engine/core/resource_database.hpp"
#include "../engine/texture.hpp"

class NineSlicePanel: public ToyMakersEngine::Resource<NineSlicePanel> {
public:
    enum ScaleMode: uint8_t {
        STRETCH,
        TILE,
    };

    NineSlicePanel(
        std::shared_ptr<ToyMakersEngine::Texture> baseTexture,
        SDL_FRect contentRegionUV
    );
    ~NineSlicePanel();
    inline static std::string getResourceTypeName() { return "NineSlicePanel"; }

    std::shared_ptr<ToyMakersEngine::Texture> generateTexture(glm::uvec2 contentDimensions) const;

    uint32_t getOffsetPixelLeft() const;
    uint32_t getOffsetPixelRight() const;
    uint32_t getOffsetPixelBottom() const;
    uint32_t getOffsetPixelTop() const;

private:
    std::shared_ptr<ToyMakersEngine::Texture> mTexture {};

    SDL_FRect mContentRegion { .x{0.f}, .y{0.f}, .w{1.f}, .h{1.f} };

    std::shared_ptr<ToyMakersEngine::ShaderProgram> mShaderHandle { nullptr };
    GLuint mVertexArrayObject {};
};

class NineSlicePanelFromDescription: public ToyMakersEngine::ResourceConstructor<NineSlicePanel, NineSlicePanelFromDescription> {
public:
    NineSlicePanelFromDescription(): ToyMakersEngine::ResourceConstructor<NineSlicePanel, NineSlicePanelFromDescription> {0} {}
    inline static std::string getResourceConstructorName() { return "fromDescription"; }
private:
    std::shared_ptr<ToyMakersEngine::IResource> createResource(const nlohmann::json& methodParameters) override;
};

NLOHMANN_JSON_SERIALIZE_ENUM(NineSlicePanel::ScaleMode, {
    {NineSlicePanel::ScaleMode::STRETCH, "stretch"},
    {NineSlicePanel::ScaleMode::TILE, "tile"},
});

#endif
