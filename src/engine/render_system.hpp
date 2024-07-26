#ifndef ZORENDERSYSTEM_H
#define ZORENDERSYSTEM_H

#include <vector>
#include <map>
#include <string>
#include <array>

#include "simple_ecs.hpp"
#include "shapegen.hpp"
#include "render_stage.hpp"
#include "fly_camera.hpp"

class RenderSystem: public System {
public:
    class LightQueue: public System {
    public:
        void enqueueTo(BaseRenderStage& renderStage);
    private:
        MeshHandle mSphereMesh { generateSphereMesh(10, 5) };
        MaterialHandle mLightMaterial{};
    };
    class OpaqueQueue: public System {
    public:
        void enqueueTo(BaseRenderStage& renderStage);
    };

    RenderSystem();
    void execute();

    void updateCameraMatrices(const FlyCamera& camera);

    /* Set the texture to be rendered to the screen by its index */
    void setScreenTexture (std::size_t n);
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
};

#endif
