#include <memory>

#include "input_system.hpp"

ActionData ActionContext::ApplyInput(const ActionDefinition& actionDefinition, const ActionData& actionData, const AxisFilter targetAxis, const UnmappedInputValue& inputValue) {
    // write action state into the actionData variable
    const float valueSign { targetAxis&AxisFilterMask::SIGN? -1.f: 1.f };
    const double newValue { valueSign * inputValue.mValue };
    ActionData newActionData{ actionData };
    newActionData.mCommonData.mTimestamp = inputValue.mTimestamp;

    switch(targetAxis){
        case AxisFilter::SIMPLE:
            assert(
                actionDefinition.mAttributes&InputAttributes::HAS_BUTTON_VALUE
                && "Action must support button values for AxisFilter::SIMPLE to apply"
            );
            newActionData.mCommonData.mActivated = inputValue.mActivated;
        break;

        case AxisFilter::X_POS:
        case AxisFilter::X_NEG:
            assert(
                (actionDefinition.mAttributes&InputAttributes::HAS_STATE_VALUE
                || actionDefinition.mAttributes&InputAttributes::HAS_CHANGE_VALUE)
                && static_cast<uint8_t>(actionDefinition.mAttributes&InputAttributes::N_AXES) >= static_cast<uint8_t>(AxisFilter::X_POS)
                && "Action must support change values or state values and have one or more axes"
            );
            if(
                actionDefinition.mValueType == ActionValueType::STATE
                // affect only if the old value and the new one belong to the 
                // same axis (or is 0.f)
                && valueSign * actionData.mOneAxisActionData.mValue >= 0.f
            ) {
                newActionData.mOneAxisActionData.mValue = newValue;
            } else if (actionDefinition.mValueType == ActionValueType::CHANGE) {
                newActionData.mOneAxisActionData.mValue += newValue;
            }
        break;

        case AxisFilter::Y_POS:
        case AxisFilter::Y_NEG:
            assert(
                (actionDefinition.mAttributes&InputAttributes::HAS_STATE_VALUE
                || actionDefinition.mAttributes&InputAttributes::HAS_CHANGE_VALUE)
                && static_cast<uint8_t>(actionDefinition.mAttributes&InputAttributes::N_AXES) >= static_cast<uint8_t>(AxisFilter::Y_POS)
                && "Action must support change values or state values and have two or more axes"
            );
            if(
                actionDefinition.mValueType == ActionValueType::STATE
                && valueSign * actionData.mTwoAxisActionData.mValue.y >= 0.f
            ) {
                newActionData.mTwoAxisActionData.mValue.y = newValue;
            } else if (actionDefinition.mValueType == ActionValueType::CHANGE) {
                newActionData.mTwoAxisActionData.mValue.y += newValue;
            }
        break;

        case AxisFilter::Z_POS:
        case AxisFilter::Z_NEG:
            assert(
                (actionDefinition.mAttributes&InputAttributes::HAS_STATE_VALUE
                || actionDefinition.mAttributes&InputAttributes::HAS_CHANGE_VALUE)
                && (static_cast<uint8_t>(actionDefinition.mAttributes&InputAttributes::N_AXES) == static_cast<uint8_t>(AxisFilter::Z_POS))
                && "Action must support change or state values and must have three axes"
            );
            if(
                actionDefinition.mValueType== ActionValueType::STATE
                && valueSign * actionData.mThreeAxisActionData.mValue.z >= 0.f
            ) {
                newActionData.mThreeAxisActionData.mValue.z = newValue;
            } else if (actionDefinition.mValueType == ActionValueType::CHANGE) {
                newActionData.mThreeAxisActionData.mValue.z += newValue;
            }
        break;

        default:
            assert(false && "This is an unsupported path and should never execute");
        break;
    }

    // Normalize/clamp non-location action states with magnitudes greater than 1.f.
    if(
        newActionData.mCommonData.mType != ActionType::BUTTON
        // Change values are kept as is
        && !(actionDefinition.mAttributes&InputAttributes::HAS_CHANGE_VALUE)
        // location states are kept at their full range (otherwise we'd miss ~1/3rd of
        // the window)
        && !(actionDefinition.mAttributes&InputAttributes::STATE_IS_LOCATION)
        && glm::length(newActionData.mThreeAxisActionData.mValue) > 1.f
    ) {
        // NOTE: in order for this to work, we need action data to guarantee
        // that unused dimensions have a value of 0.f
        newActionData.mThreeAxisActionData.mValue = glm::normalize(newActionData.mThreeAxisActionData.mValue);
    }

    return newActionData;
}

