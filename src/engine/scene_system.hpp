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

#endif
