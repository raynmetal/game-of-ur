#include <limits>

#include "spatial_query_system.hpp"

void SpatialQuerySystem::StaticModelBoundsComputeSystem::onEntityEnabled(EntityID entityID) {
    ObjectBounds objectBounds { getComponent<ObjectBounds>(entityID) };
    if(!objectBounds.mRequiresCompute) return;

    std::shared_ptr<StaticModel> model { getComponent<std::shared_ptr<StaticModel>>(entityID) };
    glm::vec3 minDimensionValues {std::numeric_limits<float>::max()};
    glm::vec3 maxDimensionValues {std::numeric_limits<float>::min()};

    for(auto meshHandle: model->getMeshHandles()) {
        assert(meshHandle->getVertexListBegin() != meshHandle->getVertexListEnd() && "Cannot compute bounding volume of an empty mesh");
        for(
            auto vertexIter { meshHandle->getVertexListBegin()};
            vertexIter != meshHandle->getVertexListEnd();
            ++vertexIter
        ) {
            minDimensionValues = {
                std::min(vertexIter->mPosition.x, minDimensionValues.x),
                std::min(vertexIter->mPosition.y, minDimensionValues.y),
                std::min(vertexIter->mPosition.z, minDimensionValues.z),
            };
            maxDimensionValues = {
                std::max(vertexIter->mPosition.x, maxDimensionValues.x),
                std::max(vertexIter->mPosition.y, maxDimensionValues.y),
                std::max(vertexIter->mPosition.z, maxDimensionValues.z)
            };
        }
    }

    VolumeBox box {
        .mDimensions{ 
            maxDimensionValues.x - minDimensionValues.x,
            maxDimensionValues.y - minDimensionValues.y,
            maxDimensionValues.z - minDimensionValues.z,
        }
    };
    objectBounds = {
        .mType { ObjectBounds::Type::BOX },
        .mPositionOffset {
            minDimensionValues.x + box.mDimensions.x/2.f,
            minDimensionValues.y + box.mDimensions.y/2.f,
            minDimensionValues.z + box.mDimensions.z/2.f,
        },
        .mDirectionOffset { glm::vec3{0.f} },
        .mOrientedBoxDimensions { box },
        .mTrueVolume { box }
    };
    objectBounds.mRequiresCompute = false;

    updateComponent<ObjectBounds>(entityID, objectBounds);
}

void SpatialQuerySystem::LightBoundsComputeSystem::onEntityEnabled(EntityID entityID) {
    ObjectBounds objectBounds { getComponent<ObjectBounds>(entityID) };
    if(!objectBounds.mRequiresCompute) return;

    LightEmissionData lightEmissionData { getComponent<LightEmissionData>(entityID) };
    glm::vec3 minDimensionValues {std::numeric_limits<float>::max()};
    glm::vec3 maxDimensionValues {std::numeric_limits<float>::min()};
    switch(lightEmissionData.mType) {
        case LightEmissionData::LightType::point:
        case LightEmissionData::LightType::spot:
        break;
        case LightEmissionData::LightType::directional:
            std::swap(minDimensionValues, maxDimensionValues);
        break;
    };

    const VolumeSphere sphere {
        lightEmissionData.mType == LightEmissionData::LightType::directional?
        std::numeric_limits<float>::infinity():
        lightEmissionData.mRadius
    };
    const VolumeBox box { .mDimensions { glm::vec3 {sphere.mRadius} } };
    objectBounds = ObjectBounds {
        .mType {ObjectBounds::Type::SPHERE},
        .mRequiresCompute { false },
        .mPositionOffset {glm::vec3{0.f}},
        .mDirectionOffset {glm::vec3{0.f}},
    };
    objectBounds.mOrientedBoxDimensions = box;
    objectBounds.mTrueVolume.mSphere = sphere;

    updateComponent<ObjectBounds>(entityID, objectBounds);
}
