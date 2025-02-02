#ifndef ZORENDERSYSTEM_H
#define ZORENDERSYSTEM_H

#include <vector>
#include <map>
#include <string>
#include <array>

#include "input_system/input_system.hpp"
#include "ecs_world.hpp"
#include "shapegen.hpp"
#include "material.hpp"
#include "render_stage.hpp"
#include "camera_system.hpp"

class RenderSystem: public System<RenderSystem, CameraProperties> {
public:
    RenderSystem(ECSWorld& world):
    System<RenderSystem, CameraProperties>{world}
    {}

    static std::string getSystemTypeName() { return "RenderSystem"; }

    class LightQueue: public System<LightQueue, Transform, LightEmissionData>{
    public:
        LightQueue(ECSWorld& world):
        System<RenderSystem::LightQueue, Transform, LightEmissionData>{world}
        {}
        void enqueueTo(BaseRenderStage& renderStage, float simulationProgress);
        static std::string getSystemTypeName() { return "RenderSystem::LightQueue"; }
    private:
        void onCreated() override;
        std::shared_ptr<StaticMesh> mSphereMesh { nullptr };
        std::shared_ptr<Material> mLightMaterial{ std::make_shared<Material>() };
    };
    class OpaqueQueue: public System<OpaqueQueue, Transform, std::shared_ptr<StaticModel>> {
    public:
        OpaqueQueue(ECSWorld& world):
        System<OpaqueQueue, Transform, std::shared_ptr<StaticModel>>{world}
        {}
        void enqueueTo(BaseRenderStage& renderStage, float simulationProgress);
        static std::string getSystemTypeName() { return "RenderSystem::OpaqueQueue"; }
    };

    void execute(float simulationProgress);

    void updateCameraMatrices(float simulationProgress);


    /* Set the texture to be rendered to the screen by its relative index */
    void renderNextTexture ();
    void setGamma(float gamma);
    float getGamma();
    void setExposure(float exposure);
    float getExposure();
    std::size_t getCurrentScreenTexture ();

private:
    void onCreated() override;

    std::size_t mCurrentScreenTexture {0};
    std::vector<std::shared_ptr<Texture>> mScreenTextures {};
    std::shared_ptr<Material> mLightMaterialHandle {nullptr};
    EntityID mActiveCamera {};
    GLuint mMatrixUniformBufferIndex {0};
    GLuint mMatrixUniformBufferBinding {0};

    std::shared_ptr<GeometryRenderStage> mGeometryRenderStage { nullptr };
    std::shared_ptr<LightingRenderStage> mLightingRenderStage { nullptr };
    std::shared_ptr<BlurRenderStage> mBlurRenderStage { nullptr };
    std::shared_ptr<TonemappingRenderStage> mTonemappingRenderStage { nullptr };
    std::shared_ptr<ScreenRenderStage> mScreenRenderStage { nullptr };

    float mGamma { 2.f };
    float mExposure { 1.f };
};

#endif
