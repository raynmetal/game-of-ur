#include <array>

#include <glm/glm.hpp>

#include "simple_ecs.hpp"

#include "light.hpp"
#include "window_context_manager.hpp"
#include "fly_camera.hpp"
#include "render_stage.hpp"
#include "model_manager.hpp"

#include "render_system.hpp"

constexpr float MAX_GAMMA  { 3.f };
constexpr float MIN_GAMMA { 1.6f };
constexpr float MAX_EXPOSURE { 15.f };
constexpr float MIN_EXPOSURE { 0.f };

RenderSystem::RenderSystem():
    mGeometryRenderStage { "src/shader/geometryShader.json" },
    mLightingRenderStage { "src/shader/lightingShader.json" },
    mBlurRenderStage { "src/shader/gaussianblurShader.json" },
    mTonemappingRenderStage { "src/shader/tonemappingShader.json" },
    mScreenRenderStage { "src/shader/screenShader.json" }
{
    Signature lightQueueSignature {};
    lightQueueSignature.set(gComponentManager.getComponentType<Placement>(), true);
    lightQueueSignature.set(gComponentManager.getComponentType<Transform>(), true);
    lightQueueSignature.set(gComponentManager.getComponentType<LightEmissionData>(), true);

    Signature opaqueObjectQueueSignature {};
    opaqueObjectQueueSignature.set(gComponentManager.getComponentType<Transform>(), true);
    opaqueObjectQueueSignature.set(gComponentManager.getComponentType<ModelHandle>(), true);

    gSystemManager.registerSystem<LightQueue>(lightQueueSignature);
    gSystemManager.registerSystem<OpaqueQueue>(opaqueObjectQueueSignature);

    mGeometryRenderStage.setup();
    mLightingRenderStage.setup();
    mBlurRenderStage.setup();
    mTonemappingRenderStage.setup();
    mScreenRenderStage.setup();

    MaterialHandle lightMaterialHandle {
        MaterialManager::getInstance().registerResource(
            "lightMaterial",
            {}
        )
    };
    lightMaterialHandle.getResource().updateIntProperty("screenWidth", 800);
    lightMaterialHandle.getResource().updateIntProperty("screenHeight", 600);

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

    // Set initial configuration for the tonemapper
    setGamma(mGamma);
    setExposure(mExposure);

    // Functions containing a set of asserts, ensuring that valid connections have
    // been made between the rendering stages
    mGeometryRenderStage.validate();
    mLightingRenderStage.validate();
    mBlurRenderStage.validate();
    mTonemappingRenderStage.validate();
    mScreenRenderStage.validate();

    glClearColor(0.f, 0.f, 0.f, 1.f);
}

void RenderSystem::renderNextTexture() {
    mCurrentScreenTexture = (mCurrentScreenTexture + 1) % mScreenTextures.size();
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
        Transform entityTransform { gComponentManager.getComponent<Transform>(entity) };
        const std::vector<MeshHandle>& meshList { modelHandle.getResource().getMeshHandles() };
        const std::vector<MaterialHandle>& materialList { modelHandle.getResource().getMaterialHandles() };

        for(std::size_t i{0}; i < meshList.size(); ++i) {
            renderStage.submitToRenderQueue({
                meshList[i],
                materialList[i],
                placement,
                entityTransform.mModelMatrix
            });
        }
    }
}

void RenderSystem::LightQueue::enqueueTo(BaseRenderStage& renderStage) {
    MaterialHandle lightMaterialHandle { 
        MaterialManager::getInstance().getResourceHandle("lightMaterial")
    };
    for(EntityID entity: getEnabledEntities()) {
        Transform entityTransform { gComponentManager.getComponent<Transform>(entity)};
        Placement placement { gComponentManager.getComponent<Placement>(entity) };
        LightEmissionData lightEmissionData { gComponentManager.getComponent<LightEmissionData>(entity) };
        renderStage.submitToRenderQueue(LightRenderUnit {
            mSphereMesh,
            lightMaterialHandle,
            placement,
            lightEmissionData,
            lightEmissionData.mType != LightEmissionData::directional?
                entityTransform.mModelMatrix:
                buildModelMatrix(placement.mPosition, {}, placement.mScale)
        });
    }
}

void RenderSystem::setGamma(float gamma) {
    if(gamma > MAX_GAMMA) gamma = MAX_GAMMA;
    else if (gamma < MIN_GAMMA) gamma = MIN_GAMMA;

    mTonemappingRenderStage.getMaterial("screenMaterial").getResource().updateFloatProperty(
        "gamma", gamma
    );

    mGamma = gamma;
}

void RenderSystem::setExposure(float exposure) {
    if(exposure > MAX_EXPOSURE) exposure = MAX_EXPOSURE;
    else if (exposure < MIN_EXPOSURE) exposure = MIN_EXPOSURE;

    mTonemappingRenderStage.getMaterial("screenMaterial").getResource().updateFloatProperty(
        "exposure", exposure
    );

    mExposure = exposure;
}

void RenderSystem::handleAction(const ActionData& actionData, const ActionDefinition& actionDefinition) {
    if(actionDefinition.mName == "UpdateGamma") {
        setGamma(mGamma + actionData.mOneAxisActionData.mValue*mGammaStep);
    }
    else if(actionDefinition.mName == "UpdateExposure") {
        setExposure(mExposure + actionData.mOneAxisActionData.mValue*mExposureStep);
    }
    else if(actionDefinition.mName == "RenderNextTexture") {
        renderNextTexture();
    }
}
