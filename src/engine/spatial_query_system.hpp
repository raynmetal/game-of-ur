#ifndef ZOSPATIALQUERYSYSTEM_H
#define ZOSPATIALQUERYSYSTEM_H

#include "ecs_world.hpp"
#include "model.hpp"
#include "light.hpp"
#include "scene_system.hpp"
#include "spatial_query_components.hpp"
#include "volumes.hpp"

class SpatialQuerySystem: public System<SpatialQuerySystem, SceneHierarchyData, WorldBounds, ObjectBounds> {
public:
    SpatialQuerySystem(std::weak_ptr<ECSWorld> world):
    System<SpatialQuerySystem, SceneHierarchyData, WorldBounds, ObjectBounds>{world} 
    {}
    static std::string getSystemTypeName() { return "SpatialQuerySystem"; }

    class StaticModelBoundsComputeSystem: public System<StaticModelBoundsComputeSystem, ObjectBounds, std::shared_ptr<StaticModel>> {
    public:
        StaticModelBoundsComputeSystem(std::weak_ptr<ECSWorld> world):
        System<SpatialQuerySystem::StaticModelBoundsComputeSystem, ObjectBounds, std::shared_ptr<StaticModel>> { world }
        {}
        static std::string getSystemTypeName() { return "SpatialQuerySystem::StaticModelBoundComputeSystem"; }
    private:
        void onEntityEnabled(EntityID entityID) override;
    };

    class LightBoundsComputeSystem: public System<LightBoundsComputeSystem, ObjectBounds, LightEmissionData> {
    public:
        LightBoundsComputeSystem(std::weak_ptr<ECSWorld> world):
        System<SpatialQuerySystem::LightBoundsComputeSystem, ObjectBounds, LightEmissionData>{world}
        {}
        static std::string getSystemTypeName() { return "SpatialQuerySystem::LightBoundsComputeSystem"; }
    private:
        void onEntityEnabled(EntityID entityID) override;
    };

private:

};

// Prevent enabling and disabling of spatial query related systems, leave their management entirely to 
// the scene system
template <>
inline void SceneNodeCore::setEnabled<SpatialQuerySystem>(bool) {/* pass */}
template <>
inline void SceneNodeCore::setEnabled<SpatialQuerySystem::LightBoundsComputeSystem>(bool) {/*pass*/}
template <>
inline void SceneNodeCore::setEnabled<SpatialQuerySystem::StaticModelBoundsComputeSystem>(bool) {/*pass*/}
template <>

inline void SceneNodeCore::updateComponent<WorldBounds>(const WorldBounds& axisAlignedBoxBounds) {
    assert(false && "Cannot update a scene node's WorldBounds component");
}
template<>
inline void SceneNodeCore::updateComponent<ObjectBounds>(const ObjectBounds& objectBounds) {
    assert(false && "Cannot update a scene node's ObjectBounds component");
}
template <>
inline void SceneNodeCore::removeComponent<WorldBounds>() {
    assert(false && "Cannot remove a scene node's AABBTreeNode component");
}
template <>
inline void SceneNodeCore::removeComponent<ObjectBounds>() {
    assert(false && "Cannot remove a scene node's ObjectBounds component");
}

#endif
