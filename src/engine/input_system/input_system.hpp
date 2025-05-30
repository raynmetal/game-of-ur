#ifndef FOOLSENGINE_INPUTSYSTEM_H
#define FOOLSENGINE_INPUTSYSTEM_H

#include <string>
#include <map>
#include <queue>
#include <memory>

#include <nlohmann/json.hpp>
#include <SDL2/SDL.h>

#include "input_data.hpp"

namespace ToyMakersEngine {
    class ActionContext;
    class ActionDispatch;
    class InputManager;
    /*
    *   Processes raw SDL input events into unmapped inputs, and later
    * reports bind value changes to an ActionContext for conversion into
    * corresponding action events
    */
    class InputManager {
    public:
        enum ActionContextPriority: uint8_t {
            VERY_LOW=0,
            LOW=1,
            DEFAULT=2,
            HIGH=3,
            VERY_HIGH=4,
            TOTAL=5,
        };

        /* 
        * Maps an event to its internal representation, if one is available 
        */
        void queueInput(const SDL_Event& inputEvent);

        void registerInputBind(const nlohmann::json& inputBindingParameters);
        void registerAction(const nlohmann::json& actionParameters);
        void loadInputConfiguration(const nlohmann::json& inputConfiguration);

        /*
        *  Registers a new action context with a given name
        */
        void registerActionContext(const ::std::string& name, ActionContextPriority priority=DEFAULT);
        /*
        *  Removes the action context associated with this name
        */
        void unregisterActionContext(const ::std::string& name);

        ActionContext& operator[] (const ::std::string& actionContext);

        /*  
        *  Dispatches mapped all inputs received before the target
        * time to any action contexts that can handle them.
        */
        ::std::vector<::std::pair<ActionDefinition, ActionData>> getTriggeredActions(uint32_t targetTimeMillis);

    private:
        friend class ActionContext;
        double getRawValue(const InputFilter& inputFilter, const SDL_Event& inputEvent) const;

        /*
        *  Register a listener for a certain input combination on behalf
        * of `forActionContext`.
        */
        void registerInputCombo(const ::std::string& actionContext, const InputCombo& inputCombo);
        /*
        *  Remove entry for a specific input within an action context.
        */
        void unregisterInputCombo(const ::std::string& actionContext, const InputCombo& inputCombo);
        /*
        *  Remove entry for all inputs bound within an action context
        */
        void unregisterInputCombos(const ::std::string& actionContext);
        /*
        *  Remove all input binds
        */
        void unregisterInputCombos();

        /**  
         * All action context name->objects
         */
        ::std::unordered_map<ContextName, ::std::pair<ActionContext, ActionContextPriority>> mActionContexts {};

        /**
         * The current, raw state of the control+axis associated with each input
         * filter, each between 0.f and 1.f (button controls get 0.f and 1.f
         * when unpressed and pressed respectively)
         */
        ::std::unordered_map<InputFilter, double> mRawInputState {};

        /**
         * All active input combinations associated with a given input
         */
        ::std::unordered_map<InputFilter, ::std::set<InputCombo>> mInputFilterToCombos {};

        /** 
         * All action contexts associated with a given input combination, organized
         * by priority
        */
        ::std::unordered_map<InputCombo, ::std::array<
            ::std::set<ContextName>, ActionContextPriority::TOTAL
        >> mInputComboToActionContexts {};

        /**
         * Input combination values, up to the most recently fired input trigger
         */
        ::std::unordered_map<InputCombo, UnmappedInputValue> mInputComboStates {};

        /**
         *  Queue of input state changes, to be consumed by whichever action
         * contexts require them
         */
        ::std::queue<::std::pair<InputCombo, UnmappedInputValue>> mUnmappedInputs {};

        /**
         * Button threshold for axes or buttons that map to modifiers
         */
        float mModifierThreshold { .7f };
    };

    /**
     * Class interface for systems that wish to be notified when action events
     * occur in an action context
     */
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

    private:
        virtual bool handleAction(const ActionData& actionData, const ActionDefinition& actionDefinition) {return false;};

