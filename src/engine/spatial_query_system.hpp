#ifndef ZOSPATIALQUERYSYSTEM_H
#define ZOSPATIALQUERYSYSTEM_H

#include <memory>
#include <array>

#include "spatial_query_octree.hpp"
#include "ecs_world.hpp"
#include "model.hpp"
#include "light.hpp"
#include "scene_system.hpp"

class SpatialQuerySystem: public System<SpatialQuerySystem, std::tuple<Transform, ObjectBounds>, std::tuple<SceneHierarchyData, AxisAlignedBounds>> {
public:
    explicit SpatialQuerySystem(std::weak_ptr<ECSWorld> world):
    System<SpatialQuerySystem, std::tuple<Transform, ObjectBounds>, std::tuple<SceneHierarchyData, AxisAlignedBounds>>{world} 
    {}
    static std::string getSystemTypeName() { return "SpatialQuerySystem"; }

    class StaticModelBoundsComputeSystem: public System<StaticModelBoundsComputeSystem, std::tuple<std::shared_ptr<StaticModel>>, std::tuple<ObjectBounds>> {
    public:
        explicit StaticModelBoundsComputeSystem(std::weak_ptr<ECSWorld> world):
        System<SpatialQuerySystem::StaticModelBoundsComputeSystem, std::tuple<std::shared_ptr<StaticModel>>, std::tuple<ObjectBounds>> { world }
        {}
        static std::string getSystemTypeName() { return "SpatialQuerySystem::StaticModelBoundsComputeSystem"; }
    private:
        void onEntityEnabled(EntityID entityID) override;
        void onEntityUpdated(EntityID entityID) override;
        void recomputeObjectBounds(EntityID entityID);
    };

    class LightBoundsComputeSystem: public System<LightBoundsComputeSystem, std::tuple<LightEmissionData>, std::tuple<ObjectBounds>> {
    public:
        explicit LightBoundsComputeSystem(std::weak_ptr<ECSWorld> world):
        System<SpatialQuerySystem::LightBoundsComputeSystem, std::tuple<LightEmissionData>, std::tuple<ObjectBounds>>{world}
        {}
        static std::string getSystemTypeName() { return "SpatialQuerySystem::LightBoundsComputeSystem"; }
    private:
        void onEntityEnabled(EntityID entityID) override;
        void onEntityUpdated(EntityID entityID) override;
        void recomputeObjectBounds(EntityID entityID);
    };

    std::vector<std::pair<EntityID, AxisAlignedBounds>> findEntitiesOverlapping(const AxisAlignedBounds& searchBounds) const;
    std::vector<std::pair<EntityID, AxisAlignedBounds>> findEntitiesOverlapping(const Ray& ray) const;

private:
    void updateBounds(EntityID entity);
    void rebuildOctree();

    void onSimulationActivated() override;
    void onSimulationStep(uint32_t timestepMillis) override;
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
