#include <limits>
#include <cmath>

#include "spatial_query_system.hpp"

void SpatialQuerySystem::StaticModelBoundsComputeSystem::onEntityEnabled(EntityID entityID) {
    recomputeObjectBounds(entityID);
}

void SpatialQuerySystem::StaticModelBoundsComputeSystem::onEntityUpdated(EntityID entityID) {
    const std::shared_ptr<StaticModel> previousModel {getComponent<std::shared_ptr<StaticModel>>(entityID, 0.f)};
    const std::shared_ptr<StaticModel> newModel { getComponent<std::shared_ptr<StaticModel>>(entityID, 1.f) };
    if(previousModel != newModel) {
        recomputeObjectBounds(entityID);
    }
}

void SpatialQuerySystem::StaticModelBoundsComputeSystem::recomputeObjectBounds(EntityID entityID) {
    ObjectBounds objectBounds { getComponent<ObjectBounds>(entityID) };

    const std::shared_ptr<StaticModel> model { getComponent<std::shared_ptr<StaticModel>>(entityID) };
    glm::vec3 minCorner {std::numeric_limits<float>::infinity()};
    glm::vec3 maxCorner {-std::numeric_limits<float>::infinity()};
    std::vector<std::shared_ptr<StaticMesh>> meshHandles { model->getMeshHandles() };
    if(meshHandles.empty()) {
        minCorner = glm::vec3 { 0.f };
        maxCorner = glm::vec3 { 0.f };

    } else {
        for(auto meshHandle: model->getMeshHandles()) {
            assert(meshHandle->getVertexListBegin() != meshHandle->getVertexListEnd() && "Cannot compute bounding volume of an empty mesh");
            for(
                auto vertexIter { meshHandle->getVertexListBegin()};
                vertexIter != meshHandle->getVertexListEnd();
                ++vertexIter
            ) {
                minCorner = glm::min(static_cast<glm::vec3>(vertexIter->mPosition), minCorner);
                maxCorner = glm::max(static_cast<glm::vec3>(vertexIter->mPosition), maxCorner);
            }
        }
    }

    const VolumeBox box { .mDimensions{ maxCorner - minCorner } };
    objectBounds = ObjectBounds::create(
        box,
        minCorner + .5f * box.mDimensions,
        glm::vec3{0.f, 0.f, 0.f}
    );

    updateComponent<ObjectBounds>(entityID, objectBounds);
}

void SpatialQuerySystem::LightBoundsComputeSystem::onEntityEnabled(EntityID entityID) {
    recomputeObjectBounds(entityID);
}
void SpatialQuerySystem::LightBoundsComputeSystem::onEntityUpdated(EntityID entityID) {
    const LightEmissionData previousLightData { getComponent<LightEmissionData>(entityID, 0.f) };
    const LightEmissionData newLightData { getComponent<LightEmissionData>(entityID, 1.f) };
    if(
        newLightData.mRadius != previousLightData.mRadius
        || newLightData.mType != previousLightData.mType
        || newLightData.mDiffuseColor != previousLightData.mDiffuseColor
    ) {
        recomputeObjectBounds(entityID);
    }
}

void SpatialQuerySystem::LightBoundsComputeSystem::recomputeObjectBounds(EntityID entityID) {
    ObjectBounds objectBounds { getComponent<ObjectBounds>(entityID) };

    LightEmissionData lightEmissionData { getComponent<LightEmissionData>(entityID) };
    objectBounds.mTrueVolume.mSphere.mRadius = lightEmissionData.mType == LightEmissionData::LightType::directional?
        0.f:
        lightEmissionData.mRadius;
    objectBounds.mType = ObjectBounds::TrueVolumeType::SPHERE;
    objectBounds.mOrientationOffset = glm::vec3{0.f};
    objectBounds.mPositionOffset = glm::vec3{0.f};

    updateComponent<ObjectBounds>(entityID, objectBounds);
}


