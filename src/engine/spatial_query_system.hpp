#ifndef ZOSPATIALQUERYSYSTEM_H
#define ZOSPATIALQUERYSYSTEM_H

#include <memory>
#include <array>

#include "spatial_query_octree.hpp"
#include "ecs_world.hpp"
#include "model.hpp"
#include "light.hpp"
#include "scene_system.hpp"

class SpatialQuerySystem: public System<SpatialQuerySystem, SceneHierarchyData, Transform, ObjectBounds, AxisAlignedBounds> {
public:
    SpatialQuerySystem(std::weak_ptr<ECSWorld> world):
    System<SpatialQuerySystem, SceneHierarchyData, Transform, ObjectBounds, AxisAlignedBounds>{world} 
    {}
    static std::string getSystemTypeName() { return "SpatialQuerySystem"; }

    class StaticModelBoundsComputeSystem: public System<StaticModelBoundsComputeSystem, ObjectBounds, std::shared_ptr<StaticModel>> {
    public:
        StaticModelBoundsComputeSystem(std::weak_ptr<ECSWorld> world):
        System<SpatialQuerySystem::StaticModelBoundsComputeSystem, ObjectBounds, std::shared_ptr<StaticModel>> { world }
        {}
        static std::string getSystemTypeName() { return "SpatialQuerySystem::StaticModelBoundsComputeSystem"; }
    private:
        void onEntityEnabled(EntityID entityID) override;
        void onEntityUpdated(EntityID entityID) override;
        void recomputeObjectBounds(EntityID entityID);
    };

    class LightBoundsComputeSystem: public System<LightBoundsComputeSystem, ObjectBounds, LightEmissionData> {
    public:
        LightBoundsComputeSystem(std::weak_ptr<ECSWorld> world):
        System<SpatialQuerySystem::LightBoundsComputeSystem, ObjectBounds, LightEmissionData>{world}
        {}
        static std::string getSystemTypeName() { return "SpatialQuerySystem::LightBoundsComputeSystem"; }
    private:
        void onEntityEnabled(EntityID entityID) override;
        void onEntityUpdated(EntityID entityID) override;
        void recomputeObjectBounds(EntityID entityID);
    };

private:
    void updateBounds(EntityID entity);
    void rebuildOctree();

    void onSimulationActivated() override;
    void onPostTransformUpdate(uint32_t timestepMillis) override;
    void onEntityEnabled(EntityID entityID) override;
    void onEntityDisabled(EntityID entityID) override;
    void onEntityUpdated(EntityID entityID) override;

    std::unique_ptr<Octree> mOctree { nullptr };
    std::set<EntityID> mComputeQueue {};
    bool mRequiresInitialization { true };
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
inline void SceneNodeCore::updateComponent<AxisAlignedBounds>(const AxisAlignedBounds& axisAlignedBoxBounds) {
    assert(false && "Cannot update a scene node's WorldBounds component");
}
template<>
inline void SceneNodeCore::updateComponent<ObjectBounds>(const ObjectBounds& objectBounds) {
    assert(false && "Cannot update a scene node's ObjectBounds component");
}
template <>
inline void SceneNodeCore::removeComponent<AxisAlignedBounds>() {
    assert(false && "Cannot remove a scene node's AABBTreeNode component");
}
template <>
inline void SceneNodeCore::removeComponent<ObjectBounds>() {
    assert(false && "Cannot remove a scene node's ObjectBounds component");
}

#endif
