#include <cassert>
#include <algorithm>
#include <iostream>

#include <SDL2/SDL.h>

#include "../util.hpp"
#include "../window_context_manager.hpp"
#include "input_system.hpp"

const std::map<InputSourceType, InputAttributesType> kInputSourceTypeAttributes {
    {{DeviceType::MOUSE, ControlType::POINT}, {
        (2&InputAttributes::N_AXES)
        | InputAttributes::HAS_STATE_VALUE
        | InputAttributes::HAS_CHANGE_VALUE
        | InputAttributes::STATE_IS_LOCATION
    }},
    {{DeviceType::MOUSE, ControlType::BUTTON}, {
        (2&InputAttributes::N_AXES)
        | InputAttributes::HAS_BUTTON_VALUE
        | InputAttributes::HAS_STATE_VALUE
        | InputAttributes::STATE_IS_LOCATION
    }},
    {{DeviceType::MOUSE, ControlType::MOTION}, {
        (2&InputAttributes::N_AXES)
        | InputAttributes::HAS_CHANGE_VALUE
    }},
    {{DeviceType::KEYBOARD, ControlType::BUTTON}, {
        InputAttributes::HAS_BUTTON_VALUE
    }},
    {{DeviceType::TOUCH, ControlType::POINT}, {
        (2&InputAttributes::N_AXES)
        | InputAttributes::HAS_BUTTON_VALUE
        | InputAttributes::HAS_STATE_VALUE
        | InputAttributes::HAS_CHANGE_VALUE
        | InputAttributes::STATE_IS_LOCATION
    }},
    {{DeviceType::CONTROLLER, ControlType::AXIS}, {
        (1&InputAttributes::N_AXES)
        | InputAttributes::HAS_STATE_VALUE
        | InputAttributes::HAS_NEGATIVE
    }},
    {{DeviceType::CONTROLLER, ControlType::RADIO},{
        (2&InputAttributes::N_AXES)
        | InputAttributes::HAS_STATE_VALUE
        | InputAttributes::HAS_NEGATIVE
    }},
    {{DeviceType::CONTROLLER, ControlType::MOTION},{
        (2&InputAttributes::N_AXES)
        | InputAttributes::HAS_CHANGE_VALUE
    }},
    {{DeviceType::CONTROLLER, ControlType::BUTTON}, {
        InputAttributes::HAS_BUTTON_VALUE
    }},
    {{DeviceType::CONTROLLER, ControlType::POINT}, {
        (2&InputAttributes::N_AXES)
        | InputAttributes::HAS_BUTTON_VALUE
        | InputAttributes::HAS_STATE_VALUE
        | InputAttributes::STATE_IS_LOCATION
    }},
    {{DeviceType::NA, ControlType::NA}, 0}
};

const std::map<DeviceType, std::string> kDeviceTypeToString {
    {DeviceType::CONTROLLER, "controller"},
    {DeviceType::KEYBOARD, "keyboard"},
    {DeviceType::MOUSE, "mouse"},
    {DeviceType::TOUCH, "touch"},
    {DeviceType::NA, "na"},
};
const std::map<std::string, DeviceType> kStringToDeviceType {
    {"controller", DeviceType::CONTROLLER},
    {"keyboard", DeviceType::KEYBOARD},
    {"mouse", DeviceType::MOUSE},
    {"touch", DeviceType::TOUCH},
    {"na", DeviceType::NA},
};
const std::map<ControlType, std::string> kControlTypeToString {
    {ControlType::AXIS, "axis"},
    {ControlType::BUTTON, "button"},
    {ControlType::MOTION, "motion"},
    {ControlType::POINT, "point"},
    {ControlType::RADIO, "radio"},
    {ControlType::NA, "na"},
};
const std::map<std::string, ControlType> kStringToControlType {
    {"axis", ControlType::AXIS},
    {"button", ControlType::BUTTON},
    {"motion", ControlType::MOTION},
    {"point", ControlType::POINT},
    {"radio", ControlType::RADIO},
    {"na", ControlType::NA}
};
const std::map<std::string, AxisFilterType> kStringToAxisFilter {
    {"simple", AxisFilter::SIMPLE},
    {"+x", AxisFilter::X_POS},
    {"-x", AxisFilter::X_NEG},
    {"+y", AxisFilter::Y_POS},
    {"-y", AxisFilter::Y_NEG},
    {"+z", AxisFilter::Z_POS},
    {"-z", AxisFilter::Z_NEG},
    {"+dx", AxisFilter::X_CHANGE_POS},
    {"-dx", AxisFilter::X_CHANGE_NEG},
    {"+dy", AxisFilter::Y_CHANGE_POS},
    {"-dy", AxisFilter::Y_CHANGE_NEG},
    {"+dz", AxisFilter::Z_CHANGE_POS},
    {"-dz", AxisFilter::Z_CHANGE_NEG},
};
const std::map<AxisFilterType, std::string> kAxisFilterToString {
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
};
const std::map<std::string, InputCombo::Trigger> kStringToInputComboTrigger {
    {"onPress", InputCombo::Trigger::ON_PRESS},
    {"onRelease", InputCombo::Trigger::ON_RELEASE},
    {"onChange", InputCombo::Trigger::ON_CHANGE},
};
const std::map<InputCombo::Trigger, std::string> kInputComboTriggerToString {
    {InputCombo::Trigger::ON_PRESS, "onPress"},
    {InputCombo::Trigger::ON_RELEASE, "onRelease"},
    {InputCombo::Trigger::ON_CHANGE, "onChange"},
};
const std::map<std::string, ActionValueType> kStringToActionValueType {
    {"state", ActionValueType::STATE},
    {"change", ActionValueType::CHANGE},
};
const std::map<ActionValueType, std::string> kActionValueTypeToString {
    {ActionValueType::STATE, "state"},
    {ActionValueType::CHANGE, "change"},
};

