#ifndef ZOINPUTSYSTEMDATA_H
#define ZOINPUTSYSTEMDATA_H

#include <set>
#include <map>
#include <string>
#include <functional>

#include <glm/glm.hpp>

struct InputSourceDescription;
enum class DeviceType: uint8_t {
    NA,
    MOUSE,
    KEYBOARD,
    CONTROLLER,
    TOUCH,
};

NLOHMANN_JSON_SERIALIZE_ENUM ( DeviceType, {
    {DeviceType::NA, "na"},
    {DeviceType::MOUSE, "mouse"},
    {DeviceType::KEYBOARD, "keyboard"},
    {DeviceType::TOUCH, "touch"},
    {DeviceType::CONTROLLER, "controller"},
})

enum class ControlType: uint8_t {
    NA,
    AXIS,
    MOTION,
    POINT,
    BUTTON,
    RADIO,
};

NLOHMANN_JSON_SERIALIZE_ENUM(ControlType, {
    {ControlType::NA, "na"},
    {ControlType::AXIS, "axis"},
    {ControlType::MOTION, "motion"},
    {ControlType::POINT, "point"},
    {ControlType::BUTTON, "button"},
    {ControlType::RADIO, "radio"},
})

typedef uint8_t AxisFilterType;
typedef uint8_t InputAttributesValueType;
typedef std::pair<DeviceType, ControlType> InputSourceType;

struct InputAttributesType {
    InputAttributesType() = default;
    InputAttributesType(InputAttributesValueType value) : mValue{value} {}
    operator InputAttributesValueType() const { return mValue; }
    InputAttributesValueType mValue {0};
};

extern const std::map<InputSourceType, InputAttributesType> kInputSourceTypeAttributes;

enum InputAttributes: InputAttributesValueType {
    /* 
     *   Mask for the first two bits containing
     * the number of axes in the value produced by
     * an input device
     */
    N_AXES=0x3,
    HAS_NEGATIVE=0x4, // lines up with the bit representing sign in AxisFilter
    HAS_CHANGE_VALUE=0x8,
    HAS_BUTTON_VALUE=0x10,
    HAS_STATE_VALUE=0x20,
    STATE_IS_LOCATION=0x40,
};

/*   Identifies a single control, such as a button, trigger, or joystick,
 * on a single device
 */
struct InputSourceDescription {
    InputAttributesType mAttributes {0};
    uint8_t mDevice {0};
    uint32_t mControl {0};
    DeviceType mDeviceType {DeviceType::NA};
    ControlType mControlType {ControlType::NA};

    bool operator==(const InputSourceDescription& other) const {
        return !(*this < other) && !(other < *this);
    }
    bool operator<(const InputSourceDescription& other) const {
        return (
            static_cast<uint8_t>(mDeviceType) < static_cast<uint8_t>(other.mDeviceType)
            || (
                mDeviceType == other.mDeviceType && (
                    mDevice < other.mDevice
                    || (
                        mDevice == other.mDevice
                        && (
                            static_cast<uint8_t>(mControlType) < static_cast<uint8_t>(other.mControlType)
                            || mControl < other.mControl
                        )
                    )
                )
            )
        );
    }

    operator bool() const {
        // Must have both a device type and a control type
        // to be considered a valid input source
        return !(
            mDeviceType == DeviceType::NA
            || mControlType == ControlType::NA
        );
    }
};

// Enumeration of all possible axis filter values
enum AxisFilter: AxisFilterType {
                      //     V- lines up with the bit representing sign in actionAttributes
    SIMPLE=0x0,       //   Sign     Index
    X_POS=0x1,        //0b  00       01
    X_NEG=0x5,        //0b  01       01
    Y_POS=0x2,        //0b  00       10
    Y_NEG=0x6,        //0b  01       10
    Z_POS=0x3,        //0b  00       11
    Z_NEG=0x7,        //0b  01       11
    X_CHANGE_POS=0x9, //0b  10       01
    X_CHANGE_NEG=0xD, //0b  11       01
    Y_CHANGE_POS=0xA, //0b  10       10
    Y_CHANGE_NEG=0xE, //0b  11       10
    Z_CHANGE_POS=0xB, //0b  10       11
    Z_CHANGE_NEG=0xF, //0b  11       11
};
NLOHMANN_JSON_SERIALIZE_ENUM( AxisFilter, {
    {AxisFilter::SIMPLE, "simple"},
    {AxisFilter::X_POS, "+x"},
    {AxisFilter::X_NEG, "-x"},
    {AxisFilter::Y_POS, "+y"},
    {AxisFilter::Y_NEG, "-y"},
    {AxisFilter::Z_POS, "+z"},
    {AxisFilter::Z_NEG, "-z"},
    {AxisFilter::X_CHANGE_POS, "+dx"},
    {AxisFilter::X_CHANGE_NEG, "-dx"},
    {AxisFilter::Y_CHANGE_POS, "+dy"},
    {AxisFilter::Y_CHANGE_NEG, "-dy"},
    {AxisFilter::Z_CHANGE_POS, "+dz"},
    {AxisFilter::Z_CHANGE_NEG, "-dz"},
})

