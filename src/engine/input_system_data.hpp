#ifndef ZOINPUTSYSTEMDATA_H
#define ZOINPUTSYSTEMDATA_H

#include <set>
#include <map>
#include <string>
#include <functional>

#include <glm/glm.hpp>

enum DeviceType: uint8_t {
    NA,
    MOUSE,
    KEYBOARD,
    CONTROLLER,
    TOUCH,
};

enum ControlType: uint8_t {
    NA,
    AXIS,
    MOTION,
    POINT,
    BUTTON,
    RADIO,
};

typedef uint8_t ActionAttributesType;

enum ActionAttributes: ActionAttributesType {
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
struct InputIdentity {
    ActionAttributesType mAttributes {0};
    uint8_t mDevice {0};
    uint32_t mControl {0};
    DeviceType mDeviceType {NA};
    ControlType mControlType {NA};

    bool operator==(const InputIdentity& other) const {
        return (
            mDevice == other.mDevice 
            && mControl == other.mControl
            && mDeviceType == other.mDeviceType
            && mControlType == other.mControlType
        );
    }

    operator bool() const {
        return !(
            mDeviceType == DeviceType::NA
            || mControlType == ControlType::NA
        );
    }
};

/*
 * The definition of a single action
 */
struct ActionDefinition {
    enum Trigger: uint8_t {
        ON_PRESS,
        ON_RELEASE,
        ON_CHANGE,
    };

    std::string mName {};
    ActionAttributes mAttributes {};
    Trigger mTrigger { ON_CHANGE };

    bool operator==(const ActionDefinition& other) const {
        return mName == other.mName;
    }
};

typedef uint8_t AxisFilterType;

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

enum AxisFilterMask: AxisFilterType {
    ID=0x3,
    SIGN=0x4,
    CHANGE=0x8,
};

struct InputFilter {
    InputIdentity mControl {};

    AxisFilterType mAxisFilter { 0x0 };

    operator bool() const {
        return mControl;
    }

    bool operator==(const InputFilter& other) const {
        return other.mControl == mControl
            && other.mAxisFilter == other.mAxisFilter;
    }
};

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

    bool mInverted { false };
    Trigger mTrigger { ON_CHANGE };
    double mDeadzone { 0.f };
    double mThreshold { .7f };

    operator bool() const {
        return mMainControl;
    }

    bool operator==(const InputCombo& other) const {
        return mMainControl == other.mMainControl
            && mModifier1 == other.mModifier1
            && mModifier2 == other.mModifier2
            && mTrigger == other.mTrigger
            && mInverted == other.mInverted
            && mThreshold == other.mThreshold;
    }
};

struct UnmappedInputValue {
    uint32_t mTimestamp {};
    bool mActivated { false };
    double mChangeValue { 0.f };
    double mStateValue { 0.f };
};

enum ActionType {
    SIMPLE,
    MULTIAXIS,
};

struct CommonActionData {
    uint32_t mTimestamp {};
    uint32_t mDuration { 0 };
    bool mActivated {false};
    ActionType mType;
};

struct SimpleActionData {
    CommonActionData mCommonData { .mType{SIMPLE} };
    double mChangeValue { 0.f };
    double mStateValue { 0.f };
};

struct MultiAxisActionData {
    CommonActionData mCommonData { .mType{MULTIAXIS} };
    glm::vec3 mChangeValue { 0.f };
    glm::vec3 mStateValue { 0.f };
};

union ActionData {
    ActionData(SimpleActionData simpleData): mSimpleData { simpleData } {};
    ActionData(MultiAxisActionData multiAxisData): mMultiAxisData{ multiAxisData } {};

    ActionData(const ActionData& actionData) = default;
    ActionData(ActionData&& actionData) = default;
    ActionData& operator=(const ActionData& actionData) = default;
    ActionData& operator=(ActionData&& actionData) = default;
    
    CommonActionData mCommonData;
    SimpleActionData mSimpleData;
    MultiAxisActionData mMultiAxisData;
};

namespace std {
    template<>
    struct std::hash<InputIdentity> {
        std::size_t operator() (const InputIdentity& definition) {
            return (
                ( (hash<uint32_t>{}(definition.mControl))
                ^ (hash<uint8_t>{}(definition.mDevice) << 1) >> 1)
                ^ (hash<uint8_t>{}(definition.mDeviceType) << 1)
            );
        }
    };

    template<>
    struct std::hash<ActionDefinition> {
        std::size_t operator() (const ActionDefinition& definition) {
            return hash<std::string>{}(definition.mName);
        }
    };

    template<>
    struct std::hash<InputFilter> {
        std::size_t operator() (const InputFilter& inputFilter) {
            return (
                (std::hash<InputIdentity>{}(inputFilter.mControl)
                ^ (std::hash<uint8_t>{}(inputFilter.mAxisFilter) << 1))
            );
        }
    };

    template<>
    struct std::hash<InputCombo> {
        std::size_t operator() (const InputCombo& inputBind) {
            return (
                ((std::hash<InputFilter>{}(inputBind.mMainControl)
                ^ (std::hash<InputFilter>{}(inputBind.mModifier1) << 1) >> 1)
                ^ (std::hash<InputFilter>{}(inputBind.mModifier2) << 1) >> 1)
                ^ (std::hash<InputCombo::Trigger>{}(inputBind.mTrigger) << 1)
            );
        }
    };
}

#endif
