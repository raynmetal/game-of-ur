#include "query_click.hpp"
#include "fly_camera.hpp"
#include "interface_pointer_callback.hpp"

ToyMakersEngine::Ray QueryClick::rayFromClickCoordinates(glm::vec2 clickCoordinates) {
    const ToyMakersEngine::CameraProperties cameraProps { getComponent<ToyMakersEngine::CameraProperties>() };
    const ToyMakersEngine::ObjectBounds bounds { getComponent<ToyMakersEngine::ObjectBounds>() };
    const glm::vec3 cameraPosition { bounds.getComputedWorldPosition() };
    const glm::quat cameraOrientation { bounds.getComputedWorldOrientation() };

    ToyMakersEngine::Ray cameraRay { .mLength { cameraProps.mNearFarPlanes.y - cameraProps.mNearFarPlanes.x } };
    glm::vec3 relativeCameraPlaneIntersection;
    switch(cameraProps.mProjectionType) {
        case ToyMakersEngine::CameraProperties::ProjectionType::ORTHOGRAPHIC: {
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

        case ToyMakersEngine::CameraProperties::ProjectionType::FRUSTUM: {
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

    return cameraRay;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> QueryClick::clone() const {
    return std::shared_ptr<QueryClick>(new QueryClick{});
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> QueryClick::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<QueryClick>(new QueryClick{});
}

bool QueryClick::onLeftClick(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    if(
        (actionData.mTwoAxisActionData.mValue.x < 0.f || actionData.mTwoAxisActionData.mValue.y < 0.f)
        || (actionData.mTwoAxisActionData.mValue.x > 1.f || actionData.mTwoAxisActionData.mValue.y > 1.f)
    ) {
        return false;
    }

    const glm::vec2 clickCoordinates {glm::vec2{actionData.mTwoAxisActionData.mValue}};

    bool entityFound { false };
    const ToyMakersEngine::Ray cameraRay { rayFromClickCoordinates(clickCoordinates) };
    for(
        const auto& foundNode:
        getWorld().lock()->getSystem<ToyMakersEngine::SpatialQuerySystem>()->findNodesOverlapping(cameraRay)
    ) {
        entityFound = true;
        std::cout << "- " << foundNode->getViewportLocalPath() << "\n";

        //then click on every aspect of our query results that can be clicked
        if(std::shared_ptr<ToyMakersEngine::SimObject> nodeAsSimObject = std::dynamic_pointer_cast<ToyMakersEngine::SimObject>(foundNode)) {
            if(nodeAsSimObject->hasAspectWithInterface<ILeftClickable>()) {
                for(ILeftClickable& clickable: nodeAsSimObject->getAspectsWithInterface<ILeftClickable>()) {
                    leftClickOn(clickable);
                }
            }
        }

    }
    return entityFound;
}

bool QueryClick::onRightClick(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    if(
        (actionData.mTwoAxisActionData.mValue.x < 0.f || actionData.mTwoAxisActionData.mValue.y < 0.f)
        || (actionData.mTwoAxisActionData.mValue.x > 1.f || actionData.mTwoAxisActionData.mValue.y > 1.f)
    ) {
        return false;
    }

    const glm::vec2 clickCoordinates { glm::vec2{actionData.mTwoAxisActionData.mValue} };

    bool entityFound { false };
    const ToyMakersEngine::Ray cameraRay { rayFromClickCoordinates(clickCoordinates) };
    for(
        const auto& foundNode:
        getWorld().lock()->getSystem<ToyMakersEngine::SpatialQuerySystem>()->findNodesOverlapping(cameraRay)
    ) {
        entityFound = true;
        std::cout << "- " << foundNode->getViewportLocalPath() << "\n";

        //then click on every aspect of our query results that can be clicked
        if(std::shared_ptr<ToyMakersEngine::SimObject> nodeAsSimObject = std::dynamic_pointer_cast<ToyMakersEngine::SimObject>(foundNode)) {
            if(nodeAsSimObject->hasAspectWithInterface<ILeftClickable>()) {
                for(IRightClickable& clickable: nodeAsSimObject->getAspectsWithInterface<IRightClickable>()) {
                    rightClickOn(clickable);
                }
            }
        }
    }

    return entityFound;
}
