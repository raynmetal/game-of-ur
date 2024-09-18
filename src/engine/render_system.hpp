#ifndef ZORENDERSYSTEM_H
#define ZORENDERSYSTEM_H

#include <vector>
#include <map>
#include <string>
#include <array>

#include "input_system/input_system.hpp"
#include "simple_ecs.hpp"
#include "shapegen.hpp"
#include "render_stage.hpp"
#include "fly_camera.hpp"

class RenderSystem: public System,  public IActionHandler {
public:
    class LightQueue: public System {
    public:
        void enqueueTo(BaseRenderStage& renderStage, float simulationProgress);
    private:
        MeshHandle mSphereMesh { generateSphereMesh(10, 5) };
        MaterialHandle mLightMaterial{};
    };
    class OpaqueQueue: public System {
    public:
        void enqueueTo(BaseRenderStage& renderStage, float simulationProgress);
    };

    RenderSystem();
    void execute(float simulationProgress);

    void updateCameraMatrices(const FlyCamera& camera);

    void handleAction(const ActionData& actionData, const ActionDefinition& actionDefinition) override;

    /* Set the texture to be rendered to the screen by its relative index */
    void renderNextTexture ();
    void setGamma(float gamma);
    void setExposure(float exposure);
    std::size_t getCurrentScreenTexture ();

private:

    std::size_t mCurrentScreenTexture {0};
    std::vector<TextureHandle> mScreenTextures {};
    GLuint mMatrixUniformBufferIndex {0};
    GLuint mMatrixUniformBufferBinding {0};

    GeometryRenderStage mGeometryRenderStage;
    LightingRenderStage mLightingRenderStage;
    BlurRenderStage mBlurRenderStage;
    TonemappingRenderStage mTonemappingRenderStage;
    ScreenRenderStage mScreenRenderStage;

    float mGamma { 2.f };
    float mExposure { 1.f };

    float mGammaStep { .1f };
    float mExposureStep { .1f };
};

#endif
