#include "query_click.hpp"
#include "fly_camera.hpp"

std::shared_ptr<BaseSimObjectAspect> QueryClick::clone() const {
    return std::shared_ptr<QueryClick>(new QueryClick{});
}

std::shared_ptr<BaseSimObjectAspect> QueryClick::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<QueryClick>(new QueryClick{});
}

bool QueryClick::onClick(const ActionData& actionData, const ActionDefinition& actionDefinition) {
    if(
        (actionData.mTwoAxisActionData.mValue.x < 0.f || actionData.mTwoAxisActionData.mValue.y < 0.f)
        || (actionData.mTwoAxisActionData.mValue.x > 1.f || actionData.mTwoAxisActionData.mValue.y > 1.f)
    ) {
        return false;
    }

    const glm::vec2 clickCoordinates {
        (!hasAspect<FlyCamera>() || !getAspect<FlyCamera>().isMouseActive())?
            glm::vec2{actionData.mTwoAxisActionData.mValue}:
            glm::vec2{.5f, .5f}
    };

    bool entityFound { false };
    const CameraProperties cameraProps { getComponent<CameraProperties>() };
    const ObjectBounds bounds { getComponent<ObjectBounds>() };
    const glm::vec3 cameraPosition { bounds.getComputedWorldPosition() };
    const glm::quat cameraOrientation { bounds.getComputedWorldOrientation() };

    Ray cameraRay { .mLength { cameraProps.mNearFarPlanes.y - cameraProps.mNearFarPlanes.x } };
    glm::vec3 relativeCameraPlaneIntersection;
    switch(cameraProps.mProjectionType) {
        case CameraProperties::ProjectionType::ORTHOGRAPHIC: {
            relativeCameraPlaneIntersection = {
                (
                    cameraProps.mOrthographicDimensions 
                    * (
                        glm::vec2{1.f, -1.f} * clickCoordinates 
                        + glm::vec2{ -.5f, .5f }
                    )
                ),
                -cameraProps.mNearFarPlanes.x
            };

            cameraRay.mDirection = cameraOrientation * glm::vec3{0.f, 0.f, -1.f};
        }
        break;

        case CameraProperties::ProjectionType::FRUSTUM: {
            const float twoTanFovByTwo { 2.f * glm::tan(glm::radians(cameraProps.mFov/2.f)) };
            relativeCameraPlaneIntersection = {
                (
                    glm::vec2 { cameraProps.mAspect*twoTanFovByTwo*.5f, twoTanFovByTwo*.5f }
                    * (
                        glm::vec2 { -.5f, .5f }
                        + glm::vec2{ 1.f, -1.f }
                        * clickCoordinates 
                    )
                ),
                -cameraProps.mNearFarPlanes.x
            };
            cameraRay.mDirection = cameraOrientation * glm::normalize(relativeCameraPlaneIntersection);
        }
        break;
    };

    cameraRay.mStart = cameraPosition + cameraOrientation * relativeCameraPlaneIntersection;

    for(
        const auto& foundNode:
        getWorld().lock()->getSystem<SpatialQuerySystem>()->findNodesOverlapping(cameraRay)
    ) {
        entityFound = true;
        std::cout << "- " << foundNode->getViewportLocalPath() << "\n";
    }

    return entityFound;
}
