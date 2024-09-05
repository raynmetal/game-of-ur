#include "input_system.hpp"

ActionData ActionContext::ApplyInput(ActionData actionData, const AxisFilter axisFilter, const UnmappedInputValue& inputValue) {
    // write action state into the actionData variable
    const int valueSign { axisFilter & AxisFilterMask::SIGN? -1: 1 };
    actionData.mCommonData.mActivated = inputValue.mActivated;
    actionData.mCommonData.mTimestamp = inputValue.mTimestamp;
    switch(axisFilter){
        case SIMPLE:
            actionData.mSimpleData.mChangeValue += valueSign * inputValue.mChangeValue;
            actionData.mSimpleData.mStateValue = valueSign * inputValue.mStateValue;
        break;
        case X_POS:
        case X_NEG:
            actionData.mMultiAxisData.mChangeValue.x += valueSign * inputValue.mChangeValue;
            actionData.mMultiAxisData.mStateValue.x = valueSign * inputValue.mStateValue;
        break;
        case Y_POS:
        case Y_NEG:
            actionData.mMultiAxisData.mChangeValue.y += valueSign * inputValue.mChangeValue;
            actionData.mMultiAxisData.mStateValue.y = valueSign * inputValue.mStateValue;
        break;
        case Z_POS:
        case Z_NEG:
            actionData.mMultiAxisData.mChangeValue.z += valueSign * inputValue.mChangeValue;
            actionData.mMultiAxisData.mStateValue.z = valueSign * inputValue.mStateValue;
        break;
        default:
            assert(false && "This is an unsupported path and should never execute");
        break;
    }

    // Normalize/clamp action states with magnitudes greater than 1.f
    if(
        actionData.mCommonData.mType == MULTIAXIS
        && actionData.mMultiAxisData.mStateValue.length() > 1.f
    ) {
        actionData.mMultiAxisData.mStateValue = glm::normalize(actionData.mMultiAxisData.mStateValue);
    } else if (
        actionData.mCommonData.mType == SIMPLE
        && (
            actionData.mSimpleData.mStateValue > 1.f
            || actionData.mSimpleData.mStateValue < -1.f
        )
    ) {
        actionData.mSimpleData.mStateValue = (
            std::signbit(actionData.mSimpleData.mStateValue)?
            -1.f: 1.f
        );
    }

    return actionData;
}

void ActionContext::registerAction(const std::string& name, ActionAttributes attributes) {
    assert(mActions.find(ActionDefinition{.mName{name}}) == mActions.end() && "Another action with this name has already been registered");

    const ActionDefinition actionDefinition {name, attributes};
    ActionData initialActionData { attributes & N_AXES? ActionData{MultiAxisActionData{}}: ActionData{SimpleActionData{}} };

    mActions.emplace(std::pair<ActionDefinition, ActionData>{actionDefinition, initialActionData});
    mActionToInputBinds.emplace(std::pair<ActionDefinition, std::set<InputCombo>>{actionDefinition, {}});
    mActionHandlers.emplace(std::pair<ActionDefinition, std::set<std::weak_ptr<IActionHandler>>> {actionDefinition, {}});
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
    ActionDefinition actionDefinition { .mName{action} };
    mActionHandlers.at(actionDefinition).erase(actionHandler);
}

void ActionContext::unregisterActionHandler(std::weak_ptr<IActionHandler> actionHandler) {
    for(const auto& actionValuePairs: mActions) {
        mActionHandlers[actionValuePairs.first].erase(actionHandler);
    }
}

void ActionContext::registerInputBind(const std::string& forAction, AxisFilter onAxis, const InputCombo& withInput){
    const auto& actionDefinitionIter {mActions.find(ActionDefinition{.mName{forAction}})};
    assert(mInputBindToAction.find(withInput) == mInputBindToAction.end() && "This input combination has already been registered with another action");
    assert(actionDefinitionIter != mActions.end() && "This action has not been registered");

    const ActionDefinition& actionDefinition { actionDefinitionIter->first };
    assert(
        ( // Assert that the axis filter and action match
            ( // onAxis should indicate a corresponding axis if the action is multiaxis
                actionDefinition.mAttributes & ActionAttributes::N_AXES
                && onAxis & AxisFilterMask::ID
            ) || ( // onAxis must be simple if the action is simple
                !(actionDefinition.mAttributes & ActionAttributes::N_AXES)
                && !(onAxis & AxisFilterMask::ID)
            )

        ) && ( // the axis indicated by onAxis must be one of the dimensions possessed by the action
            (onAxis & AxisFilterMask::ID)
            <= (actionDefinition.mAttributes & ActionAttributes::N_AXES)

        ) && ( // if onAxis is negative, then the action must support negative values
            (onAxis & AxisFilterMask::SIGN)
            == (actionDefinition.mAttributes & ActionAttributes::HAS_NEGATIVE)
        ) 

        && "The axis specified is not among those available for this action"
    );

    mInputBindToAction.emplace(withInput, std::pair<AxisFilter, ActionDefinition>{onAxis, actionDefinition});
    mActionToInputBinds[actionDefinition].emplace(withInput);
    mInputManager.registerInputCombo(mName, withInput);
}

