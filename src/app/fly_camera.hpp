#ifndef ZOAPPFLYCAMERA_H
#define ZOAPPFLYCAMERA_H

#include "../engine/camera_system.hpp"
#include "../engine/sim_system.hpp"
#include "../engine/input_system/input_system.hpp"

extern const float DEFAULT_CAMERA_SPEED;
extern const float LOOK_SENSITIVITY;

class FlyCamera: public SimObjectAspect<FlyCamera> {
public:
    inline static std::string getSimObjectAspectTypeName() { return "FlyCamera"; }
    void update(uint32_t deltaSimtimeMillis) override;
    void handleAction(const ActionData& actionData, const ActionDefinition& actionDefinition) override;
    std::shared_ptr<BaseSimObjectAspect> makeCopy() const override;

    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);

private:
    FlyCamera(): SimObjectAspect<FlyCamera>{0} {}
    void updatePitch(glm::quat& current, float dPitch);
    void updateYaw(glm::quat& current, float dYaw);
    void updateFOV(float dFOV);
    void setActive(bool active);

    bool mActive { false };
    float mMaxSpeed { DEFAULT_CAMERA_SPEED };
    glm::vec3 mVelocity { 0.f };
    float mLookSensitivity { LOOK_SENSITIVITY };
    float mZoomSensitivity { 1.5f };
};

#endif
