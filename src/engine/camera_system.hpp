#ifndef FOOLSENGINE_CAMERASYSTEM_H
#define FOOLSENGINE_CAMERASYSTEM_H

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "core/ecs_world.hpp"
#include "scene_components.hpp"

namespace ToyMaker {

    struct CameraProperties {
        enum class ProjectionType: uint8_t {
            FRUSTUM,
            ORTHOGRAPHIC,
        };
        ProjectionType mProjectionType { ProjectionType::FRUSTUM };
        float mFov {45.f};
        float mAspect { 16.f/9.f };
        glm::vec2 mOrthographicDimensions { 19.f, 12.f };
        glm::vec2 mNearFarPlanes { 100.f, -100.f };
        glm::mat4 mProjectionMatrix {};
        glm::mat4 mViewMatrix {};

        inline static std::string getComponentTypeName() { return "CameraProperties"; }
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(CameraProperties::ProjectionType, {
        {CameraProperties::ProjectionType::FRUSTUM, "frustum"},
        {CameraProperties::ProjectionType::ORTHOGRAPHIC, "orthographic"},
    });

    class CameraSystem: public System<CameraSystem, std::tuple<Transform, CameraProperties>, std::tuple<>> {
    public:
        explicit CameraSystem(std::weak_ptr<ECSWorld> world):
        System<CameraSystem, std::tuple<Transform, CameraProperties>, std::tuple<>>{world}
        {}
        void updateActiveCameraMatrices();
        static std::string getSystemTypeName() { return "CameraSystem"; }
    private:
        void onEntityEnabled(EntityID entityID) override;
        void onEntityDisabled(EntityID entityID) override;
        void onEntityUpdated(EntityID entityID) override;
        void onSimulationActivated() override;
        void onPreRenderStep(float simulationProgress) override;

        std::set<EntityID> mProjectionUpdateQueue {};
        std::set<EntityID> mViewUpdateQueue {};
    };

    template<>
    inline CameraProperties Interpolator<CameraProperties>::operator() (
        const CameraProperties& previousState, const CameraProperties& nextState,
        float simulationProgress
    ) const {
        simulationProgress = mProgressLimits(simulationProgress);
        return {
            .mProjectionType {previousState.mProjectionType
            },
            .mFov { simulationProgress * nextState.mFov + (1.f-simulationProgress) * previousState.mFov },
            .mAspect { simulationProgress * nextState.mAspect + (1.f-simulationProgress) * previousState.mAspect},
            .mOrthographicDimensions { 
                (simulationProgress * nextState.mOrthographicDimensions)
                + (1.f-simulationProgress) * previousState.mOrthographicDimensions
            },
            .mNearFarPlanes {
                (simulationProgress * nextState.mNearFarPlanes)
                + (1.f-simulationProgress) * previousState.mNearFarPlanes
            },
            .mProjectionMatrix {
                (simulationProgress * nextState.mProjectionMatrix) 
                + ((1.f-simulationProgress) * previousState.mProjectionMatrix)
            },
            .mViewMatrix {
                (simulationProgress * nextState.mViewMatrix)
                + ((1.f-simulationProgress) * previousState.mViewMatrix)
            },
        };
    }

    inline void from_json(const nlohmann::json& json, CameraProperties& cameraProperties) {
        assert(json.at("type").get<std::string>() == CameraProperties::getComponentTypeName() && "Type mismatch, json must be of camera properties type");
        json.at("projection_mode").get_to(cameraProperties.mProjectionType);
        json.at("fov").get_to(cameraProperties.mFov);
        json.at("aspect").get_to(cameraProperties.mAspect);
        json.at("orthographic_dimensions")
            .at("horizontal")
            .get_to(cameraProperties.mOrthographicDimensions.x);
        json.at("orthographic_dimensions")
            .at("vertical")
            .get_to(cameraProperties.mOrthographicDimensions.y);
        json.at("near_far_planes").at("near").get_to(cameraProperties.mNearFarPlanes.x);
        json.at("near_far_planes").at("far").get_to(cameraProperties.mNearFarPlanes.y);
    }

    inline void to_json(nlohmann::json& json, const CameraProperties& cameraProperties) {
        json = {
            {"type", CameraProperties::getComponentTypeName()},
            {"projection_mode", cameraProperties.mProjectionType},
            {"fov", cameraProperties.mFov},
            {"aspect", cameraProperties.mAspect},
            {"orthographic_dimensions", {
                {"horizontal", cameraProperties.mOrthographicDimensions.x},
                {"vertical", cameraProperties.mOrthographicDimensions.y},
            }},
            {"near_far_planes", {
                {"near", cameraProperties.mNearFarPlanes.x},
                {"far", cameraProperties.mNearFarPlanes.y},
            }}
        };
    }

}
#endif
