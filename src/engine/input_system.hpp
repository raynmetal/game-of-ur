#ifndef ZOINPUTSYSTEM_H
#define ZOINPUTSYSTEM_H

#include <string>
#include <map>
#include <queue>
#include <memory>

#include <SDL2/SDL.h>

#include "input_system_data.hpp"

class ActionContext;
class InputManager;

/*
 *   Processes raw SDL input events into unmapped inputs, and later
 * reports bind value changes to an ActionContext for conversion into 
 * associated action state changes
 */
class InputManager {
public:
    enum ActionContextPriority: uint8_t {
        VERY_LOW=0,
        LOW=1,
        DEFAULT=2,
        HIGH=3,
        VERY_HIGH=4,
        TOTAL=5
    };

    /* 
     * Maps an event to its internal representation, if one is available 
     */
    void queueInput(const SDL_Event& inputEvent);

    /*
     *  Registers a new action context with a given name
     */
    void registerActionContext(const std::string& name, ActionContextPriority priority=DEFAULT);

    /*
     *  Removes the action context associated with this name
     */
    void unregisterActionContext(const std::string& name);

    ActionContext& operator[] (const std::string& actionContext);

    /*  
     *  Dispatches mapped all inputs received before the target
     * time to any action contexts that can handle them.
     */
    void dispatch(uint32_t targetTimeMillis);
private:
    friend class ActionContext;
    double getRawValue(const InputFilter& inputFilter, const SDL_Event& inputEvent) const;

    /*
     *  Register a listener for a certain input combination on behalf
     * of `forActionContext`.
     */
    void registerInputCombo(const std::string& actionContext, const InputCombo& inputCombo);
    /*
     *  Remove entry for a specific input within an action context.
     */
    void unregisterInputCombo(const std::string& actionContext, const InputCombo& inputCombo);
    /*
     *  Remove entry for all inputs bound within an action context
     */
    void unregisterInputCombos(const std::string& actionContext);
    /*
     *  Remove all input binds
     */
    void unregisterInputCombos();


    typedef std::string ActionContextName;

    // All action context name->objects
    std::unordered_map<ActionContextName, std::pair<ActionContext, ActionContextPriority>> mActionContexts {};

    // The current, raw state of the control+axis associated with each input
    // filter, each between 0.f and 1.f (button controls get 0.f and 1.f
    // when unpressed and unpressed respectively)
    std::unordered_map<InputFilter, double> mRawInputState {};

    // all binds associated with a given input
    std::unordered_map<InputFilter, std::set<InputCombo>> mInputFilterToCombos {};

    // all action contexts associated with a given input combo, organized
    // by priority
    std::unordered_map<InputCombo, std::array<
        std::set<ActionContextName>, ActionContextPriority::TOTAL
    >> mInputComboToActionContexts {};

    // Bind values, up to the most recently fired input trigger
    std::unordered_map<InputCombo, UnmappedInputValue> mInputComboStates {};

    /*  Queue of input state changes, to be consumed by whichever action
     * contexts require them
     */
    std::queue<std::pair<InputCombo, UnmappedInputValue>> mUnmappedInputs {};

    /**
     * Button threshold for axes or buttons that map to modifiers
     */
    float mModifierThreshold { .7f };
};

class IActionHandler {
public:
    /*
     *  The action handling function in any class that implements 
     * this interface. 
     * 
     *  Should return a bool indicating whether the
     * input that triggered it can be propagated to lower
     * precedence action contexts.
     */
    virtual void handleAction(const ActionData& actionData) = 0;
};

class ActionContext {
public:
    ActionContext(InputManager& inputManager, const std::string& name): mInputManager{inputManager}, mName {name} {}
    ActionContext(InputManager&& inputManager, const std::string& name) = delete;

    static ActionData ApplyInput(ActionData actionData, const AxisFilter axisFilter, const UnmappedInputValue& inputValue);

    // Creates an action of the same name and attributes
    void registerAction(const std::string& name, ActionAttributes attributes);
    void unregisterAction(const std::string& name);

    void registerActionHandler(const std::string& action, std::weak_ptr<IActionHandler> actionHandler);
    void unregisterActionHandler(const std::string& action, std::weak_ptr<IActionHandler> actionHandler);
    void unregisterActionHandler(std::weak_ptr<IActionHandler> actionHandler);

    void registerInputBind(const std::string& forAction, AxisFilter onAxis, const InputCombo& withInput);
    void unregisterInputBind(const InputCombo& inputCombo);
    void unregisterInputBinds(const std::string& forAction);
    void unregisterInputBinds();

    void resetActionData(const std::string& forAction, uint32_t timestamp);
    void resetActionData(uint32_t timestamp);

    void dispatch();

    // Maps the given input value to its assigned action state
    void mapToAction(const UnmappedInputValue& inputValue, const InputCombo& inputCombo);

    inline bool allowPropagate() { return mPropagateInput; };
    inline void setAllowPropagate(bool allowPropagate) { mPropagateInput = allowPropagate; }

    inline bool enabled() { return mEnabled; }
    inline bool setEnabled(bool enable)  { mEnabled = enable; }

private:
    void mapToContinuousAction(const UnmappedInputValue& inputValue, const InputCombo& inputCombo);
    void mapToTriggeredAction(const UnmappedInputValue& inputValue, const InputCombo& inputCombo);

    /* 
     *  Reference to the input manager that created this 
     * action context. 
     * 
     * (apparently the technical term for this is "dependency injection
     * via constructor")
     */
    InputManager& mInputManager;

    /*
     * The name of this action context
     */
    const std::string& mName;

    bool mEnabled { true };
    bool mPropagateInput { false };

    /*
     *  All actions defined for this context, and their present states
     */
    std::unordered_map<ActionDefinition, ActionData> mActions {};

    /*
     * Action state changes that have occurred over the course of the 
     * current frame, up until the present moment, for actions of type 
     * multiaxis
     */
    std::unordered_map<ActionDefinition, ActionData> mPendingContinuousActions {};

    /*
     * Action state changes that correspond to triggers and toggles
     */
    std::vector<std::pair<ActionDefinition, ActionData>> mPendingTriggeredActions {};

    /*
     *  All input bindings associated with a specific action
     */
    std::unordered_map<ActionDefinition, std::set<InputCombo>> mActionToInputBinds {};

    /*
     *  Mapping from unmapped input controls, provided by the input manager, to
     * their associated action definitions.
     */
    std::unordered_map<InputCombo, std::pair<AxisFilter, ActionDefinition>> mInputBindToAction {};

    /*
     * Pointers to all action handler instances waiting for a particular action
     */
    std::unordered_map<ActionDefinition, std::set<std::weak_ptr<IActionHandler>>> mActionHandlers {};
};

#endif
