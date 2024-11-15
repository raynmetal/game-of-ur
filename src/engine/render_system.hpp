#ifndef ZORENDERSYSTEM_H
#define ZORENDERSYSTEM_H

#include <vector>
#include <map>
#include <string>
#include <array>

#include "input_system/input_system.hpp"
#include "simple_ecs.hpp"
#include "shapegen.hpp"
#include "material.hpp"
#include "render_stage.hpp"

class RenderSystem: public System<RenderSystem>,  public IActionHandler {
public:
    class LightQueue: public System<LightQueue>{
    public:
        LightQueue();
        void enqueueTo(BaseRenderStage& renderStage, float simulationProgress);
    private:
        std::shared_ptr<StaticMesh> mSphereMesh { nullptr };
        std::shared_ptr<Material> mLightMaterial{ std::make_shared<Material>() };
    };
    class OpaqueQueue: public System<OpaqueQueue> {
    public:
        void enqueueTo(BaseRenderStage& renderStage, float simulationProgress);
    };

    void execute(float simulationProgress);

    void updateCameraMatrices(float simulationProgress);

    void handleAction(const ActionData& actionData, const ActionDefinition& actionDefinition) override;

    /* Set the texture to be rendered to the screen by its relative index */
    void renderNextTexture ();
    void setGamma(float gamma);
    void setExposure(float exposure);
    std::size_t getCurrentScreenTexture ();

private:
    void onCreated() override;

    std::size_t mCurrentScreenTexture {0};
    std::vector<std::shared_ptr<Texture>> mScreenTextures {};
    std::shared_ptr<Material> mLightMaterialHandle {nullptr};
    EntityID mActiveCamera {};
    GLuint mMatrixUniformBufferIndex {0};
    GLuint mMatrixUniformBufferBinding {0};

    GeometryRenderStage mGeometryRenderStage { "src/shader/geometryShader.json" };
    LightingRenderStage mLightingRenderStage { "src/shader/lightingShader.json" };
    BlurRenderStage mBlurRenderStage { "src/shader/gaussianblurShader.json" };
    TonemappingRenderStage mTonemappingRenderStage { "src/shader/tonemappingShader.json" };
    ScreenRenderStage mScreenRenderStage { "src/shader/screenShader.json" };

    float mGamma { 2.f };
    float mExposure { 1.f };

    float mGammaStep { .1f };
    float mExposureStep { .1f };
};

#endif