bool hasValue(const InputSourceDescription& input, const AxisFilterType filter) {
    uint8_t axisID { static_cast<uint8_t>(filter&AxisFilterMask::ID) };
    uint8_t axisSign { static_cast<uint8_t>(filter&AxisFilterMask::SIGN) };
    uint8_t axisDelta { static_cast<uint8_t>(filter&AxisFilterMask::CHANGE) };

    return (
        input
        && (
            // No axis specified, so in other words it's a button
            (axisID == AxisFilter::SIMPLE && input.mAttributes&HAS_BUTTON_VALUE)

            // An axis is specified, so check whether the desired value
            // supports negatives or represents a change
            || (
                axisID > 0
                && (axisID <= (input.mAttributes&N_AXES))
                && (
                    (axisSign <= (input.mAttributes&HAS_NEGATIVE))
                    || (axisDelta <= (input.mAttributes&HAS_CHANGE_VALUE))
                )
            )
        )
    );
}

bool isValid(const InputFilter& inputFilter) {
    return hasValue(inputFilter.mControl, inputFilter.mAxisFilter);
}

double InputManager::getRawValue(const InputFilter& inputFilter, const SDL_Event& inputEvent) const {
    assert(isValid(inputFilter) && "This is an empty input filter, and does not map to any input value");

    double value {0.f};

    int windowWidth, windowHeight;
    SDL_GetWindowSize(WindowContextManager::getInstance().getSDLWindow(), &windowWidth, &windowHeight);

    switch(inputFilter.mControl.mDeviceType) {

        // Extract raw value from mouse events
        case DeviceType::MOUSE:
            switch(inputFilter.mControl.mControlType) {
                case ControlType::BUTTON:
                    switch(inputFilter.mAxisFilter) {
                        case AxisFilter::SIMPLE:
                            value = inputEvent.button.state;
                        break;
                        case AxisFilter::X_POS:
                            value = RangeMapperLinear{0.f, static_cast<double>(windowWidth), 0.f, 1.f}(inputEvent.button.x);
                        break;
                        case AxisFilter::Y_POS:
                            value = RangeMapperLinear{0.f, static_cast<double>(windowHeight), 0.f, 1.f}(inputEvent.button.y);
                        break;

                        default:
                            assert(false && "Not a valid mouse event, this line should never be reached");
                        break;
                    }
                case ControlType::POINT:
                    switch(inputFilter.mAxisFilter) {
                        // Mouse location filter
                        case X_POS:
                            value = RangeMapperLinear{0.f, static_cast<double>(windowWidth), 0.f, 1.f}(inputEvent.motion.x);
                        break;
                        case Y_POS:
                            value = RangeMapperLinear{0.f, static_cast<double>(windowHeight), 0.f, 1.f}(inputEvent.motion.y);
                        break;
                        // Mouse movement filter
                        case X_CHANGE_POS:
                        case X_CHANGE_NEG:
                        {
                            const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                            value = RangeMapperLinear{0.f, static_cast<double>(windowWidth), 0.f, 1.f}(sign * inputEvent.motion.xrel);
                        }
                        break;
                        case Y_CHANGE_POS:
                        case Y_CHANGE_NEG:
                        {
                            const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                            value = RangeMapperLinear{0.f, static_cast<double>(windowHeight), 0.f, 1.f}(sign * inputEvent.motion.yrel);
                        }
                        break;

                        // Everything else is unsupported
                        default:
                            assert(false && "Not a valid mouse event, this line should never be reached");
                        break;
                    }
                break;
                // mouse wheel filters
                case ControlType::MOTION:
                    switch(inputFilter.mAxisFilter) {
                        case X_CHANGE_POS:
                        case X_CHANGE_NEG: {
                            const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                            value = RangeMapperLinear{0.f, 1.f, 0.f, 1.f}(sign * inputEvent.wheel.x);
                        }
                        break;
                        case Y_CHANGE_POS:
                        case Y_CHANGE_NEG: {
                            const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                            value = RangeMapperLinear{0.f, 1.f, 0.f, 1.f}(sign * inputEvent.wheel.y);
                        }
                        break;
                    }
                break;
                default:
                    assert(false && "Invalid control type for mouse");
                break;
            }
        break;

        // Extract raw value from keyboard events
        case DeviceType::KEYBOARD: {
           assert(inputFilter.mAxisFilter == AxisFilter::SIMPLE && "Invalid keyboard axis filter, keyboards only support `AxisFilter::SIMPLE`");
            value = inputEvent.key.state;
        }
        break;

        // Extract raw value from controller events
        case DeviceType::CONTROLLER: {
            switch(inputFilter.mControl.mControlType) {
                // controller touchpad events
                case ControlType::POINT:
                    switch(inputFilter.mAxisFilter) {
                        case AxisFilter::SIMPLE:
                            value = inputEvent.ctouchpad.pressure;
                        break;
                        case AxisFilter::X_POS:
                            value = inputEvent.ctouchpad.x;
                        break;
                        case AxisFilter::Y_POS:
                            value = inputEvent.ctouchpad.y;
                        break;
                        default:
                            assert(false && "This device control does not support this type of value");
                        break;
                    }
                break;

                case ControlType::BUTTON:
                    assert(inputFilter.mAxisFilter == AxisFilter::SIMPLE);
                    value = inputEvent.cbutton.state;
                break;

                case ControlType::AXIS:
                    switch(inputFilter.mAxisFilter) {
                        case X_POS:
                        case X_NEG: {
                            const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f};
                            value = RangeMapperLinear{0.f, 32768.f, 0.f, 1.f} (sign * inputEvent.caxis.value);
                        }
                        break;
                        default:
                            assert(false && "Invalid filter type for this device and control");
                        break;
                    }
                break;

                case ControlType::RADIO:
                    switch(inputFilter.mAxisFilter) {
                        case AxisFilter::X_POS:
                            switch(inputEvent.jhat.value) {
                                case SDL_HAT_RIGHT:
                                case SDL_HAT_RIGHTDOWN:
                                case SDL_HAT_RIGHTUP:
                                    value = 1.f;
                                break;
                                default:
                                    value = 0.f;
                                break;
                            }
                        break;
                        case AxisFilter::Y_POS:
                            switch(inputEvent.jhat.value) {
                                case SDL_HAT_RIGHTUP:
                                case SDL_HAT_UP:
                                case SDL_HAT_LEFTUP:
                                    value = 1.f;
                                break;
                                default:
                                    value = 0.f;
                                break;
                            }
                        break;
                        case AxisFilter::X_NEG:
                            switch(inputEvent.jhat.value) {
                                case SDL_HAT_LEFTUP:
                                case SDL_HAT_LEFT:
                                case SDL_HAT_LEFTDOWN:
                                    value = 1.f;
                                break;
                                default:
                                    value = 0.f;
                                break;
                            }
                        break;
                        case AxisFilter::Y_NEG:
                            switch(inputEvent.jhat.value) {
                                case SDL_HAT_LEFTDOWN:
                                case SDL_HAT_DOWN:
                                case SDL_HAT_RIGHTDOWN:
                                    value = 1.f;
                                break;
                                default:
                                    value = 0.f;
                                break;
                            }
                        break;
                        default:
                            assert(false && "Unsupported axis for this device and control");
                        break;
                    }
                break;

                case ControlType::MOTION:
                    switch(inputFilter.mAxisFilter) {
                        case X_CHANGE_POS:
                        case X_CHANGE_NEG: {
                            const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                            value = RangeMapperLinear{ 0.f, 128.f, 0.f, 1.f }(sign * inputEvent.jball.xrel);
                        }
                        break;
                        case Y_CHANGE_POS:
                        case Y_CHANGE_NEG: {
                            const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                            value = RangeMapperLinear{ 0.f, 128.f, 0.f, 1.f }(sign * inputEvent.jball.yrel);
                        }
                        break;
                        default:
                            assert(false && "Invalid filter for this device and control");
                        break;
                    }
                break;
                default:
                    assert(false && "Invalid or unsupported control type for this device");
                break;
            }
        }
        break;

        // Extract raw value from touch events
        case DeviceType::TOUCH:
            switch(inputFilter.mControl.mControlType){
                case ControlType::POINT:
                    switch(inputFilter.mAxisFilter) {
                        case AxisFilter::SIMPLE:
                            value = inputEvent.tfinger.pressure;
                        break;
                        case AxisFilter::X_POS:
                            value = inputEvent.tfinger.x;
                        break;
                        case AxisFilter::Y_POS:
                            value = inputEvent.tfinger.y;
                        break;
                        case AxisFilter::X_CHANGE_POS:
                        case AxisFilter::X_CHANGE_NEG: {
                            const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                            value = RangeMapperLinear{0.f, 1.f, 0.f, 1.f}(sign * inputEvent.tfinger.dx);
                        }
                        case AxisFilter::Y_CHANGE_POS:
                        case AxisFilter::Y_CHANGE_NEG: {
                            const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                            value = RangeMapperLinear{0.f, 1.f, 0.f, 1.f}(sign * inputEvent.tfinger.dy);
                        }
                    }
                break;
                default: 
                    assert(false && "unsupported touch control type");
                break;
            }
        break;
        default:
            assert(false && "Unsupported device type");
        break;
    }
    return value;
}

