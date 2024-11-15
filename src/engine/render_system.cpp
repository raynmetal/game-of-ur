#include <array>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "resource_database.hpp"
#include "window_context_manager.hpp"
#include "simple_ecs.hpp"

#include "light.hpp"
#include "camera_system.hpp"
#include "render_stage.hpp"
#include "model.hpp"

#include "render_system.hpp"

constexpr float MAX_GAMMA  { 3.f };
constexpr float MIN_GAMMA { 1.6f };
constexpr float MAX_EXPOSURE { 15.f };
constexpr float MIN_EXPOSURE { 0.f };

RenderSystem::LightQueue::LightQueue() {
    if(!ResourceDatabase::hasResourceDescription("sphereLight-10lat-5long")) {
        nlohmann::json sphereLightDescription {
            {"name", "sphereLight-10lat-5long"},
            {"type", StaticMesh::getName()},
            {"method", StaticMeshSphereLatLong::getName()},
            {"parameters", {
                {"nLatitudes", 10},
                {"nMeridians", 5}
            }}
        };
        ResourceDatabase::addResourceDescription(sphereLightDescription);
    }
    mSphereMesh = ResourceDatabase::getResource<StaticMesh>("sphereLight-10lat-5long");
}

void RenderSystem::onCreated() {
    SimpleECS::registerSystem<LightQueue, Transform, LightEmissionData>();
    SimpleECS::registerSystem<OpaqueQueue, Transform, std::shared_ptr<StaticModel>>();

    mGeometryRenderStage.setup();
    mLightingRenderStage.setup();
    mBlurRenderStage.setup();
    mTonemappingRenderStage.setup();
    mScreenRenderStage.setup();

    if(!ResourceDatabase::hasResourceDescription("lightMaterial")) {
        nlohmann::json lightMaterialDescription{
            {"name", "lightMaterial"},
            {"type", Material::getName()},
            {"method", MaterialFromDescription::getName()},
            {"parameters", {
                {"properties", nlohmann::json::array()},
            }}
        };
        ResourceDatabase::addResourceDescription(lightMaterialDescription);
    }
    mLightMaterialHandle = ResourceDatabase::getResource<Material>("lightMaterial");
    mLightMaterialHandle->updateIntProperty("screenWidth", 800);
    mLightMaterialHandle->updateIntProperty("screenHeight", 600);

    // TODO: Make it so that we can control which camera is used by the render
    // system, and what portion of the screen it renders to
    mActiveCamera = *(getEnabledEntities().begin());
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

void RenderSystem::updateCameraMatrices(float simulationProgress) {
    CameraProperties cameraProps { getComponent<CameraProperties>(mActiveCamera, simulationProgress) };
    // Send shared matrices to the uniform buffer
    glBindBuffer(GL_UNIFORM_BUFFER, mMatrixUniformBufferIndex);
        glBufferSubData(
            GL_UNIFORM_BUFFER,
            0,
            sizeof(glm::mat4),
            glm::value_ptr(cameraProps.mProjectionMatrix)
        );
        glBufferSubData(
            GL_UNIFORM_BUFFER,
            sizeof(glm::mat4),
            sizeof(glm::mat4),
            glm::value_ptr(cameraProps.mViewMatrix)
        );
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

std::size_t RenderSystem::getCurrentScreenTexture() {
    return mCurrentScreenTexture;
}

void RenderSystem::execute(float simulationProgress) {
    mActiveCamera = *(getEnabledEntities().begin());

    updateCameraMatrices(simulationProgress);

    // Execute each rendering stage in its proper order
    SimpleECS::getSystem<OpaqueQueue>()->enqueueTo(mGeometryRenderStage, simulationProgress);
    mGeometryRenderStage.execute();

    SimpleECS::getSystem<LightQueue>()->enqueueTo(mLightingRenderStage, simulationProgress);
    mLightingRenderStage.execute();

    mBlurRenderStage.execute();
    mTonemappingRenderStage.execute();
    mScreenRenderStage.execute();

    WindowContextManager::getInstance().swapBuffers();
}

void RenderSystem::OpaqueQueue::enqueueTo(BaseRenderStage& renderStage, float simulationProgress) {
    for(EntityID entity: getEnabledEntities()) {
        std::shared_ptr<StaticModel> modelHandle { getComponent<std::shared_ptr<StaticModel>>(entity) };
        Transform entityTransform { getComponent<Transform>(entity, simulationProgress) };
        const std::vector<std::shared_ptr<StaticMesh>>& meshList { modelHandle->getMeshHandles() };
        const std::vector<std::shared_ptr<Material>>& materialList { modelHandle->getMaterialHandles() };

        for(std::size_t i{0}; i < meshList.size(); ++i) {
            renderStage.submitToRenderQueue({
                meshList[i],
                materialList[i],
                entityTransform.mModelMatrix
            });
        }
    }
}

void RenderSystem::LightQueue::enqueueTo(BaseRenderStage& renderStage, float simulationProgress) {
    std::shared_ptr<Material> lightMaterialHandle { 
        ResourceDatabase::getResource<Material>("lightMaterial")
    };
    for(EntityID entity: getEnabledEntities()) {
        Transform entityTransform { getComponent<Transform>(entity, simulationProgress)};
        LightEmissionData lightEmissionData { getComponent<LightEmissionData>(entity, simulationProgress) };
        renderStage.submitToRenderQueue(LightRenderUnit {
            mSphereMesh,
            lightMaterialHandle,
            lightEmissionData,
            entityTransform.mModelMatrix
        });
    }
}

void RenderSystem::setGamma(float gamma) {
    if(gamma > MAX_GAMMA) gamma = MAX_GAMMA;
    else if (gamma < MIN_GAMMA) gamma = MIN_GAMMA;

    mTonemappingRenderStage.getMaterial("screenMaterial")->updateFloatProperty(
        "gamma", gamma
    );

    mGamma = gamma;
}

void RenderSystem::setExposure(float exposure) {
    if(exposure > MAX_EXPOSURE) exposure = MAX_EXPOSURE;
    else if (exposure < MIN_EXPOSURE) exposure = MIN_EXPOSURE;

    mTonemappingRenderStage.getMaterial("screenMaterial")->updateFloatProperty(
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