void ActionContext::registerAction(const std::string& name, InputAttributesType attributes) {
    assert(mActions.find(ActionDefinition{.mName{name}}) == mActions.end() && "Another action with this name has already been registered");
    assert(
        !((attributes&InputAttributes::HAS_CHANGE_VALUE) && (attributes&InputAttributes::HAS_STATE_VALUE))
        && "Action may either have a change value or a state value but not both"
    );

    ActionDefinition actionDefinition { name, attributes };
    actionDefinition.mValueType = attributes&InputAttributes::HAS_CHANGE_VALUE? ActionValueType::CHANGE: ActionValueType::STATE;
    ActionData initialActionData { static_cast<uint8_t>(actionDefinition.mAttributes&InputAttributes::N_AXES) };

    mActions.emplace(std::pair<ActionDefinition, ActionData>{actionDefinition, initialActionData});
    mActionToInputBinds.emplace(std::pair<ActionDefinition, std::set<InputCombo>>{actionDefinition, {}});
    mActionHandlers.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(actionDefinition),
        std::forward_as_tuple<std::set<std::weak_ptr<IActionHandler>, std::owner_less<std::weak_ptr<IActionHandler>>>>({})
    );
}

void ActionContext::unregisterAction(const std::string& name) {
    const auto& actionIter {mActions.find(ActionDefinition{.mName{name}})};
    assert(actionIter != mActions.end() && "This action is not registered with this context");
    const auto& actionDefinition { actionIter->first };

    mActionHandlers.erase(actionDefinition);
    unregisterInputBinds(actionDefinition.mName);
    mActions.erase(actionDefinition);
}

void ActionContext::registerActionHandler(const std::string& action, std::weak_ptr<IActionHandler> actionHandler) {
    ActionDefinition actionDefinition { .mName{action} };
    mActionHandlers.at(actionDefinition).insert(actionHandler);
}

void ActionContext::unregisterActionHandler(const std::string& action, std::weak_ptr<IActionHandler> actionHandler) {
    const ActionDefinition& actionDefinition { .mName{action} };
    mActionHandlers.at(actionDefinition).erase(actionHandler);
}

void ActionContext::unregisterActionHandler(std::weak_ptr<IActionHandler> actionHandler) {
    for(const auto& actionValuePairs: mActions) {
        mActionHandlers.at(actionValuePairs.first).erase(actionHandler);
    }
}

void ActionContext::registerInputBind(const std::string& forAction, AxisFilter targetAxis, const InputCombo& withInput){
    const auto& actionDefinitionIter {mActions.find(ActionDefinition{.mName{forAction}})};
    assert(mInputBindToAction.find(withInput) == mInputBindToAction.end() && "This input combination has already been registered with another action");
    assert(actionDefinitionIter != mActions.end() && "This action has not been registered");

    const ActionDefinition& actionDefinition { actionDefinitionIter->first };
    assert(
        ( // the axis indicated by onAxis must be one of the dimensions possessed by the action
            ((static_cast<uint8_t>(targetAxis)&AxisFilterMask::ID) <= (static_cast<uint8_t>(actionDefinition.mAttributes)&InputAttributes::N_AXES))
        ) && ( 
            // Target axis isn't negative, ...
            !(static_cast<uint8_t>(targetAxis)&AxisFilterMask::SIGN)
            // ... or if it is, then ...
            || (
                // .. the action must support negative state values...
                (static_cast<uint8_t>(actionDefinition.mAttributes)&InputAttributes::HAS_NEGATIVE)
                // ... or support change values (which de facto support negative values).
                || (static_cast<uint8_t>(actionDefinition.mAttributes)&InputAttributes::HAS_CHANGE_VALUE)
            )
        )
        && "The axis specified is not among those available for this action"
    );
    mInputBindToAction.emplace(withInput, std::pair<AxisFilter, ActionDefinition>{targetAxis, actionDefinition});
    mActionToInputBinds[actionDefinition].emplace(withInput);
    mInputManager.registerInputCombo(mName, withInput);
}