std::vector<AxisFilter> deriveAxisFilters(InputAttributesType attributes) {
    std::vector<AxisFilter> result {};

    if(attributes&HAS_BUTTON_VALUE) result.push_back(AxisFilter::SIMPLE);

    for(AxisFilterType i{X_POS}; i <= (attributes&N_AXES); ++i) {
        result.push_back(static_cast<AxisFilter>(i));

        if(attributes&HAS_NEGATIVE) {
            result.push_back(static_cast<AxisFilter>(i|AxisFilterMask::SIGN));
        }

        if(attributes&HAS_CHANGE_VALUE) {
            result.push_back(static_cast<AxisFilter>(i|AxisFilterMask::CHANGE));
            result.push_back(static_cast<AxisFilter>(i|AxisFilterMask::SIGN|AxisFilterMask::CHANGE));
        }
    }

    return result;
}

InputSourceDescription getInputIdentity(const SDL_Event& inputEvent) {
    InputSourceDescription inputIdentity {};
    switch(inputEvent.type) {
        /**
         * Mouse events
         */
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            inputIdentity.mDeviceType = DeviceType::MOUSE;
            inputIdentity.mControlType = ControlType::BUTTON;
            inputIdentity.mDevice = inputEvent.button.which;
            inputIdentity.mControl = inputEvent.button.button;
        break;
        case SDL_MOUSEMOTION:
            inputIdentity.mDeviceType = DeviceType::MOUSE;
            inputIdentity.mControlType = ControlType::POINT;
            inputIdentity.mDevice = inputEvent.motion.which;
        break;
        case SDL_MOUSEWHEEL:
            inputIdentity.mDeviceType = DeviceType::MOUSE;
            inputIdentity.mControlType = ControlType::MOTION;
            inputIdentity.mDevice = inputEvent.wheel.which;
        break;

        /** 
         * Keyboard events
         */
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            inputIdentity.mDeviceType = DeviceType::KEYBOARD;
            inputIdentity.mControlType = ControlType::BUTTON;
            inputIdentity.mControl = inputEvent.key.keysym.sym;
        break;
        case SDL_FINGERUP:
        case SDL_FINGERDOWN:
        case SDL_FINGERMOTION:
            inputIdentity.mDeviceType = DeviceType::TOUCH;
            inputIdentity.mControlType = ControlType::POINT;
            inputIdentity.mControl = inputEvent.tfinger.touchId;
        break;

        /**
         * Controller-like events
         */
        case SDL_JOYAXISMOTION:
            inputIdentity.mDeviceType = DeviceType::CONTROLLER;
            inputIdentity.mControlType = ControlType::AXIS;
            inputIdentity.mDevice = inputEvent.jaxis.which;
            inputIdentity.mControl = inputEvent.jaxis.axis;
        break;
        case SDL_JOYHATMOTION:
            inputIdentity.mDeviceType = DeviceType::CONTROLLER;
            inputIdentity.mControlType = ControlType::RADIO;
            inputIdentity.mDevice = inputEvent.jhat.which;
            inputIdentity.mControl = inputEvent.jhat.hat;
        break;
        case SDL_JOYBALLMOTION:
            inputIdentity.mDeviceType = DeviceType::CONTROLLER;
            inputIdentity.mControlType = ControlType::MOTION;
            inputIdentity.mControl = inputEvent.jball.ball;
            inputIdentity.mDevice = inputEvent.jball.which;
        break;
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            inputIdentity.mDeviceType = DeviceType::CONTROLLER;
            inputIdentity.mControlType = ControlType::BUTTON;
            inputIdentity.mDevice = inputEvent.jbutton.which;
            inputIdentity.mControl = inputEvent.jbutton.button;
        break;
        case SDL_CONTROLLERTOUCHPADDOWN:
        case SDL_CONTROLLERTOUCHPADUP:
        case SDL_CONTROLLERTOUCHPADMOTION:
            inputIdentity.mDeviceType = DeviceType::CONTROLLER;
            inputIdentity.mControlType = ControlType::POINT;
            inputIdentity.mControl = inputEvent.ctouchpad.touchpad;
            inputIdentity.mDevice = inputEvent.ctouchpad.which;
        break;
        default:
            // assert (false && "This event is unsupported");
        break;
    }
    if(inputIdentity.mDeviceType != DeviceType::NA) {
        inputIdentity.mAttributes = kInputSourceTypeAttributes.at(
            {inputIdentity.mDeviceType, inputIdentity.mControlType}
        );
    }
    return inputIdentity;
}

