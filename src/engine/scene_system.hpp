#ifndef ZOSCENESYSTEM_H
#define ZOSCENESYSTEM_H

#include <vector>
#include <queue>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "simple_ecs.hpp"

enum class RelativeTo:int {
    Parent=0,
    World=1,
};

struct Placement {
    glm::vec4 mPosition {glm::vec3{ 0.f }, 1.f};
    glm::quat mOrientation { glm::vec3{ 0.f } };
    glm::vec3 mScale { 1.f, 1.f, 1.f };
};

struct Transform {
    glm::mat4 mModelMatrix {1.f};
};

struct SceneNode {
    EntityID mParent { kMaxEntities };
    RelativeTo mRelativeTo { RelativeTo::Parent };
    std::string mName {""};
    std::set<EntityID> mChildren {};
};

class SceneSystem: public System {
public:
    void rebuildGraph();
    void markDirty(EntityID entity);
    void updateTransforms();

private:
    SceneNode mRootNode {};
    std::set<EntityID> mComputeTransformQueue {};
    std::set<EntityID> mValidatedEntities {};

    bool cycleDetected(EntityID entityID);
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

template<>
inline SceneNode Interpolator<SceneNode>::operator() (
    const SceneNode& previousState, const SceneNode& nextState,
    float simulationProgress
) const {
    // Once a node has been reparented, there's no reason
    // to think about the old parent
    return nextState;
}

#endif
