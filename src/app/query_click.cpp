#include <algorithm>

#include "../engine/spatial_query_math.hpp"

#include "query_click.hpp"
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
    (void)jsonAspectProperties; // prevent unused parameter warning
    return std::shared_ptr<QueryClick>(new QueryClick{});
}

bool QueryClick::onLeftClick(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    (void)actionDefinition; // prevent unused parameter warning
    if(
        (actionData.mTwoAxisActionData.mValue.x < 0.f || actionData.mTwoAxisActionData.mValue.y < 0.f)
        || (actionData.mTwoAxisActionData.mValue.x > 1.f || actionData.mTwoAxisActionData.mValue.y > 1.f)
    ) {
        return false;
    }

    const glm::vec2 clickCoordinates {glm::vec2{actionData.mTwoAxisActionData.mValue}};

    bool entityFound { false };
    const ToyMakersEngine::Ray cameraRay { rayFromClickCoordinates(clickCoordinates) };
    std::vector<std::shared_ptr<ToyMakersEngine::SceneNodeCore>> currentQueryResults {
        getWorld().lock()->getSystem<ToyMakersEngine::SpatialQuerySystem>()->findNodesOverlapping(cameraRay)
    };
    for(const auto& foundNode: currentQueryResults) {
        entityFound = true;
        std::cout << "- " << foundNode->getViewportLocalPath() << "\n";

        //then click on every aspect of our query results that can be clicked
        if(std::shared_ptr<ToyMakersEngine::SimObject> nodeAsSimObject = std::dynamic_pointer_cast<ToyMakersEngine::SimObject>(foundNode)) {
            if(nodeAsSimObject->hasAspectWithInterface<ILeftClickable>()) {
                glm::vec4 intersectionLocation { ToyMakersEngine::computeIntersections(cameraRay, foundNode->getComponent<ToyMakersEngine::AxisAlignedBounds>()).second.first, 1.f };
                for(ILeftClickable& clickable: nodeAsSimObject->getAspectsWithInterface<ILeftClickable>()) {
                    leftClickOn(clickable, intersectionLocation);
                }
            }
        }

    }
    mPreviousQueryResults = currentQueryResults;
    return entityFound;
}

bool QueryClick::onLeftRelease(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    (void)actionDefinition; // prevent unused parameter warning
    if(
        (actionData.mTwoAxisActionData.mValue.x < 0.f || actionData.mTwoAxisActionData.mValue.y < 0.f)
        || (actionData.mTwoAxisActionData.mValue.x > 1.f || actionData.mTwoAxisActionData.mValue.y > 1.f)
    ) {
        return false;
    }

    const glm::vec2 clickCoordinates {glm::vec2{actionData.mTwoAxisActionData.mValue}};

    bool entityFound { false };
    const ToyMakersEngine::Ray cameraRay { rayFromClickCoordinates(clickCoordinates) };
    std::vector<std::shared_ptr<ToyMakersEngine::SceneNodeCore>> currentQueryResults {
        getWorld().lock()->getSystem<ToyMakersEngine::SpatialQuerySystem>()->findNodesOverlapping(cameraRay)
    };
    for(const auto& foundNode: currentQueryResults) {
        entityFound = true;
        std::cout << "- " << foundNode->getViewportLocalPath() << "\n";

        //then unclick on every aspect of our query results that can be clicked
        if(std::shared_ptr<ToyMakersEngine::SimObject> nodeAsSimObject = std::dynamic_pointer_cast<ToyMakersEngine::SimObject>(foundNode)) {
            if(nodeAsSimObject->hasAspectWithInterface<ILeftClickable>()) {
                glm::vec4 intersectionLocation { ToyMakersEngine::computeIntersections(cameraRay, foundNode->getComponent<ToyMakersEngine::AxisAlignedBounds>()).second.first, 1.f };
                for(ILeftClickable& clickable: nodeAsSimObject->getAspectsWithInterface<ILeftClickable>()) {
                    leftReleaseOn(clickable, intersectionLocation);
                }
            }
        }
    }
    mPreviousQueryResults = currentQueryResults;
    return entityFound;
}

bool QueryClick::onPointerMove(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    (void)actionDefinition; // prevent unused parameter warning
    if(
        (actionData.mTwoAxisActionData.mValue.x < 0.f || actionData.mTwoAxisActionData.mValue.y < 0.f)
        || (actionData.mTwoAxisActionData.mValue.x > 1.f || actionData.mTwoAxisActionData.mValue.y > 1.f)
    ) {
        return false;
    }

    const glm::vec2 clickCoordinates {glm::vec2{actionData.mTwoAxisActionData.mValue}};

    bool entityFound { false };
    const ToyMakersEngine::Ray cameraRay { rayFromClickCoordinates(clickCoordinates) };

    std::vector<std::shared_ptr<ToyMakersEngine::SceneNodeCore>> currentQueryResults {
        getWorld().lock()->getSystem<ToyMakersEngine::SpatialQuerySystem>()->findNodesOverlapping(cameraRay)
    };
    for(const auto& foundNode: currentQueryResults) {
        entityFound = true;
        if(std::shared_ptr<ToyMakersEngine::SimObject> nodeAsSimObject = std::dynamic_pointer_cast<ToyMakersEngine::SimObject>(foundNode)) {
            if(nodeAsSimObject->hasAspectWithInterface<IHoverable>()) {
                const bool isInPreviousQuery { std::find(mPreviousQueryResults.begin(), mPreviousQueryResults.end(), foundNode) != mPreviousQueryResults.end() };
                if(!isInPreviousQuery) {
                    for(IHoverable& hoverableAspect: nodeAsSimObject->getAspectsWithInterface<IHoverable>()) {
                        glm::vec4 intersectionLocation { ToyMakersEngine::computeIntersections(cameraRay, foundNode->getComponent<ToyMakersEngine::AxisAlignedBounds>()).second.first, 1.f };
                        pointerEnter(hoverableAspect, intersectionLocation);
                    }
                }
            }
        }
    }

    // unhover on every aspect of our previous query results that we had been hovering on
    for(const auto& foundNode: mPreviousQueryResults) {
        if(std::shared_ptr<ToyMakersEngine::SimObject> nodeAsSimObject = std::dynamic_pointer_cast<ToyMakersEngine::SimObject>(foundNode)) {
            if(nodeAsSimObject->hasAspectWithInterface<IHoverable>()) {
                const bool isInCurrentQuery { std::find(currentQueryResults.begin(), currentQueryResults.end(), foundNode) != currentQueryResults.end() };
                if(!isInCurrentQuery) {
                    for(IHoverable& hoverableAspect: nodeAsSimObject->getAspectsWithInterface<IHoverable>()) {
                        pointerLeave(hoverableAspect);
                    }
                }
            }
        }
    }
    mPreviousQueryResults = currentQueryResults;

    return entityFound;
}
