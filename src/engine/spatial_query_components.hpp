#ifndef ZOSPATIALQUERYCOMPONENTS_H
#define ZOSPATIALQUERYCOMPONENTS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "volumes.hpp"
#include "ecs_world.hpp"

struct ObjectBounds {
    inline static std::string getComponentTypeName() { return "ObjectBounds"; }

    enum class Type {
        BOX,
        SPHERE,
        CAPSULE,
    } mType { Type::BOX };
    bool mRequiresCompute {true};
    glm::vec3 mPositionOffset {0.f};
    glm::quat mDirectionOffset { glm::vec3{0.f} };
    VolumeBox mOrientedBoxDimensions { .mDimensions {glm::vec3{0.f}} };

    union Volume {
        VolumeBox mBox { .mDimensions{glm::vec3{0.f}} };
        VolumeCapsule mCapsule;
        VolumeSphere mSphere;
    } mTrueVolume {};
};


struct WorldBounds {
    inline static std::string getComponentTypeName() { return "AxisAlignedBoxBounds"; }
    glm::vec3 mWorldPosition { 0.f, 0.f, 0.f};
    glm::quat mWorldDirection { glm::vec3{0.f, 0.f, 0.f} };
    VolumeBox mAxisAlignedBoundingBox {};
};

inline void to_json(nlohmann::json& json, const ObjectBounds& objectBounds) {/*never used, so pass*/}
inline void from_json(const nlohmann::json& json, ObjectBounds& objectBounds) {/*never used, so pass*/}
inline void to_json(nlohmann::json& json, const WorldBounds& axisAlignedBounds) {/*never used, so pass*/}
inline void from_json(const nlohmann::json& json, WorldBounds& axisAlignedBounds) {/*never used, so pass*/}

template <>
inline ObjectBounds Interpolator<ObjectBounds>::operator() (
    const ObjectBounds& previousState, const ObjectBounds& nextState,
    float simulationProgress
) const {
    return nextState;
}

template <>
inline WorldBounds Interpolator<WorldBounds>::operator() (
    const WorldBounds& previousState, const WorldBounds& nextState,
    float simulationProgress
) const {
    return nextState;
}

#endif