ActionContext& InputManager::operator[] (const std::string& actionContext) {
    return mActionContexts.at(actionContext).first;
}

void InputManager::queueInput(const SDL_Event& inputEvent) {
    // variable storing the (internal) identity of the
    // control that created this event
    InputSourceDescription inputIdentity { getInputIdentity(inputEvent) };
    // assert(inputIdentity && "This event is not supported");
    if(!inputIdentity) return;

    // Update the raw values of any input filters that are in use and
    // have changed this frame
    std::vector<InputFilter> updatedInputFilters {};
    for(AxisFilter axisFilter: deriveAxisFilters(inputIdentity.mAttributes)) {
        InputFilter inputFilter {inputIdentity, axisFilter};
        if(mRawInputState.find(inputFilter) != mRawInputState.end()) {
            double newValue { getRawValue(inputFilter, inputEvent) };
            if(mRawInputState[inputFilter] != newValue || inputFilter.mAxisFilter&AxisFilterMask::CHANGE){
                mRawInputState[inputFilter] = newValue;
                updatedInputFilters.push_back(inputFilter);
            }
        }
    }

    std::unordered_map<InputCombo, UnmappedInputValue> finalComboStates {};
    //  Apply filter changes to any mapped combination, adding input combo events
    // to the queue if the combo condition is fulfilled
    for(const InputFilter& filter: updatedInputFilters) {
        for(const InputCombo& combo: mInputFilterToCombos[filter]) {
            assert(combo && "This combo does not have a main control, making it invalid");
            bool modifier1Held { !combo.mModifier1 || mRawInputState[combo.mModifier1] >= mModifierThreshold };
            bool modifier2Held { !combo.mModifier2 || mRawInputState[combo.mModifier2] >= mModifierThreshold };

            // Prepare new combo state parameters
            const UnmappedInputValue previousComboState { mInputComboStates[combo] };
            UnmappedInputValue newComboState {};
            if(modifier1Held && modifier2Held) {
                const double upperBound { 1.f };
                const double lowerBound { combo.mDeadzone };
                const double threshold { combo.mTrigger == InputCombo::Trigger::ON_CHANGE? 0.f: combo.mThreshold };
                newComboState.mValue= RangeMapperLinear{lowerBound, upperBound, 0.f, 1.f}(mRawInputState[combo.mMainControl]);
                newComboState.mActivated = newComboState.mValue >= threshold;
            } else {
                newComboState.mValue= 0.f;
                newComboState.mActivated = false;
            }
            newComboState.mTimestamp = inputEvent.common.timestamp;

            //  Add input to queue to be consumed by subscribed action contexts when value ...
            if(
                ( // ... just exceeded threshold
                    combo.mTrigger == InputCombo::Trigger::ON_PRESS
                    && newComboState.mActivated 
                    && !previousComboState.mActivated

                ) || ( // ... just dropped below threshold
                    combo.mTrigger == InputCombo::Trigger::ON_RELEASE
                    && !newComboState.mActivated
                    && previousComboState.mActivated

                ) || ( // ... change just occurred
                    combo.mTrigger == InputCombo::Trigger::ON_CHANGE
                    && (
                        // ... and input type is change
                        (combo.mMainControl.mAxisFilter&AxisFilterMask::CHANGE && newComboState.mActivated)
                        // ... or a state change occurred (and therefore the new state should 
                        // be forwarded)
                        || (newComboState.mValue != previousComboState.mValue)
                    )
                )
            ) {
                mUnmappedInputs.push(std::pair(combo, newComboState));
            } 

            // Update presently stored combo state
            finalComboStates[combo] = newComboState;
        }
    }

    for(const auto& comboValuePair: finalComboStates) {
        mInputComboStates[comboValuePair.first] = comboValuePair.second;
    }
}

