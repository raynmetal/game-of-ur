#include <SDL2/SDL.h>

#include "../engine/input_system/input_data.hpp"
#include "../engine/spatial_query_system.hpp"
#include "../engine/scene_system.hpp"
#include "../engine/camera_system.hpp"
#include "fly_camera.hpp"

const float DEFAULT_CAMERA_SPEED { 2.f };
const float MAX_PITCH { 89.f };
const float LOOK_SENSITIVITY { .23f };
const float ZOOM_SENSITIVITY { 1.5f };
const float MAX_FOV {100.f};
const float MIN_FOV {40.f};

void FlyCamera::variableUpdate(uint32_t variableStepMillis) {
    if(!mActive) return;

    ToyMakersEngine::Placement placement { getComponent<ToyMakersEngine::Placement>() };

    const glm::mat4 rotationMatrix { glm::mat4_cast(placement.mOrientation) };
    const glm::vec4 localForward { rotationMatrix * glm::vec4{0.f, 0.f, -1.f, 0.f} };
    const glm::vec4 localRight { rotationMatrix * glm::vec4{1.f, 0.f, 0.f, 0.f} };

    placement.mPosition += (
        (variableStepMillis/1000.f) * (
            mVelocity.z * localForward
            +  mVelocity.x * localRight
        )
    );
    updateComponent<ToyMakersEngine::Placement>(placement);
}

bool FlyCamera::onToggleControl(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    setActive(!mActive);
    return true;
}

bool FlyCamera::onRotate(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition & actionDefinition) {
    if(!mActive) return false;

    // Handle camera rotation commands
    ToyMakersEngine::Placement placement { getComponent<ToyMakersEngine::Placement>() };
    updatePitch(placement.mOrientation, actionData.mTwoAxisActionData.mValue.y);
    updateYaw(placement.mOrientation, actionData.mTwoAxisActionData.mValue.x);
    updateComponent<ToyMakersEngine::Placement>(placement);
    return true;
}

bool FlyCamera::onMove(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    if(!mActive) return false;
    // movement commands
    mVelocity.x = mMaxSpeed * actionData.mTwoAxisActionData.mValue.x;
    mVelocity.z = mMaxSpeed * actionData.mTwoAxisActionData.mValue.y;
    return true;
}

bool FlyCamera::onUpdateFOV(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    if(!mActive) return false;
    // "Zoom" commands
    const float dFOV { static_cast<float>(-actionData.mOneAxisActionData.mValue) };
    updateFOV(dFOV);
    return true;
}

void FlyCamera::updatePitch(glm::quat& current, float dPitch) {
    float dOutPitch { mLookSensitivity * -dPitch };

    // apply constraints to pitch
    const float oldPitch { glm::pitch(current) };
    const float newPitch { oldPitch + dOutPitch };

    // TODO: find a better way to do this
    // Overcomplicated way to constrain pitch to range (-MAX_PITCH, MAX_PITCH)
    if(
        glm::sin(newPitch) > glm::sin(glm::radians(MAX_PITCH)) // approaching complete alignment with the up vector
        || ( // or
            !std::signbit(glm::sin(newPitch)) && !std::signbit(glm::sin(oldPitch)) // positive y axis
            && std::signbit(glm::cos(newPitch)) != std::signbit(glm::cos(oldPitch)) // but flip in x axis
        )
    ) { 
        dOutPitch = (
            glm::cos(oldPitch) < 0.f? 
                glm::radians(180.f) - glm::radians(MAX_PITCH): 
                glm::radians(MAX_PITCH)
        ) - oldPitch;
    }
    else if(
        glm::sin(newPitch) < glm::sin(glm::radians(-MAX_PITCH))
        || (
            std::signbit(glm::sin(newPitch)) && std::signbit(glm::sin(oldPitch)) // negative y axis
            && std::signbit(glm::cos(newPitch)) != std::signbit(glm::cos(oldPitch))  // but flip in x axis
        )
    ) {
        dOutPitch = (
            glm::cos(oldPitch) < 0.f?
                glm::radians(180.f) - glm::radians(-MAX_PITCH):
                glm::radians(-MAX_PITCH)
        ) - oldPitch;
    }

    const glm::quat pitchQuat { glm::vec3{ dOutPitch, 0.f, 0.f} };
    current = glm::normalize(current * pitchQuat);
}

void FlyCamera::updateYaw(glm::quat& current, float dYaw) {
    float dOutYaw{ mLookSensitivity * -dYaw };
    const glm::quat yawQuat { glm::vec3{ 0.f, dOutYaw, 0.f} };
    current = glm::normalize(yawQuat * current);
}

void FlyCamera::setActive(bool active) {
    mActive = active;
    SDL_SetRelativeMouseMode(mActive? SDL_TRUE: SDL_FALSE);
    // SDL_ShowCursor(mActive? SDL_FALSE: SDL_TRUE);
}

void FlyCamera::updateFOV(float dFOV) {
    ToyMakersEngine::CameraProperties cameraProps { getComponent<ToyMakersEngine::CameraProperties>() };
    cameraProps.mFov += mZoomSensitivity * dFOV;
    if(cameraProps.mFov > MAX_FOV) { cameraProps.mFov = MAX_FOV; }
    else if(cameraProps.mFov < MIN_FOV) { cameraProps.mFov = MIN_FOV; }
    updateComponent<ToyMakersEngine::CameraProperties>(cameraProps);
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> FlyCamera::clone() const {
    return std::shared_ptr<FlyCamera>(new FlyCamera{});
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> FlyCamera::create(const nlohmann::json& jsonAspectProeprties) {
    return std::shared_ptr<FlyCamera>(new FlyCamera{});
}
