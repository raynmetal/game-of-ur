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

struct RenderSet;
using RenderSetID = uint32_t;

const RenderSetID kMaxRenderSetIDs { 10000 };

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
        void onInitialize() override;
        std::shared_ptr<StaticMesh> mSphereMesh { nullptr };
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
    void renderToScreen();

    /* Set the texture to be rendered to the screen by its relative index */
    RenderSetID createRenderSet(glm::u16vec2 renderDimensions, glm::u16vec2 targetDimensions, const SDL_Rect& viewportDimensions);
    void useRenderSet(RenderSetID renderSet);
    void setRenderProperties(glm::u16vec2 renderDimensions, glm::u16vec2 targetDimensions, const SDL_Rect& viewportDimensions);
    void deleteRenderSet(RenderSetID renderSet);

    // Render set functions
    void renderNextTexture ();
    void setCamera(EntityID cameraEntity);
    void setGamma(float gamma);
    float getGamma();
    void setExposure(float exposure);
    float getExposure();
    std::shared_ptr<Texture> getCurrentScreenTexture ();

private:
    void onInitialize() override;
    void copyAndResize();

    std::map<RenderSetID, RenderSet> mRenderSets {};
    RenderSetID mActiveRenderSetID;
    std::set<RenderSetID> mDeletedRenderSetIDs {};
    RenderSetID mNextRenderSetID { 0 };

    GLuint mMatrixUniformBufferIndex { 0 };
    GLuint mMatrixUniformBufferBinding { 0 };
};

struct RenderSet {
    void renderNextTexture ();
    void setRenderProperties(glm::u16vec2 renderDimensions, glm::u16vec2 targetDimensions, const SDL_Rect& viewportDimensions);
    std::shared_ptr<Texture> getCurrentScreenTexture ();
    void copyAndResize();

    void setCamera(EntityID cameraEntity);
    void setGamma(float gamma);
    float getGamma();
    void setExposure(float exposure);
    float getExposure();


    std::size_t mCurrentScreenTexture {0};
    std::vector<std::shared_ptr<Texture>> mScreenTextures {};
    EntityID mActiveCamera {};
    std::shared_ptr<Material> mLightMaterialHandle {nullptr};

    std::shared_ptr<GeometryRenderStage> mGeometryRenderStage { nullptr };
    std::shared_ptr<LightingRenderStage> mLightingRenderStage { nullptr };
    std::shared_ptr<BlurRenderStage> mBlurRenderStage { nullptr };
    std::shared_ptr<TonemappingRenderStage> mTonemappingRenderStage { nullptr };

    std::shared_ptr<ResizeRenderStage> mResizeRenderStage { nullptr };
    std::shared_ptr<ScreenRenderStage> mScreenRenderStage { nullptr };

    float mGamma { 2.f };
    float mExposure { 1.f };
    bool mRerendered { true };
};

#endif