void InputManager::loadInputConfiguration(const nlohmann::json& inputConfiguration) {
    // clear old bindings
    std::vector<std::string> oldActionContexts {};
    for(auto actionContext: mActionContexts) {
        oldActionContexts.push_back(actionContext.first);
    }
    for(const std::string& context: oldActionContexts) {
        unregisterActionContext(context);
    }

    for(const std::string& actionContextName: inputConfiguration.at("action_contexts").get<std::vector<std::string>>()) {
        registerActionContext(actionContextName);
    }
    for(const nlohmann::json& actionDefinition: inputConfiguration.at("actions").get<std::vector<nlohmann::json>>()) {
        registerAction(actionDefinition);
    }
    for(const nlohmann::json& inputBinding: inputConfiguration.at("input_binds").get<std::vector<nlohmann::json>>()) {
        registerInputBind(inputBinding);
    }
}

void InputManager::registerInputBind(const nlohmann::json& inputBindingParameters) {
    mActionContexts.at(inputBindingParameters.at("context").get<std::string>()).first.registerInputBind(inputBindingParameters);
}

void InputManager::registerAction(const nlohmann::json& actionParameters) {
    mActionContexts.at(actionParameters.at("context").get<std::string>()).first.registerAction(actionParameters);
}

void InputManager::registerActionContext(const std::string& name, ActionContextPriority priority) {
    assert(mActionContexts.find(name) == mActionContexts.end()
        && "An action context with this name has already been registered");
    mActionContexts.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name),
        std::forward_as_tuple(
            std::pair<ActionContext, ActionContextPriority>(
                std::piecewise_construct,
                std::forward_as_tuple(*this, name),
                std::forward_as_tuple(priority)
            )
        )
    );
}