enum AxisFilterMask: AxisFilterType {
    ID=0x3,
    SIGN=0x4,
    CHANGE=0x8,
};

struct InputFilter {
    InputSourceDescription mControl {};

    AxisFilter mAxisFilter { 0x0 };

    bool operator==(const InputFilter& other) const {
        return !(*this < other) && !(other < *this);
    }

    bool operator<(const InputFilter& other) const {
        return (
            mControl < other.mControl
            || (
                mControl == other.mControl
                && (
                    mAxisFilter < other.mAxisFilter
                )
            )
        );
    }

    operator bool() const {
        return mControl;
    }
};

/**
 * An input combo whose value ranges from 0..1. Triggered
 * by events of a certain kind, and supports up to two modifiers.
 */
struct InputCombo {
    enum Trigger: uint8_t {
        ON_PRESS,
        ON_RELEASE,
        ON_CHANGE,
    };

    // Axis value may be sampled from here, if present
    InputFilter mMainControl {}; 

    // Always treated as button types
    InputFilter mModifier1 {}; 

    // Always treated as button types
    InputFilter mModifier2 {}; 

    Trigger mTrigger { ON_CHANGE };
    double mDeadzone { 0.f };
    double mThreshold { .7f };

    operator bool() const {
        return mMainControl;
    }

