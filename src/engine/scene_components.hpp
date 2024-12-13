#ifndef ZOSCENECOMPONENTS_H
#define ZOSCENECOMPONENTS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

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

inline void from_json(const nlohmann::json& json, Placement& placement) {
    assert(json.at("type").get<std::string>() == Placement::getComponentTypeName() && "Type mismatch. Component json must have type Placement");
    json.at("position").at(0).get_to(placement.mPosition.x);
    json.at("position").at(1).get_to(placement.mPosition.y);
    json.at("position").at(2).get_to(placement.mPosition.z);
    json.at("position").at(3).get_to(placement.mPosition.w);

    json.at("orientation").at(0).get_to(placement.mOrientation.w);
    json.at("orientation").at(1).get_to(placement.mOrientation.x);
    json.at("orientation").at(2).get_to(placement.mOrientation.y);
    json.at("orientation").at(3).get_to(placement.mOrientation.z);
    placement.mOrientation = glm::normalize(placement.mOrientation);

    json.at("scale").at(0).get_to(placement.mScale.x);
    json.at("scale").at(1).get_to(placement.mScale.y);
    json.at("scale").at(2).get_to(placement.mScale.z);
}
inline void to_json(nlohmann::json& json, const Placement& placement) {
    json = {
        {"type", Placement::getComponentTypeName()},
        {"position", {
            placement.mPosition.x,
            placement.mPosition.y,
            placement.mPosition.z,
            placement.mPosition.w,
        }},
        {"orientation", {
            placement.mOrientation.w,
            placement.mOrientation.x,
            placement.mOrientation.y,
            placement.mOrientation.z,
        }},
        {"scale", {
            placement.mScale.x,
            placement.mScale.y,
            placement.mScale.z,
        }}
    };
}
inline void from_json(const nlohmann::json& json, Transform& transform) {
    assert(json.at("type") == Transform::getComponentTypeName() && "Type mismatch. Component json must have type Transform");
    transform.mModelMatrix = glm::mat4{1.f};
}
inline void to_json(nlohmann::json& json, const Transform& transform) {
    json = {
        {"type", Transform::getComponentTypeName()},
    };
}

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
