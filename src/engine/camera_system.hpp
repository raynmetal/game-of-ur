#ifndef ZOCAMERASYSTEM_H
#define ZOCAMERASYSTEM_H

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "ecs_world.hpp"
#include "apploop_events.hpp"

struct CameraProperties {
    enum class ProjectionType: uint8_t {
        FRUSTUM,
        ORTHOGRAPHIC,
    };
    ProjectionType mProjectionType { ProjectionType::FRUSTUM };
    double mFov {45.f};
    double mOrthographicScale {3.f};
    glm::mat4 mProjectionMatrix {};
    glm::mat4 mViewMatrix {};

    inline static std::string getComponentTypeName() { return "CameraProperties"; }
};

NLOHMANN_JSON_SERIALIZE_ENUM(CameraProperties::ProjectionType, {
    {CameraProperties::ProjectionType::FRUSTUM, "frustum"},
    {CameraProperties::ProjectionType::ORTHOGRAPHIC, "orthographic"},
})

class CameraSystem: public System<CameraSystem, Transform, CameraProperties> {
public:
    CameraSystem(ECSWorld& world):
    System<CameraSystem, Transform, CameraProperties>{world}
    {}
    void updateActiveCameraMatrices();
    void onEntityUpdated(EntityID entityID) override;

    static std::string getSystemTypeName() { return "CameraSystem"; }
private:
    class ApploopEventHandler : public IApploopEventHandler<ApploopEventHandler> {
    public:
        inline void initializeEventHandler(CameraSystem* pSystem)  { mSystem = pSystem; }
    private:
        void onPreRenderStep(float simulationProgress) override;
        void onApplicationStart() override;
        CameraSystem* mSystem;
    friend class CameraSystem;
    };

    std::shared_ptr<ApploopEventHandler> mApploopEventHandler { ApploopEventHandler::registerHandler(this) };
    std::set<EntityID> mProjectionUpdateQueue {};
    std::set<EntityID> mViewUpdateQueue {};
friend class CameraSystem::ApploopEventHandler;
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
        .mOrthographicScale { simulationProgress * nextState.mOrthographicScale 
            + (1.f-simulationProgress) * previousState.mOrthographicScale
        },
        .mProjectionMatrix {(simulationProgress * nextState.mProjectionMatrix) 
            + ((1.f-simulationProgress) * previousState.mProjectionMatrix)
        },
        .mViewMatrix {(simulationProgress * nextState.mViewMatrix)
            + ((1.f-simulationProgress) * previousState.mViewMatrix)
        },
    };
}

inline void from_json(const nlohmann::json& json, CameraProperties& cameraProperties) {
    assert(json.at("type").get<std::string>() == CameraProperties::getComponentTypeName() && "Type mismatch, json must be of camera properties type");
    json.at("projectionMode").get_to(cameraProperties.mProjectionType);
    json.at("fov").get_to(cameraProperties.mFov);
    json.at("orthographicScale").get_to(cameraProperties.mOrthographicScale);
}

inline void to_json(nlohmann::json& json, const CameraProperties& cameraProperties) {
    json = {
        {"type", CameraProperties::getComponentTypeName()},
        {"projectionMode", cameraProperties.mProjectionType},
        {"fov", cameraProperties.mFov},
        {"orthographicScale", cameraProperties.mOrthographicScale},
    };
}

#endif