    bool operator==(const InputCombo& other) const {
        return !(*this < other) && !(other < *this);
    }
    bool operator<(const InputCombo& other) const {
        return (
            mMainControl < other.mMainControl
            || (
                mMainControl == other.mMainControl
                && (
                    mModifier1 < other.mModifier1
                    || (
                        mModifier1 == other.mModifier1
                        && (
                            mModifier2 < other.mModifier2
                            || (
                                mModifier2 == other.mModifier2 
                                && (
                                    static_cast<uint8_t>(mTrigger) < static_cast<uint8_t>(other.mTrigger)
                                    || (
                                        mTrigger == other.mTrigger 
                                        && (
                                            mThreshold < other.mThreshold
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            )
        );
    }
};

NLOHMANN_JSON_SERIALIZE_ENUM( InputCombo::Trigger, {
    {InputCombo::Trigger::ON_PRESS, "onPress"},
    {InputCombo::Trigger::ON_RELEASE, "onRelease"},
    {InputCombo::Trigger::ON_CHANGE, "onChange"},
})

/**
 *  An input state that hasn't yet been mapped to its 
 * corresponding action
 */
struct UnmappedInputValue {
    uint32_t mTimestamp {};
    bool mActivated { false };
    double mValue { 0.f };
};

enum class ActionValueType: uint8_t {
    STATE,
    CHANGE,
};

NLOHMANN_JSON_SERIALIZE_ENUM( ActionValueType, {
    {ActionValueType::STATE, "state"},
    {ActionValueType::CHANGE, "change"},
})

/*
 * The definition of a single action
 */
struct ActionDefinition {
    std::string mName {};
    InputAttributesType mAttributes {};
    ActionValueType mValueType {};
    std::string mContext {};

    bool operator==(const ActionDefinition& other) const {
        return !(mName < other.mName) && !(other.mName < mName);
    }
    bool operator<(const ActionDefinition& other) const {
        return mName < other.mName;
    }
};

void to_json(nlohmann::json& json, const InputAttributesType& inputAttributes);
void from_json(const nlohmann::json& json, InputAttributesType& inputAttributes);

void to_json(nlohmann::json& json, const InputSourceDescription& inputSourceDescription);
void from_json(const nlohmann::json& json, InputSourceDescription& inputSourceDescription);

void to_json(nlohmann::json& json, const InputFilter& inputFilter);
void from_json(const nlohmann::json& json, InputFilter& inputFilter);

void to_json(nlohmann::json& json, const InputCombo& inputCombo);
void from_json(const nlohmann::json& json, InputCombo& inputCombo);

void to_json(nlohmann::json& json, const ActionDefinition& actionDefinition);
void from_json(const nlohmann::json& json, ActionDefinition& actionDefinition);

enum class ActionType {
    BUTTON,
    ONE_AXIS,
    TWO_AXIS,
    THREE_AXIS,
};

enum class ActionTrigger: uint8_t {
    UPDATE,
    RESET,
};

struct CommonActionData {
    ActionTrigger mTriggeredBy { ActionTrigger::UPDATE };
    uint32_t mTimestamp {};
    uint32_t mDuration { 0 };
    bool mActivated {false};
    ActionType mType{ ActionType::BUTTON };
};

typedef CommonActionData SimpleActionData;

struct OneAxisActionData {
    CommonActionData mCommonData { .mType{ActionType::ONE_AXIS} };
    double mValue { 0.f };
};

struct TwoAxisActionData {
    CommonActionData mCommonData { .mType {ActionType::TWO_AXIS} };
    glm::dvec2 mValue { 0.f };
};

struct ThreeAxisActionData {
    CommonActionData mCommonData { .mType{ActionType::THREE_AXIS} };
    glm::dvec3 mValue { 0.f };
};

union ActionData {
    static constexpr ActionType toType[4] {
        ActionType::BUTTON, ActionType::ONE_AXIS,
        ActionType::TWO_AXIS, ActionType::THREE_AXIS
    };
    ActionData(uint8_t nAxes): ActionData{ toType[nAxes] } {}
    ActionData(ActionType actionType): mCommonData{ .mType{actionType} } {
        // Regardless of the type, all the data that corresponds
        // to the action value should be initialized with 0
        mThreeAxisActionData.mValue = glm::vec3{0.f};
    }
    ActionData(): ActionData{ ActionType::BUTTON }  {}
    ActionData(SimpleActionData simpleData): ActionData{ActionType::BUTTON} {
        mSimpleData = simpleData;
    }
    ActionData(OneAxisActionData oneAxisActionData): ActionData{ActionType::ONE_AXIS} {
        mOneAxisActionData = oneAxisActionData;
    }
    ActionData(TwoAxisActionData twoAxisActionData): ActionData{ActionType::TWO_AXIS} {
        mTwoAxisActionData = twoAxisActionData;
    }
    ActionData(ThreeAxisActionData threeAxisActionData): ActionData{ActionType::THREE_AXIS} {
        mThreeAxisActionData = threeAxisActionData;
    }

    CommonActionData mCommonData;
    SimpleActionData mSimpleData;
    OneAxisActionData mOneAxisActionData;
    TwoAxisActionData mTwoAxisActionData;
    ThreeAxisActionData mThreeAxisActionData;
};

namespace std {
    template<>
    struct hash<InputSourceDescription> {
        size_t operator() (const InputSourceDescription& definition) const {
            return (
                (( (hash<uint32_t>{}(definition.mControl))
                ^ (hash<uint8_t>{}(definition.mDevice) << 1) >> 1)
                ^ (hash<uint8_t>{}(static_cast<uint8_t>(definition.mDeviceType)) << 1) >> 1)
                ^ (hash<uint8_t>{}(static_cast<uint8_t>(definition.mControlType) << 1))
            );
        }
    };

    template<>
    struct hash<ActionDefinition> {
        size_t operator() (const ActionDefinition& definition) const {
            return hash<std::string>{}(definition.mName);
        }
    };

    template<>
    struct hash<InputFilter> {
        size_t operator() (const InputFilter& inputFilter) const {
            return (
                (hash<InputSourceDescription>{}(inputFilter.mControl)
                ^ (hash<uint8_t>{}(inputFilter.mAxisFilter) << 1))
            );
        }
    };

    template<>
    struct hash<InputCombo> {
        size_t operator() (const InputCombo& inputBind) const {
            return (
                (((hash<InputFilter>{}(inputBind.mMainControl)
                ^ (hash<InputFilter>{}(inputBind.mModifier1) << 1) >> 1)
                ^ (hash<InputFilter>{}(inputBind.mModifier2) << 1) >> 1)
                ^ (hash<InputCombo::Trigger>{}(inputBind.mTrigger) << 1) >> 1)
                ^ (hash<float>{}(inputBind.mThreshold) << 1)
            );
        }
    };
}

#endif
