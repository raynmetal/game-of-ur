#ifndef ZOSCENECOMPONENTS_H
#define ZOSCENECOMPONENTS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "simple_ecs.hpp"

struct Placement {
    glm::vec4 mPosition {glm::vec3{ 0.f }, 1.f};
    glm::quat mOrientation { glm::vec3{ 0.f } };
    glm::vec3 mScale { 1.f, 1.f, 1.f };
    inline static std::string getComponentTypeName() { return "Placement"; }
};

struct Transform {
    glm::mat4 mModelMatrix {1.f};
    inline static std::string getComponentTypeName() { return "Transform"; }
};

template<>
inline Placement Interpolator<Placement>::operator() (
    const Placement& previousState, const Placement& nextState,
    float simulationProgress
) const {
    simulationProgress = mProgressLimits(simulationProgress);
    return {
        .mPosition{ (1.f - simulationProgress) * previousState.mPosition + simulationProgress * nextState.mPosition },
        .mOrientation{ glm::slerp(previousState.mOrientation, nextState.mOrientation, simulationProgress) },
        .mScale{ (1.f - simulationProgress) * previousState.mScale + simulationProgress * nextState.mScale }
    };
}

template<>
inline Transform Interpolator<Transform>::operator() (
    const Transform& previousState, const Transform& nextState,
    float simulationProgress
) const {
    simulationProgress = mProgressLimits(simulationProgress);
    return {
        previousState.mModelMatrix * (1.f - simulationProgress)
        + nextState.mModelMatrix * (simulationProgress)
    };
}


#endif
