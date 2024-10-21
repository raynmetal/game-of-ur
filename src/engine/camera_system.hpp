#ifndef ZOCAMERASYSTEM_H
#define ZOCAMERASYSTEM_H

#include <glm/glm.hpp>

#include "simple_ecs.hpp"
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
};

class CameraSystem: public System<CameraSystem> {
public:
    void updateActiveCameraMatrices();
    void onEntityUpdated(EntityID entityID) override;
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


#endif