    friend class ActionDispatch;
    };

    class ActionContext {
    public:
        ActionContext(InputManager& inputManager, const ContextName& name): mInputManager{inputManager}, mName {name} {}
        ActionContext(InputManager&& inputManager, const ContextName& name) = delete;

        // Apply the input value to its target action-axis combination
        static ActionData ApplyInput(const ActionDefinition& actionDefinition, const ActionData& actionData, const AxisFilter targetAxis, const UnmappedInputValue& inputValue);

        ::std::vector<::std::pair<ActionDefinition, ActionData>> getTriggeredActions();

        // Creates an action of the same name and attributes
        void registerAction(const ActionName& name, InputAttributesType attributes);
        void registerAction(const nlohmann::json& actionParameters);
        // Remove the action with this name
        void unregisterAction(const ActionName& name);

        // Register a binding from an input-sign-axis-modifier combination to a specific axis
        // of the action named.
        void registerInputBind(const ActionName& forAction, AxisFilter targetAxis, const InputCombo& withInput);
        void registerInputBind(const nlohmann::json& inputBindParameters);
        // Remove the binding from this input-sign-axis-modifier combination to whatever
        // action it's bound to
    void unregisterInputBind(const InputCombo& inputCombo);
        // Remove all bindings that map to any axis for this action
        void unregisterInputBinds(const ActionName& forAction);
        // Remove all input combo -> action bindings
        void unregisterInputBinds();

        // Check whether this context allows propagation to lower priority contexts
        inline bool propagateAllowed() { return mPropagateInput; };
        // Enable or disable input propagation to lower priority contexts
        inline void setPropagateAllowed(bool allowPropagate) { mPropagateInput = allowPropagate; }

        // Check whether this context is active and able to process input events
        inline bool enabled() { return mEnabled; }
        // Enable or disable this context, allowing it to or preventing it from 
        // receiving input events
        inline void setEnabled(bool enable)  { mEnabled = enable; }

    private:
        // Set all action data for this action to 0.f or false, and queue a RESET action
        void resetActionData(const ActionName& forAction, uint32_t timestamp);
        // Set all action data to 0.f or false, queue RESET actions for
        // each action
        void resetActionData(uint32_t timestamp);

        // Maps the given input value to its assigned action state
        void mapToAction(const UnmappedInputValue& inputValue, const InputCombo& inputCombo);

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
        const ContextName& mName;

        /**
         * Determines whether this action context is active and allowed to
         * process any bound input events
         */
        bool mEnabled { true };
        /**
         * Determines whether, after mapping an input event to its corresponding 
         * action, other contexts waiting for the input event are allowed to have
         * a go at processing it also
         */
        bool mPropagateInput { false };

        /**
         * All actions defined for this context and their most recently triggered state
         */
        ::std::unordered_map<ActionDefinition, ActionData> mActions {};

        /**
         * Action state changes that have recently been triggered, in the order that they
         * were triggered
         */
        ::std::vector<::std::pair<ActionDefinition, ActionData>> mPendingTriggeredActions {};

        /**
         *  All input bindings associated with a specific action
         */
        ::std::unordered_map<ActionDefinition, ::std::set<InputCombo>> mActionToInputBinds {};

        /**
         *  Mapping from unmapped input controls, provided by the input manager, to
         * their associated action definitions.
         */
        ::std::unordered_map<InputCombo, ::std::pair<AxisFilter, ActionDefinition>> mInputBindToAction {};

    friend class InputManager;
    };

    class ActionDispatch {
    public:
        void registerActionHandler(const QualifiedActionName& contextActionPair, ::std::weak_ptr<IActionHandler> actionHandler);
        void unregisterActionHandler(const QualifiedActionName& contextActionPair, ::std::weak_ptr<IActionHandler> actionHandler);
        void unregisterActionHandler(::std::weak_ptr<IActionHandler> actionHandler);

        bool dispatchAction(const ::std::pair<ActionDefinition, ActionData>& pendingAction);
    private:

        /**
         * Pointers to all action handler instances waiting for a particular action
         */
        ::std::map<QualifiedActionName, ::std::set<::std::weak_ptr<IActionHandler>, ::std::owner_less<::std::weak_ptr<IActionHandler>>>, ::std::less<QualifiedActionName>> mActionHandlers {};
    };

}

#endif
