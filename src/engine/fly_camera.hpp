#ifndef ZOFLYCAMERA_H
#define ZOFLYCAMERA_H

#include <SDL2/SDL.h>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "apploop_events.hpp"
#include "input_system/input_system.hpp"
#include "simple_ecs.hpp"

class FlyCamera: public IActionHandler {
public:
    FlyCamera();
    FlyCamera(glm::vec3 position, float yaw, float pitch, float fov);

    void update(float deltaTime);
    // Replaces the old handle action event
    void handleAction(const ActionData& actionData, const ActionDefinition& actionDefinition) override;

    glm::vec3 getPosition() const;
    glm::vec3 getForward() const;
    glm::mat4 getViewMatrix() const;
    glm::mat4 getRotationMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    void setActive(bool active);
    void setLookSensitivity(float lookSensitivity);
    void setZoomSensitivity(float zoomSensitivity);

private:
    void updateYaw(float dYaw);
    void updatePitch(float dPitch);
    void updateFOV(float dFOV);

    bool mActive { false };
    float mFOV { 45.f };
    float mLookSensitivity { 0.1f };
    float mZoomSensitivity { 1.5f };

    glm::vec3 mPosition; //position in world units
    glm::vec2 mOrientation; // pitch and yaw, in degrees
    glm::vec3 mVelocity;
};

struct CameraProperties {
    enum class ProjectionType: uint8_t {
        FRUSTUM,
        ORTHOGRAPHIC,
    };
    ProjectionType mProjectionType { ProjectionType::FRUSTUM };
    double mFov {45.f};
    double mOrthographicScale {3.f};
    glm::mat4 mProjectionMatrix {};
    glm::mat4 mViewMatrix {};
};

template<>
inline CameraProperties Interpolator<CameraProperties>::operator() (
    const CameraProperties& previousState, const CameraProperties& nextState,
    float simulationProgress
) const {
    simulationProgress = mProgressLimits(simulationProgress);
    return {
        .mProjectionType {previousState.mProjectionType
        },
        .mProjectionMatrix {(simulationProgress * nextState.mProjectionMatrix) 
            + ((1.f-simulationProgress) * previousState.mProjectionMatrix)
        },
        .mViewMatrix {(simulationProgress * nextState.mViewMatrix)
            + ((1.f-simulationProgress) * previousState.mViewMatrix)
        },
    };
}

class CameraSystem: public System<CameraSystem> {
public:
    void updateActiveCameraMatrices();

    void onEntityUpdated(EntityID entityID) override;
private:
    class ApploopEventHandler : public IApploopEventHandler<ApploopEventHandler> {
    public:
        inline void initializeEventHandler(CameraSystem* pSystem)  { mSystem = pSystem; }
    private:
        void onPreRenderStep(float simulationProgress) override;
        CameraSystem* mSystem;
    friend class CameraSystem;
    };

    std::shared_ptr<ApploopEventHandler> mApploopEventHandler { ApploopEventHandler::registerHandler(this) };
    std::set<EntityID> mProjectionUpdateQueue {};
    std::set<EntityID> mViewUpdateQueue {};
friend class CameraSystem::ApploopEventHandler;
};

#endif
