#include <algorithm>
#include <set>

#include "scene_system.hpp"
#include "camera_system.hpp"

const float MAX_FOV {90.f};
const float MIN_FOV {40.f};


void CameraSystem::updateActiveCameraMatrices() {
    std::set<EntityID> enabledEntities { getEnabledEntities() };

    // Build a list of active cameras that require projection or
    // view updates
    std::set<EntityID> requiresProjectionUpdate {};
    std::set<EntityID> requiresViewUpdate {};
    std::set_intersection(mProjectionUpdateQueue.begin(), mProjectionUpdateQueue.end(),
        enabledEntities.begin(), enabledEntities.end(),
        std::inserter(requiresProjectionUpdate, requiresProjectionUpdate.end())
    );
    std::set_intersection(mViewUpdateQueue.begin(), mViewUpdateQueue.end(),
        enabledEntities.begin(), enabledEntities.end(),
        std::inserter(requiresViewUpdate, requiresViewUpdate.begin())
    );
    // any matrices remaining in these queues were disabled or removed
    // before updateActiveCameraMatrices was called
    mViewUpdateQueue.clear();
    mProjectionUpdateQueue.clear();

    // Apply pending updates
    for(EntityID entity: requiresProjectionUpdate) {
        CameraProperties cameraProperties { getComponent<CameraProperties>(entity, 1.f) };
        switch(cameraProperties.mProjectionType) {
            case CameraProperties::ProjectionType::FRUSTUM:
                cameraProperties.mProjectionMatrix = glm::perspective(
                    glm::radians(glm::clamp<double>(cameraProperties.mFov, MIN_FOV, MAX_FOV)), 
                    static_cast<double>(800.f/600.f),
                    static_cast<double>(.5f), static_cast<double>(100.f)
                );
            break;
            case CameraProperties::ProjectionType::ORTHOGRAPHIC:
                cameraProperties.mProjectionMatrix = static_cast<float>(cameraProperties.mOrthographicScale) * glm::mat4(1.f) * glm::ortho(-4.f, 4.f, -3.f, 3.f, 0.5f, 4.5f);
            break;
        }
        updateComponent<CameraProperties>(entity, cameraProperties);
    }
    for(EntityID entity: requiresViewUpdate) {
        CameraProperties cameraProperties { getComponent<CameraProperties>(entity, 1.f) };
        cameraProperties.mViewMatrix = glm::inverse(getComponent<Transform>(entity, 1.f).mModelMatrix);
        updateComponent<CameraProperties>(entity, cameraProperties);
    }
}

void CameraSystem::onEntityUpdated(EntityID entity)  {
    const Transform placementBefore { getComponent<Transform>(entity, 0.f) };
    const Transform placementAfter { getComponent<Transform>(entity, 1.f) };
    const CameraProperties cameraPropsBefore { getComponent<CameraProperties>(entity, 0.f) };
    const CameraProperties cameraPropsAfter { getComponent<CameraProperties>(entity, 1.f) };

    if(placementBefore.mModelMatrix != placementAfter.mModelMatrix) {
        mViewUpdateQueue.insert(entity);
    }

    if(
        cameraPropsBefore.mProjectionType != cameraPropsAfter.mProjectionType
        || (
            cameraPropsAfter.mProjectionType == CameraProperties::ProjectionType::FRUSTUM
            && (cameraPropsAfter.mFov != cameraPropsBefore.mFov)
        ) || (
            cameraPropsAfter.mProjectionType == CameraProperties::ProjectionType::ORTHOGRAPHIC
            && (cameraPropsAfter.mOrthographicScale != cameraPropsBefore.mOrthographicScale)
        )
    ) {
        mProjectionUpdateQueue.insert(entity);
    }
}

void CameraSystem::ApploopEventHandler::onPreRenderStep(float simulationProgress) {
    mSystem->updateActiveCameraMatrices();
}

void CameraSystem::ApploopEventHandler::onApplicationStart() {
    for(EntityID entity : mSystem->getEnabledEntities()) {
        mSystem->mViewUpdateQueue.insert(entity);
        mSystem->mProjectionUpdateQueue.insert(entity);
    }
    mSystem->updateActiveCameraMatrices();
}
