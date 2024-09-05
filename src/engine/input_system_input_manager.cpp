#include <cassert>
#include <algorithm>

#include <SDL2/SDL.h>

#include "util.hpp"
#include "window_context_manager.hpp"
#include "input_system.hpp"

bool hasValue(const InputIdentity& input, const AxisFilterType filter) {
    uint8_t axisID { filter&AxisFilterMask::ID };
    uint8_t axisSign { filter&AxisFilterMask::SIGN };
    uint8_t axisDelta { filter&AxisFilterMask::CHANGE };

    return (
        input
        && (
            // No axis specified, so in other words it's a button
            (axisID == AxisFilter::SIMPLE && input.mAttributes&HAS_BUTTON_VALUE)

            // An axis is specified, so check whether the desired value
            // supports negatives or represents a change
            || (
                axisID > 0
                && axisID <= input.mAttributes&N_AXES
                && (
                    axisSign <= input.mAttributes&HAS_NEGATIVE
                    || axisDelta <= input.mAttributes&HAS_CHANGE_VALUE
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
                case ControlType::POINT:
                    switch(inputFilter.mAxisFilter) {

                        // Mouse button filter
                        case SIMPLE:
                            value = inputEvent.button.state;
                        break;

                        // Mouse location filter
                        case X_POS:
                            switch(inputEvent.type) {
                                case SDL_MOUSEMOTION:
                                    value = RangeMapperLinear{0.f, windowWidth, 0.f, 1.f}(inputEvent.motion.x);
                                break;
                                case SDL_MOUSEBUTTONDOWN:
                                case SDL_MOUSEBUTTONUP:
                                    value = RangeMapperLinear{0.f, windowWidth, 0.f, 1.f}(inputEvent.button.x);
                                break;
                            }
                        break;
                        case Y_POS:
                            switch(inputEvent.type) {
                                case SDL_MOUSEMOTION:
                                    value = RangeMapperLinear{0.f, windowHeight, 0.f, 1.f}(inputEvent.motion.y);
                                break;
                                case SDL_MOUSEBUTTONDOWN:
                                case SDL_MOUSEBUTTONUP:
                                    value = RangeMapperLinear{0.f, windowHeight, 0.f, 1.f}(inputEvent.button.y);
                                break;
                            }
                        break;

                        // Mouse movement filter
                        case X_CHANGE_POS:
                        case X_CHANGE_NEG:
                        {
                            const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                            value = RangeMapperLinear{0.f, windowWidth, 0.f, 1.f}(sign * inputEvent.motion.xrel);
                        }
                        break;
                        case Y_CHANGE_POS:
                        case Y_CHANGE_NEG:
                        {
                            const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                            value = RangeMapperLinear{0.f, windowHeight, 0.f, 1.f}(sign * inputEvent.motion.yrel);
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
                        case X_CHANGE_NEG:
                        {
                            const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                            value = RangeMapperLinear{0.f, 1.f, 0.f, 1.f}(sign * inputEvent.wheel.x);
                        }
                        break;
                        case Y_CHANGE_POS:
                        case Y_CHANGE_NEG:
                        {
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
                case POINT:
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

                case BUTTON:
                    assert(inputFilter.mAxisFilter == AxisFilter::SIMPLE);
                    value = inputEvent.cbutton.state;
                break;

                case AXIS:
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

                case RADIO:
                    switch(inputFilter.mAxisFilter) {
                        case X_POS:
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
                        case Y_POS:
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
                        case X_NEG:
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
                        case Y_NEG:
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

                case MOTION:
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
            switch(inputFilter.mAxisFilter) {
                case SIMPLE:
                    value = inputEvent.tfinger.pressure;
                break;
                case X_POS:
                    value = inputEvent.tfinger.x;
                break;
                case Y_POS:
                    value = inputEvent.tfinger.y;
                break;
                case X_CHANGE_POS:
                case X_CHANGE_NEG: {
                    const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                    value = RangeMapperLinear{0.f, 1.f, 0.f, 1.f}(sign * inputEvent.tfinger.dx);
                }
                case Y_CHANGE_POS:
                case Y_CHANGE_NEG: {
                    const float sign { inputFilter.mAxisFilter&AxisFilterMask::SIGN? -1.f: 1.f };
                    value = RangeMapperLinear{0.f, 1.f, 0.f, 1.f}(sign * inputEvent.tfinger.dy);
                }
            }
        break;
        default:
            assert(false && "Unsupported device type");
        break;
    }
    return value;
}

std::vector<AxisFilter> deriveAxisFilters(ActionAttributesType attributes) {
    std::vector<AxisFilter> result {};

    if(attributes&HAS_BUTTON_VALUE) result.push_back(AxisFilter::SIMPLE);

    for(AxisFilterType i{X_POS}; i <= attributes&N_AXES; ++i) {
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

InputIdentity getInputIdentity(const SDL_Event& inputEvent) {
    InputIdentity inputIdentity;
    inputIdentity.mControl = inputEvent.common.type;
    switch(inputEvent.type) {
        /**
         * Mouse events
         */
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            inputIdentity.mAttributes = (
                (N_AXES & 2) 
                | HAS_BUTTON_VALUE
                | HAS_STATE_VALUE
                | STATE_IS_LOCATION
            );
            inputIdentity.mDevice = inputEvent.button.which;
            inputIdentity.mControl = inputEvent.button.button;
            inputIdentity.mDeviceType = DeviceType::MOUSE;
            inputIdentity.mControlType = ControlType::POINT;
        break;
        case SDL_MOUSEMOTION:
            inputIdentity.mAttributes = (
                (N_AXES & 2) 
                | HAS_STATE_VALUE
                | HAS_CHANGE_VALUE
                | STATE_IS_LOCATION
            );
            inputIdentity.mDevice = inputEvent.motion.which;
            inputIdentity.mDeviceType = DeviceType::MOUSE;
            inputIdentity.mControlType = ControlType::POINT;
        break;
        case SDL_MOUSEWHEEL:
            inputIdentity.mAttributes = (
                (N_AXES & 2) 
                | HAS_CHANGE_VALUE
            );
            inputIdentity.mDevice = inputEvent.wheel.which;
            inputIdentity.mDeviceType = DeviceType::MOUSE;
            inputIdentity.mControlType = ControlType::MOTION;
        break;

        /** 
         * Keyboard events
         */
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            inputIdentity.mAttributes = (
                HAS_BUTTON_VALUE
            );
            inputIdentity.mControl = inputEvent.key.keysym.sym;
            inputIdentity.mDeviceType = DeviceType::KEYBOARD;
            inputIdentity.mControlType = ControlType::BUTTON;
        break;

        case SDL_FINGERUP:
        case SDL_FINGERDOWN:
            inputIdentity.mAttributes = HAS_BUTTON_VALUE;
        case SDL_FINGERMOTION:
            inputIdentity.mAttributes |= (
                (N_AXES & 2)
                | HAS_STATE_VALUE
                | HAS_CHANGE_VALUE
                | STATE_IS_LOCATION
            );
            inputIdentity.mControl = inputEvent.tfinger.touchId;
            inputIdentity.mDeviceType = DeviceType::TOUCH;
            inputIdentity.mControlType = ControlType::POINT;
        break;

        /**
         * Controller-like events
         */
        case SDL_JOYAXISMOTION:
            inputIdentity.mAttributes = (
                (N_AXES&1) 
                | HAS_STATE_VALUE
                | HAS_NEGATIVE
            );
            inputIdentity.mDevice = inputEvent.jaxis.which;
            inputIdentity.mControl = inputEvent.jaxis.axis;
            inputIdentity.mDeviceType = DeviceType::CONTROLLER;
            inputIdentity.mControlType = ControlType::AXIS;
        break;
        case SDL_JOYHATMOTION:
            inputIdentity.mAttributes = (
                (N_AXES&2)
                | HAS_STATE_VALUE
                | HAS_NEGATIVE
            );
            inputIdentity.mDevice = inputEvent.jhat.which;
            inputIdentity.mControl = inputEvent.jhat.hat;
            inputIdentity.mDeviceType = DeviceType::CONTROLLER;
            inputIdentity.mControlType = ControlType::RADIO;
        break;
        case SDL_JOYBALLMOTION:
            inputIdentity.mAttributes = (
                (N_AXES&2)
                | HAS_CHANGE_VALUE
            );
            inputIdentity.mControl = inputEvent.jball.ball;
            inputIdentity.mDevice = inputEvent.jball.which;
            inputIdentity.mDeviceType = DeviceType::CONTROLLER;
            inputIdentity.mControlType = ControlType::MOTION;
        break;
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            inputIdentity.mAttributes = (
                HAS_BUTTON_VALUE
            );
            inputIdentity.mDevice = inputEvent.jbutton.which;
            inputIdentity.mControl = inputEvent.jbutton.button;
            inputIdentity.mDeviceType = DeviceType::CONTROLLER;
            inputIdentity.mControlType = ControlType::BUTTON;
        break;
        case SDL_CONTROLLERTOUCHPADDOWN:
        case SDL_CONTROLLERTOUCHPADUP:
            inputIdentity.mAttributes = HAS_BUTTON_VALUE;
        case SDL_CONTROLLERTOUCHPADMOTION:
            inputIdentity.mAttributes |= (
                (N_AXES & 2)
                | HAS_STATE_VALUE
                | STATE_IS_LOCATION
            );
            inputIdentity.mControl = inputEvent.ctouchpad.touchpad;
            inputIdentity.mDevice = inputEvent.ctouchpad.which;
            inputIdentity.mDeviceType = DeviceType::CONTROLLER;
            inputIdentity.mControlType = ControlType::POINT;
        break;
        default:
            assert (false && "This event is unsupported");
        break;
    }
    return inputIdentity;
}

ActionContext& InputManager::operator[] (const std::string& actionContext) {
    return mActionContexts[actionContext].first;
}

void InputManager::queueInput(const SDL_Event& inputEvent) {
    // variable storing the (internal) identity of the
    // control that created this event
    InputIdentity inputIdentity { getInputIdentity(inputEvent) };
    assert(inputIdentity && "This event is not supported");

    // Update the raw values of any input filters that are in use
    std::vector<InputFilter> updatedInputFilters {};
    for(AxisFilter axisFilter: deriveAxisFilters(inputIdentity.mAttributes)) {
        InputFilter inputFilter {inputIdentity, axisFilter};
        if(mRawInputState.find(inputFilter) != mRawInputState.end()) {
            mRawInputState[inputFilter] = getRawValue(inputFilter, inputEvent);
            updatedInputFilters.push_back(inputFilter);
        }
    }

    // Apply filter changes to any mapped combination, adding input events
    // to the queue if the combo condition is fulfilled
    for(const InputFilter& filter: updatedInputFilters) {
        for(const InputCombo& combo: mInputFilterToCombos[filter]) {
            assert(combo && "This combo does not have a main control, making it invalid");
            bool modifier1Held { !combo.mModifier1 || mRawInputState[combo.mModifier1] >= mModifierThreshold };
            bool modifier2Held { !combo.mModifier2 || mRawInputState[combo.mModifier2] >= mModifierThreshold };

            // Calculate new combo state parameters
            const UnmappedInputValue previousComboState { mInputComboStates[combo] };
            UnmappedInputValue newComboState {};
            if(modifier1Held && modifier2Held) {
                double mainControlState { mRawInputState[combo.mMainControl] };
                if(combo.mInverted) mainControlState = 1.f - mainControlState;
                float upperBound { combo.mInverted? 1.f - combo.mDeadzone: 1.f };
                float lowerBound { combo.mInverted? 0.f: combo.mDeadzone };
                mainControlState = RangeMapperLinear{ lowerBound, upperBound, 0.f, 1.f }(mainControlState);
                newComboState.mStateValue = mainControlState;
            } else {
                newComboState.mStateValue = 0.f;
            }
            newComboState.mChangeValue =  newComboState.mStateValue - previousComboState.mStateValue;
            newComboState.mTimestamp = inputEvent.common.timestamp;
            newComboState.mActivated = false;

            switch(combo.mTrigger) {
                case InputCombo::Trigger::ON_PRESS:
                    if(
                        newComboState.mStateValue >= combo.mThreshold
                        && previousComboState.mStateValue < combo.mThreshold
                    ) {
                        newComboState.mActivated = true;
                        mUnmappedInputs.push(std::pair(combo, newComboState));

                    } else if(
                        newComboState.mStateValue < combo.mThreshold
                        && previousComboState.mStateValue >= combo.mThreshold
                    ) {
                        mUnmappedInputs.push(std::pair(combo, newComboState));
                    }
                break;
                case InputCombo::Trigger::ON_RELEASE:
                    if(
                        newComboState.mStateValue >= combo.mThreshold
                        && previousComboState.mStateValue < combo.mThreshold
                    ) {
                        mUnmappedInputs.push(std::pair(combo, newComboState));
                    } else if(
                        newComboState.mStateValue < combo.mThreshold
                        && previousComboState.mStateValue >= combo.mThreshold
                    ) {
                        newComboState.mActivated = true;
                        mUnmappedInputs.push(std::pair(combo, newComboState));
                    }
                break;
                case InputCombo::Trigger::ON_CHANGE:
                    newComboState.mActivated = newComboState.mStateValue > 0.f;
                    if(newComboState.mChangeValue) {
                        mUnmappedInputs.push(std::pair(combo, newComboState));
                    }
                break;
            }

            // Update presently stored combo state
            mInputComboStates[combo] = newComboState;
        }
    }
}

void InputManager::registerActionContext(const std::string& name, ActionContextPriority priority) {
    assert(mActionContexts.find(name) == mActionContexts.end()
        && "An action context with this name has already been registered");
    mActionContexts.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name),
        std::forward_as_tuple(*this, name)
    );
}

void InputManager::unregisterActionContext(const std::string& actionContextName) {
    assert(mActionContexts.find(actionContextName) != mActionContexts.end()
        && "No action context with this name has been registered before");

    {
        ActionContext& actionContext {
            mActionContexts[actionContextName].first
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
                ActionContext& actionContext{ mActionContexts[actionContextName].first };
                if(actionContext.enabled()) {
                    actionContext.mapToAction(inputPair.second, inputPair.first);
                    updatedActionContexts.insert(actionContextName);
                    allowPropagate = actionContext.allowPropagate();
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
        mActionContexts[name].first.dispatch();
    }
}

void InputManager::registerInputCombo(const std::string& actionContext, const InputCombo& inputCombo) {
    assert(mActionContexts.find(actionContext) != mActionContexts.end() && "No action context with this name has been registered");
    const ActionContextPriority& priority { mActionContexts[actionContext].second };

    // Add associated input filters to records that use them
    std::array<const InputFilter&, 3> inputComboFilters {
        inputCombo.mMainControl,
        inputCombo.mModifier1,
        inputCombo.mModifier2
    };
    for(const InputFilter& inputFilter: inputComboFilters) {
        mRawInputState.try_emplace(
            inputFilter,
            0.f
        );
        mInputFilterToCombos[inputFilter].insert(inputCombo);
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

    ActionContextPriority contextPriority {mActionContexts[actionContext].second};
    mInputComboToActionContexts[inputCombo][ActionContextPriority::TOTAL - 1 - contextPriority].erase(
        actionContext
    );

    // See if there is any action context for which this
    // input combo or any of its input filters must 
    // be retained
    std::array<const InputFilter&, 3> inputComboFilters {
        inputCombo.mMainControl,
        inputCombo.mModifier1,
        inputCombo.mModifier2,
    };

    // Determine whether the combo is used by any other action 
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

    // If the combo is to be removed, see if any of its 
    // associated input filters (corresponding to individual
    // controls) are in use
    if(!keepCombo) {
        for(std::size_t i{0}; i < inputComboFilters.size(); ++i) {
            // Empty filter, and therefore does not require
            // processing
            if(!inputComboFilters[i]) {
                keepFilter[i] = true;
                continue;
            }

            // A size greater than 1 indicates that there's at least
            // one other combo (besides the one being deleted) using
            // this filter
            const std::set<InputCombo>& filterCombos { mInputFilterToCombos[inputComboFilters[i]] };
            if(filterCombos.size() > 1) {
                keepFilter[i] = true;
            }
        }
    }

    if(!keepCombo) {
        // Remove the segment keeping track of the latest processed 
        // value for this combo
        mInputComboStates.erase(inputCombo);

        // Remove all events in the input queue that correspond
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

    // find all combos mapped to this action context
    ActionContextPriority priority { mActionContexts[actionContext].second };
    std::vector<InputCombo> combosToUnregister {};
    for(const auto& inputMapping: mInputComboToActionContexts) {
        if(inputMapping.second[priority].find(actionContext) != inputMapping.second[priority].end()) {
            combosToUnregister.push_back(inputMapping.first);
        }
    }

    // Remove the mapping between each found combo and the context
    for(const InputCombo& combo: combosToUnregister) {
        unregisterInputCombo(actionContext, combo);
    }
}

void InputManager::unregisterInputCombos() {
    for(auto& actionPair: mActionContexts) {
        unregisterInputCombos(actionPair.first);
    }
}