void SpatialQuerySystem::updateBounds(EntityID entity) {
    // Compute new object position based on its transform
    const glm::mat4 modelMatrix { getComponent<Transform>(entity).mModelMatrix };
    ObjectBounds objectBounds { getComponent<ObjectBounds>(entity) };
    objectBounds.applyModelMatrix(modelMatrix);

    // Compute axis aligned bounds based on object bounds
    const AxisAlignedBounds axisAlignedBounds { objectBounds };

    // Apply updates
    updateComponent<ObjectBounds>(entity, objectBounds);
    updateComponent<AxisAlignedBounds>(entity, axisAlignedBounds);   
}


void SpatialQuerySystem::rebuildOctree() {
    // one pass to transform all object positions and orientations, and simultaneously compute 
    // axis aligned bounding boxes
    AxisAlignedBounds regionToEncompass {glm::vec3{0.f}, glm::vec3{0.f}};
    bool firstObject { true };
    for(EntityID entity: getEnabledEntities()) {
        updateBounds(entity);
        if(firstObject) {
            regionToEncompass.setPosition(getComponent<ObjectBounds>(entity).getComputedWorldPosition());
            firstObject = false;
        }
        regionToEncompass = regionToEncompass + getComponent<AxisAlignedBounds>(entity);
    }
    assert(isFinite(regionToEncompass.getPosition()) && "Start position must be finite");
    assert(isFinite(regionToEncompass.getDimensions()) && "Region to encompass is too large to be bound in an octree");
    if(!isPositive(regionToEncompass.getDimensions())) {
        regionToEncompass.setDimensions(glm::vec3{1.f});
    }

    // another pass to create and populate our octree
    mOctree = std::make_unique<Octree>(8, regionToEncompass);
    for(EntityID entity: getEnabledEntities()) {
        mOctree->insertEntity(entity, getComponent<AxisAlignedBounds>(entity));
    }
}

void SpatialQuerySystem::onEntityEnabled(EntityID entityID) {
    mComputeQueue.insert(entityID);
}

void SpatialQuerySystem::onEntityDisabled(EntityID entityID) {
    mComputeQueue.erase(entityID);
    if(!mRequiresInitialization) { mOctree->removeEntity(entityID); }
}

void SpatialQuerySystem::onEntityUpdated(EntityID entityID) {
    const Transform previousTransform { getComponent<Transform>(entityID, 0.f) };
    const Transform newTransform { getComponent<Transform>(entityID, 1.f) };
    const ObjectBounds previousBounds { getComponent<ObjectBounds>(entityID, 0.f) };
    const ObjectBounds newBounds { getComponent<ObjectBounds>(entityID, 1.f) };

    if(
        newTransform.mModelMatrix != previousTransform.mModelMatrix
        || newBounds.mOrientationOffset != previousBounds.mOrientationOffset
        || newBounds.mPositionOffset != previousBounds.mPositionOffset
        // TODO: This is a stopgap that works on the assumption that the parameters
        // representing a volume will be at most 3 floats long (i.e., the no. of dimensions of
        // a cube);  If a new type is added that requires more parameters, this will need
        // to be revised.
        // (To research: can we do a comparison of all the bits representing
        // a union, regardless of how large it is? Say by reinterpreting it as
        // an integer or an array of integers?)
        || newBounds.mTrueVolume.mBox.mDimensions != previousBounds.mTrueVolume.mBox.mDimensions
        || newBounds.mType != previousBounds.mType
    ) {
        mComputeQueue.insert(entityID);
    }
}

void SpatialQuerySystem::onSimulationActivated() {
    mRequiresInitialization = true;
}

void SpatialQuerySystem::onPostTransformUpdate(uint32_t timestepMillis) {
    if(mRequiresInitialization)  {
        mComputeQueue.clear();
        rebuildOctree();
        mRequiresInitialization = false;
        return;
    }

    for(EntityID entity: mComputeQueue) {
        mOctree->removeEntity(entity);
        updateBounds(entity);
        mOctree->insertEntity(entity, getComponent<AxisAlignedBounds>(entity));
    }

    mComputeQueue.clear();
}