void ActionContext::unregisterInputBind(const InputCombo& inputCombo) {
    const auto& inputBindToActionIter { mInputBindToAction.find(inputCombo) };
    assert(inputBindToActionIter != mInputBindToAction.end() && "This input binding does not exist");

    const ActionDefinition& actionDefinition { std::get<1>(inputBindToActionIter->second) };
    mInputBindToAction.erase(inputCombo);
    mActionToInputBinds[actionDefinition].erase(inputCombo);
    mInputManager.unregisterInputCombo(mName, inputCombo);
}

void ActionContext::unregisterInputBinds(const std::string& forAction) {
    const auto& actionIter { mActions.find(ActionDefinition{.mName{ forAction }}) };
    assert(actionIter != mActions.end() && "This action has not been registered");

    while(!mActionToInputBinds[actionIter->first].empty()) {
        const InputCombo inputCombo { *mActionToInputBinds[actionIter->first].begin() };
        mInputBindToAction.erase(inputCombo);
        mActionToInputBinds[actionIter->first].erase(inputCombo);
        mInputManager.unregisterInputCombo(mName, inputCombo);
    }
}

void ActionContext::unregisterInputBinds() {
    for(const auto& actionIter: mActions) {
        unregisterInputBinds(actionIter.first.mName);
    }
}

void ActionContext::resetActionData(const std::string& forAction, uint32_t timestamp) {
    // TODO: Figure out how to handle this properly. When will this take place anyway?
    const ActionDefinition& actionDefinition { mActions.find(ActionDefinition{ forAction })->first };

    // Let action listeners know that a reset has occurred
    ActionData actionData { static_cast<uint8_t>(actionDefinition.mAttributes&InputAttributes::N_AXES) };
    actionData.mCommonData.mTimestamp = timestamp;
    actionData.mCommonData.mTriggeredBy = ActionTrigger::RESET;
    mPendingTriggeredActions.push_back(std::pair(actionDefinition, actionData));
    mActions[actionDefinition] = actionData;
}

void ActionContext::resetActionData(uint32_t timestamp) {
    for(const auto& action: mActions) {
        resetActionData(action.first.mName, timestamp);
    }
}

void ActionContext::mapToAction(const UnmappedInputValue& inputValue, const InputCombo& inputCombo) {
    const auto& comboToActionIter { mInputBindToAction.find(inputCombo) };
    assert(
        comboToActionIter != mInputBindToAction.end() 
        && "This input combination has not been bound to any action in this context"
    );

    const ActionDefinition& actionDefinition { mActions.find(comboToActionIter->second.second)->first};
    const AxisFilter& axisFilter { comboToActionIter->second.first };

    // Action state values should be retrieved from memory, while change 
    // values should be made fresh
    ActionData actionData { 
        (actionDefinition.mAttributes&InputAttributes::HAS_CHANGE_VALUE)?
        ActionData { static_cast<uint8_t>(actionDefinition.mAttributes&N_AXES) }:
        mActions[actionDefinition] 
    };
    if(
        !mPendingTriggeredActions.empty()
        && actionDefinition == mPendingTriggeredActions.back().first
    ) {
        actionData = mPendingTriggeredActions.back().second;
        mPendingTriggeredActions.pop_back();
    }

    actionData = ApplyInput(actionDefinition, actionData, axisFilter, inputValue);

    //  Push the newly constructed actionData to the back of
    // our pending action queue
    mPendingTriggeredActions.push_back(std::pair(actionDefinition, actionData));

    // Update the action map with this latest action value
    mActions[actionDefinition] = actionData;
}

void ActionContext::dispatch() {
    for(const auto& pendingAction: mPendingTriggeredActions) {
        std::vector<std::weak_ptr<IActionHandler>> handlersToUnregister {};
        for(auto handler: mActionHandlers[pendingAction.first]) {
            if(!handler.expired()) {
                handler.lock()->handleAction(pendingAction.second, pendingAction.first);
            } else {
                handlersToUnregister.push_back(handler);
            }
        }
        for(auto handler: handlersToUnregister) {
            unregisterActionHandler(handler);
        }
    }
    mPendingTriggeredActions.clear();
}