void InputManager::unregisterActionContext(const std::string& actionContextName) {
    assert(mActionContexts.find(actionContextName) != mActionContexts.end()
        && "No action context with this name has been registered before");

    {
        ActionContext& actionContext {
            mActionContexts.at(actionContextName).first
        };
        actionContext.unregisterInputBinds();
    }

    // Erase the last trace of this action context
    mActionContexts.erase(actionContextName);
}

void InputManager::dispatch(uint32_t targetTimeMillis) {
    if(mUnmappedInputs.empty()) return;

    // Send each pending input event to all action contexts 
    // that are listenining for it
    std::set<ActionContextName> updatedActionContexts {};
    while(
        !mUnmappedInputs.empty() 
        && mUnmappedInputs.front().second.mTimestamp <= targetTimeMillis
    ) {
        const std::pair<InputCombo, UnmappedInputValue>& inputPair { mUnmappedInputs.front() };
        bool allowPropagate { true };

        // each set of associated actions, in descending order of priority
        for(const std::set<ActionContextName>& actionSet: mInputComboToActionContexts[inputPair.first]) {
            // each action in this priority level
            for(const ActionContextName& actionContextName: actionSet) {
                ActionContext& actionContext{ mActionContexts.at(actionContextName).first };
                if(actionContext.enabled()) {
                    actionContext.mapToAction(inputPair.second, inputPair.first);
                    updatedActionContexts.insert(actionContextName);
                    allowPropagate = actionContext.propagateAllowed();
                }
                if(!allowPropagate) break;
            }
            if(!allowPropagate) break;
        }

        // Remove this input from the queue
        mUnmappedInputs.pop();
    }

    // Let each updated context dispatch actions to their subscribed
    // action handlers
    for(const ActionContextName& name: updatedActionContexts) {
        mActionContexts.at(name).first.dispatch();
    }
}

void InputManager::registerInputCombo(const std::string& actionContext, const InputCombo& inputCombo) {
    assert(mActionContexts.find(actionContext) != mActionContexts.end() && "No action context with this name has been registered");
    const ActionContextPriority& priority { mActionContexts.at(actionContext).second };

    // Add associated input filters to records that use them
    std::array<const InputFilter, 3> inputComboFilters {{
        {inputCombo.mMainControl},
        {inputCombo.mModifier1},
        {inputCombo.mModifier2}
    }};
    for(const InputFilter& inputFilter: inputComboFilters) {
        if(inputFilter) {
            mRawInputState.try_emplace(
                inputFilter,
                0.f
            );
            // TODO: only one of multiple combos gets mapped to a filter. We need all of them.
            mInputFilterToCombos[inputFilter].emplace(inputCombo);
        }
    }

    // Add input combo to records that require it
    mInputComboStates.try_emplace(inputCombo, UnmappedInputValue{});
    mInputComboToActionContexts[inputCombo][ActionContextPriority::TOTAL - 1 - priority].insert(actionContext);
}

