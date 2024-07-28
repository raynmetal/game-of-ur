#include <array>

#include <glm/glm.hpp>

#include "simple_ecs.hpp"

#include "light.hpp"
#include "window_context_manager.hpp"
#include "fly_camera.hpp"
#include "render_stage.hpp"
#include "model_manager.hpp"

#include "render_system.hpp"


RenderSystem::RenderSystem():
    mGeometryRenderStage { "src/shader/geometryShader.json" },
    mLightingRenderStage { "src/shader/lightingShader.json" },
    mBlurRenderStage { "src/shader/gaussianblurShader.json" },
    mTonemappingRenderStage { "src/shader/tonemappingShader.json" },
    mScreenRenderStage { "src/shader/screenShader.json" }
{
    Signature lightQueueSignature {};
    lightQueueSignature.set(gComponentManager.getComponentType<Placement>(), true);
    lightQueueSignature.set(gComponentManager.getComponentType<LightEmissionData>(), true);

    Signature opaqueObjectQueueSignature {};
    opaqueObjectQueueSignature.set(gComponentManager.getComponentType<Placement>(), true);
    opaqueObjectQueueSignature.set(gComponentManager.getComponentType<ModelHandle>(), true);

    gSystemManager.registerSystem<LightQueue>(lightQueueSignature);
    gSystemManager.registerSystem<OpaqueQueue>(opaqueObjectQueueSignature);

    MaterialHandle lightMaterialHandle {
        MaterialManager::getInstance().registerResource(
            "lightMaterial",
            {}
        )
    };

    mGeometryRenderStage.setup();
    mLightingRenderStage.setup();
    mBlurRenderStage.setup();
    mTonemappingRenderStage.setup();
    mScreenRenderStage.setup();

    // Set up a uniform buffer for shared matrices
    glGenBuffers(1, &mMatrixUniformBufferIndex);
    glBindBuffer(GL_UNIFORM_BUFFER, mMatrixUniformBufferIndex);
        glBufferData(
            GL_UNIFORM_BUFFER,
            2*sizeof(glm::mat4),
            NULL,
            GL_STATIC_DRAW
        );
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // Let the shader(s) know where to find the shared matrices

    // Bind the shared matrix uniform buffer to the same binding point
    glBindBufferRange(
        GL_UNIFORM_BUFFER,
        mMatrixUniformBufferBinding,
        mMatrixUniformBufferIndex,
        0,
        2*sizeof(glm::mat4)
    );

    // Debug: list of screen textures that may be rendered
    mCurrentScreenTexture = 0;
    mScreenTextures.push_back({mGeometryRenderStage.getRenderTarget("geometryPosition")});
    mScreenTextures.push_back({mGeometryRenderStage.getRenderTarget("geometryNormal")});
    mScreenTextures.push_back({mGeometryRenderStage.getRenderTarget("geometryAlbedoSpecular")});
    mScreenTextures.push_back({mLightingRenderStage.getRenderTarget("litScene")}); 
    mScreenTextures.push_back({mLightingRenderStage.getRenderTarget("brightCutoff")});
    mScreenTextures.push_back({mBlurRenderStage.getRenderTarget("pingBuffer")});
    mScreenTextures.push_back({mTonemappingRenderStage.getRenderTarget("tonemappedScene")});

    // Last pieces of pipeline setup, where we connect all the
    // render stages together
    mLightingRenderStage.attachTexture("positionMap", mGeometryRenderStage.getRenderTarget("geometryPosition"));
    mLightingRenderStage.attachTexture("normalMap", mGeometryRenderStage.getRenderTarget("geometryNormal"));
    mLightingRenderStage.attachTexture("albedoSpecularMap", mGeometryRenderStage.getRenderTarget("geometryAlbedoSpecular"));
    mBlurRenderStage.attachTexture("unblurredImage", mLightingRenderStage.getRenderTarget("brightCutoff"));
    mTonemappingRenderStage.attachTexture("litScene", mLightingRenderStage.getRenderTarget("litScene"));
    mTonemappingRenderStage.attachTexture("bloomEffect", mBlurRenderStage.getRenderTarget("pingBuffer"));
    mScreenRenderStage.attachTexture("renderSource", mScreenTextures[mCurrentScreenTexture]);
    mGeometryRenderStage.validate();
    mLightingRenderStage.validate();
    mBlurRenderStage.validate();
    mTonemappingRenderStage.validate();
    mScreenRenderStage.validate();

    glClearColor(0.f, 0.f, 0.f, 1.f);
}

void RenderSystem::setScreenTexture(std::size_t n) {
    mCurrentScreenTexture = n % mScreenTextures.size();
    mScreenRenderStage.attachTexture("renderSource", mScreenTextures[mCurrentScreenTexture]);
}

void RenderSystem::updateCameraMatrices(const FlyCamera& camera) {
    // Send shared matrices to the uniform buffer
    glBindBuffer(GL_UNIFORM_BUFFER, mMatrixUniformBufferIndex);
        glBufferSubData(
            GL_UNIFORM_BUFFER,
            0,
            sizeof(glm::mat4),
            glm::value_ptr(camera.getProjectionMatrix())
        );
        glBufferSubData(
            GL_UNIFORM_BUFFER,
            sizeof(glm::mat4),
            sizeof(glm::mat4),
            glm::value_ptr(camera.getViewMatrix())
        );
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

std::size_t RenderSystem::getCurrentScreenTexture() {
    return mCurrentScreenTexture;
}

void RenderSystem::execute() {
    // Execute each rendering stage in its proper order
    gSystemManager.getSystem<OpaqueQueue>()->enqueueTo(mGeometryRenderStage);
    mGeometryRenderStage.execute();

    gSystemManager.getSystem<LightQueue>()->enqueueTo(mLightingRenderStage);
    mLightingRenderStage.execute();

    mBlurRenderStage.execute();
    mTonemappingRenderStage.execute();
    mScreenRenderStage.execute();

    WindowContextManager::getInstance().swapBuffers();
}

void RenderSystem::OpaqueQueue::enqueueTo(BaseRenderStage& renderStage) {

    for(EntityID entity: getEnabledEntities()) {
        Placement placement { gComponentManager.getComponent<Placement>(entity) };
        ModelHandle modelHandle { gComponentManager.getComponent<ModelHandle>(entity) };
        std::vector<MeshHandle> meshList { modelHandle.getResource().getMeshHandles() };
        std::vector<MaterialHandle> materialList { modelHandle.getResource().getMaterialHandles() };
        for(std::size_t i{0}; i < meshList.size(); ++i) {
            renderStage.submitToRenderQueue({
                meshList[i],
                materialList[i],
                placement
            });
        }
    }
}

void RenderSystem::LightQueue::enqueueTo(BaseRenderStage& renderStage) {
    MaterialHandle lightMaterialHandle { 
        MaterialManager::getInstance().getResourceHandle("lightMaterial")
    };
    for(EntityID entity: getEnabledEntities()) {
        Placement placement { gComponentManager.getComponent<Placement>(entity) };
        LightEmissionData lightEmissionData { gComponentManager.getComponent<LightEmissionData>(entity) };
        renderStage.submitToRenderQueue(RenderLightUnit {
            mSphereMesh,
            lightMaterialHandle,
            placement,
            lightEmissionData
        });
    }
}
