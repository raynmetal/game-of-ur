#include <algorithm>
#include <set>

#include "scene_system.hpp"
#include "camera_system.hpp"

const float MAX_FOV {90.f};
const float MIN_FOV {40.f};


void CameraSystem::updateActiveCameraMatrices() {
    std::set<EntityID> enabledEntities { getEnabledEntities() };

    // Get and clear list of cameras awaiting a view or projection update
    std::set<EntityID> requiresProjectionUpdate{};
    std::set<EntityID> requiresViewUpdate{};
    std::swap(requiresProjectionUpdate, mProjectionUpdateQueue);
    std::swap(requiresViewUpdate, mViewUpdateQueue);

    // Apply pending updates
    for(EntityID entity: requiresProjectionUpdate) {
        CameraProperties cameraProperties { getComponent<CameraProperties>(entity) };
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
        CameraProperties cameraProperties { getComponent<CameraProperties>(entity) };
        cameraProperties.mViewMatrix = glm::inverse(getComponent<Transform>(entity).mModelMatrix);
        updateComponent<CameraProperties>(entity, cameraProperties);
    }
}

void CameraSystem::onEntityEnabled(EntityID entity) {
    mViewUpdateQueue.insert(entity);
    mProjectionUpdateQueue.insert(entity);
}
void CameraSystem::onEntityDisabled(EntityID entity) {
    mViewUpdateQueue.erase(entity);
    mProjectionUpdateQueue.erase(entity);
}

void CameraSystem::onEntityUpdated(EntityID entity)  {
    mViewUpdateQueue.insert(entity);
    mProjectionUpdateQueue.insert(entity);
}

void CameraSystem::onPreRenderStep(float simulationProgress) {
    updateActiveCameraMatrices();
}

void CameraSystem::onSimulationActivated() {
    for(EntityID entity : getEnabledEntities()) {
        mViewUpdateQueue.insert(entity);
        mProjectionUpdateQueue.insert(entity);
    }
    updateActiveCameraMatrices();
}