void ActionContext::unregisterInputBind(const InputCombo& inputCombo) {
    const auto& inputBindToActionIter { mInputBindToAction.find(inputCombo) };
    assert(inputBindToActionIter != mInputBindToAction.end() && "This input binding does not exist");

    const ActionDefinition& actionDefinition { inputBindToActionIter->second.second };
    mInputBindToAction.erase(inputCombo);
    mActionToInputBinds[actionDefinition].erase(inputCombo);
    mInputManager.unregisterInputCombo(mName, inputCombo);
}

void ActionContext::unregisterInputBinds(const std::string& forAction) {
    const auto& actionIter { mActions.find(ActionDefinition{.mName{ forAction }}) };
    assert(actionIter != mActions.end() && "This action has not been registered");

    while(!mActionToInputBinds[actionIter->first].empty()) {
        const InputCombo& inputCombo { *mActionToInputBinds[actionIter->first].begin() };
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
    const ActionDefinition& actionDefinition { mActions.find(ActionDefinition{forAction})->first };

    // Get data from the front of the queue if this input happens
    // to be a component of the same action. Otherwise, construct a
    // fresh actionData struct
    ActionData actionData {
        actionDefinition.mAttributes & ActionAttributes::N_AXES?
            ActionData{ MultiAxisActionData{} }:
            ActionData{ SimpleActionData{} }
    };

    actionData.mCommonData.mTimestamp = timestamp;

    switch(actionDefinition.mTrigger) {
        case ActionDefinition::ON_PRESS:
        case ActionDefinition::ON_RELEASE:
            if(actionDefinition == mPendingTriggeredActions.back().first) {
                mPendingTriggeredActions.pop_back();
            }
            mPendingTriggeredActions.push_back({actionDefinition, actionData});
        break;

        case ActionDefinition::ON_CHANGE:
            mPendingContinuousActions[actionDefinition] = actionData;
        break;
    }
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

    const ActionDefinition& actionDefinition { mActions.find(comboToActionIter->second.second)->first };
    switch(actionDefinition.mTrigger) {
        case ActionDefinition::ON_PRESS:
        case ActionDefinition::ON_RELEASE:
            mapToTriggeredAction(inputValue, inputCombo);
        break;
        case ActionDefinition::ON_CHANGE:
            mapToContinuousAction(inputValue, inputCombo);
        break;
        default:
            assert(false && "The current input combo has an unknown trigger condition");
        break;
    }
}

void ActionContext::mapToTriggeredAction(const UnmappedInputValue& inputValue, const InputCombo& inputCombo) {
    const ActionDefinition& actionDefinition { 
        mActions.find(mInputBindToAction.at(inputCombo).second)->first
    };
    const AxisFilter& axisFilter { mInputBindToAction.at(inputCombo).first };

    // Get data from the front of the queue if this input happens 
    // to be a component of the same action. Otherwise, construct a
    // fresh actionData struct
    ActionData actionData {
        actionDefinition.mAttributes & ActionAttributes::N_AXES?
            ActionData{ MultiAxisActionData{} }:
            ActionData{ SimpleActionData{} }
    };
    if(
        !mPendingTriggeredActions.empty()
        && actionDefinition == mPendingTriggeredActions.back().first
    ) {
        actionData = mPendingTriggeredActions.back().second;
        mPendingTriggeredActions.pop_back();
    }

    actionData = ApplyInput(actionData, axisFilter, inputValue);

    //  Push the newly constructed actionData to the back of
    // our pending action queue
    mPendingTriggeredActions.push_back(
        std::pair<ActionDefinition, ActionData>(actionDefinition, actionData)
    );
}

void ActionContext::mapToContinuousAction(const UnmappedInputValue& inputValue, const InputCombo& inputCombo) {
    const ActionDefinition& actionDefinition { 
        mActions.find(mInputBindToAction.at(inputCombo).second)->first
    };
    const AxisFilter& axisFilter { mInputBindToAction.at(inputCombo).first };

    // Create or retrieve an actionData struct representing a snapshot
    // of action data changes in this frame, later to be forwarded to any
    // handlers subscribed to this action
    ActionData actionData {
        actionDefinition.mAttributes & ActionAttributes::N_AXES?
            ActionData{ MultiAxisActionData{} }:
            ActionData{ SimpleActionData{} }
    };
    if(mPendingContinuousActions.find(actionDefinition) != mPendingContinuousActions.end()) {
        actionData = mPendingContinuousActions[actionDefinition];
    }

    actionData = ApplyInput(actionData, axisFilter, inputValue);

    // Store the new value in our pending action list
    mPendingContinuousActions[actionDefinition] = actionData;
}

void ActionContext::dispatch() {
    for(const auto& pendingAction: mPendingContinuousActions) {
        std::vector<std::weak_ptr<IActionHandler>> handlersToUnregister {};
        for(auto handler: mActionHandlers[pendingAction.first]) {
            if(!handler.expired()) {
                handler.lock()->handleAction(pendingAction.second);
            } else {
                handlersToUnregister.push_back(handler);
            }
        }
        for(auto handler: handlersToUnregister) {
            unregisterActionHandler(handler);
        }
    }
    mPendingContinuousActions.clear();

    for(const auto& pendingAction: mPendingTriggeredActions) {
        std::vector<std::weak_ptr<IActionHandler>> handlersToUnregister {};
        for(auto handler: mActionHandlers[pendingAction.first]) {
            if(!handler.expired()) {
                handler.lock()->handleAction(pendingAction.second);
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