void InputManager::unregisterInputCombo(const std::string& actionContext, const InputCombo& inputCombo) {
    assert(mActionContexts.find(actionContext) != mActionContexts.end() && "No action context with this name has been registered");
    assert(mInputComboStates.find(inputCombo) != mInputComboStates.end() 
        && "This input combination has not been registered, or has already\
        been unregistered."
    );

    ActionContextPriority contextPriority {mActionContexts.at(actionContext).second};
    mInputComboToActionContexts[inputCombo][ActionContextPriority::TOTAL - 1 - contextPriority].erase(
        actionContext
    );

    //  See if there is any action context for which this
    // input combo or any of its input filters must 
    // be retained
    std::array<const InputFilter, 3> inputComboFilters {{
        {inputCombo.mMainControl},
        {inputCombo.mModifier1},
        {inputCombo.mModifier2},
    }};

    //  Determine whether the combo is used by any other action 
    // context
    bool keepCombo { false };
    std::array<bool, 3> keepFilter { false, false, false};
    for(const auto& eachSet: mInputComboToActionContexts[inputCombo]) {
        if(!eachSet.empty()) {
            keepCombo=true;
            for(auto& keep: keepFilter) {
                keep=true;
            }
            break;
        }
    }

    //  If the combo is to be removed, see if any of its 
    // associated input filters (corresponding to individual
    // controls) are in use
    if(!keepCombo) {
        for(std::size_t i{0}; i < inputComboFilters.size(); ++i) {
            //  Empty filter, and therefore does not require
            // processing
            if(!inputComboFilters[i]) {
                keepFilter[i] = true;
                continue;
            }

            //  A size greater than 1 indicates that there's at least
            // one other combo (besides the one being deleted) using
            // this filter
            const std::set<InputCombo>& filterCombos { mInputFilterToCombos[inputComboFilters[i]] };
            if(filterCombos.size() > 1) {
                keepFilter[i] = true;
            }
        }
    }

    if(!keepCombo) {
        //  Remove the segment keeping track of the latest processed 
        // value for this combo
        mInputComboStates.erase(inputCombo);

        //  Remove all events in the input queue that correspond
        // to this input combination
        std::queue<std::pair<InputCombo, UnmappedInputValue>> newInputQueue {};
        while(!mUnmappedInputs.empty()) {
            const auto& currentInput { mUnmappedInputs.back() };
            if(currentInput.first != inputCombo) {
                newInputQueue.push(currentInput);
            }
            mUnmappedInputs.pop();
        }
        mUnmappedInputs = newInputQueue;

        // Remove this combo from filters that map to 
        // it, as well as the filter itself if there
        // are no more combos that require it.
        for(std::size_t i{0}; i < keepFilter.size(); ++i) {
            auto& filter { inputComboFilters[i] };
            bool keep { keepFilter[i] };

            mInputFilterToCombos[filter].erase(inputCombo);

            if(!keep) {
                mInputFilterToCombos.erase(filter);
                mRawInputState.erase(filter);
            }
        }
    }
}

void InputManager::unregisterInputCombos(const std::string& actionContext){
    assert(mActionContexts.find(actionContext) != mActionContexts.end() && "No action context with this name has been registered");

    //  Find all combos mapped to this action context
    ActionContextPriority priority { mActionContexts.at(actionContext).second };
    std::vector<InputCombo> combosToUnregister {};
    for(const auto& inputMapping: mInputComboToActionContexts) {
        if(inputMapping.second[priority].find(actionContext) != inputMapping.second[priority].end()) {
            combosToUnregister.push_back(inputMapping.first);
        }
    }

    //  Remove the mapping between each found combo and the context
    for(const InputCombo& combo: combosToUnregister) {
        unregisterInputCombo(actionContext, combo);
    }
}

void InputManager::unregisterInputCombos() {
    for(auto& actionPair: mActionContexts) {
        unregisterInputCombos(actionPair.first);
    }
}

nlohmann::json inputAttributesToJSON(InputAttributesType inputAttributes) {
    nlohmann::json inputAttributesParameters {
        {"n_axes", (inputAttributes&InputAttributes::N_AXES)},
        {"has_negative", (inputAttributes&InputAttributes::HAS_NEGATIVE) > 0},
        {"has_change_value", (inputAttributes&InputAttributes::HAS_CHANGE_VALUE) > 0},
        {"has_button_value", (inputAttributes&InputAttributes::HAS_BUTTON_VALUE) > 0},
        {"has_state_value", (inputAttributes&InputAttributes::HAS_STATE_VALUE) > 0},
        {"state_is_location", (inputAttributes&InputAttributes::STATE_IS_LOCATION) > 0},
    };
    return inputAttributesParameters;
}
InputAttributesType jsonToInputAttributes(const nlohmann::json inputAttributesParameters){
    InputAttributesType inputAttributes{ static_cast<InputAttributesType>(
        inputAttributesParameters.at("n_axes").get<uint8_t>()
        | (inputAttributesParameters.at("has_negative").get<bool>()?
            InputAttributes::HAS_NEGATIVE: 0)
        | (inputAttributesParameters.at("has_change_value").get<bool>()?
            InputAttributes::HAS_CHANGE_VALUE: 0)
        | (inputAttributesParameters.at("has_button_value").get<bool>()?
            InputAttributes::HAS_BUTTON_VALUE: 0)
        | (inputAttributesParameters.at("has_state_value").get<bool>()?
            InputAttributes::HAS_STATE_VALUE: 0)
        | (inputAttributesParameters.at("state_is_location").get<bool>()?
            InputAttributes::STATE_IS_LOCATION: 0)
    )};
    return inputAttributes;
}

