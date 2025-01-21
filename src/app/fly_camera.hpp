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

    std::shared_ptr<BaseSimObjectAspect> makeCopy() const override;

    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
protected:
    void onMove(const ActionData& actionData, const ActionDefinition& actionDefinition);
    void onRotate(const ActionData& actionData, const ActionDefinition& actionDefinition);
    void onToggleControl(const ActionData& actionData, const ActionDefinition& actionDefinition);
    void onUpdateFOV(const ActionData& actionData, const ActionDefinition& actionDefinition);

    std::weak_ptr<FixedActionBinding> handlerMove { 
        declareFixedActionBinding(
            "Camera",
            "Move",
            [this](const ActionData& actionData, const ActionDefinition& actionDefinition) {
                this->onMove(actionData, actionDefinition);
            }
        )
    };

    std::weak_ptr<FixedActionBinding> handlerRotate { 
        declareFixedActionBinding(
            "Camera",
            "Rotate",
            [this](const ActionData& actionData, const ActionDefinition& actionDefinition) {
                this->onRotate(actionData, actionDefinition);
            }
        )
    };

    std::weak_ptr<FixedActionBinding> handlerUpdateFOV {
        declareFixedActionBinding(
            "Camera",
            "UpdateFOV",
            [this](const ActionData& actionData, const ActionDefinition& actionDefinition) {
                this->onUpdateFOV(actionData, actionDefinition);
            }
        )
    };
    std::weak_ptr<FixedActionBinding> handlerToggleControl {
        declareFixedActionBinding(
            "Camera",
            "ToggleControl",
            [this](const ActionData& actionData, const ActionDefinition& actionDefinition) {
                this->onToggleControl(actionData, actionDefinition);
            }
        )
    };

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