nlohmann::json inputSourceDescriptionToJSON(const InputSourceDescription& inputSourceDescription) {
    nlohmann::json inputSourceDescriptionParameters {
        {"device_type", kDeviceTypeToString.at(inputSourceDescription.mDeviceType)},
        {"control_type", kControlTypeToString.at(inputSourceDescription.mControlType)},
        {"device", inputSourceDescription.mDevice},
        {"control", inputSourceDescription.mControl},
    };
    return inputSourceDescriptionParameters;
}
InputSourceDescription jsonToInputSourceDescription(const nlohmann::json& inputSourceDescriptionParameters) {
    InputSourceDescription inputSourceDescription {
        .mDevice { inputSourceDescriptionParameters.at("device").get<uint8_t>() },
        .mControl { inputSourceDescriptionParameters.at("control").get<uint32_t>() },
        .mDeviceType { kStringToDeviceType.at(inputSourceDescriptionParameters.at("device_type")) },
        .mControlType { kStringToControlType.at(inputSourceDescriptionParameters.at("control_type"))},
    };
    const InputSourceType inputSourceType { inputSourceDescription.mDeviceType, inputSourceDescription.mControlType };
    inputSourceDescription.mAttributes = kInputSourceTypeAttributes.at(inputSourceType);
    return inputSourceDescription;
}

nlohmann::json inputFilterToJSON(const InputFilter& inputFilter) {
    nlohmann::json inputFilterParameters {
        {"input_source", inputSourceDescriptionToJSON(inputFilter.mControl) },
        {"filter", kAxisFilterToString.at(inputFilter.mAxisFilter)}
    };
    return inputFilterParameters;
}
InputFilter jsonToInputFilter(const nlohmann::json& inputFilterParameters) {
    InputFilter inputFilter {
        .mControl { jsonToInputSourceDescription(inputFilterParameters.at("input_source")) },
        .mAxisFilter { kStringToAxisFilter.at(inputFilterParameters.at("filter")) },
    };
    return inputFilter;
}
nlohmann::json inputComboToJSON(const InputCombo& inputCombo) {
    nlohmann::json inputComboParameters{
        {"trigger", kInputComboTriggerToString.at(inputCombo.mTrigger)},
        {"main_control", inputFilterToJSON(inputCombo.mMainControl)},
        {"modifier_1", inputFilterToJSON(inputCombo.mModifier1)},
        {"modifier_2", inputFilterToJSON(inputCombo.mModifier2)},
        {"deadzone", inputCombo.mDeadzone},
        {"threshold", inputCombo.mThreshold},
    };
    return inputComboParameters;
}
InputCombo jsonToInputCombo(const nlohmann::json& inputComboParameters) {
    InputCombo inputCombo {
        .mMainControl { jsonToInputFilter(inputComboParameters.at("main_control").get<nlohmann::json::object_t>()) },
        .mModifier1 { jsonToInputFilter(inputComboParameters.at("modifier_1").get<nlohmann::json::object_t>()) },
        .mModifier2 { jsonToInputFilter(inputComboParameters.at("modifier_2").get<nlohmann::json::object_t>()) },
        .mTrigger { kStringToInputComboTrigger.at(inputComboParameters.at("trigger").get<std::string>()) },
        .mDeadzone { inputComboParameters.at("deadzone").get<float>() },
        .mThreshold { inputComboParameters.at("threshold").get<float>() },
    };
    return inputCombo;
}
nlohmann::json actionDefinitionToJSON(const ActionDefinition& actionDefinition) {
    nlohmann::json actionDefinitionParameters {
        {"name", actionDefinition.mName},
        {"attributes", inputAttributesToJSON(actionDefinition.mAttributes)},
        {"value_type", kActionValueTypeToString.at(actionDefinition.mValueType)},
    };
    return actionDefinitionParameters;
}
ActionDefinition jsonToActionDefinition(const nlohmann::json& actionDefinitionParameters) {
    ActionDefinition actionDefinition {
        .mName { actionDefinitionParameters.at("name").get<std::string>() },
        .mAttributes { jsonToInputAttributes(actionDefinitionParameters.at("attributes").get<nlohmann::json::object_t>())},
        .mValueType { kStringToActionValueType.at(actionDefinitionParameters.at("value_type").get<std::string>()) },
    };
    return actionDefinition;
}
