/**
 * @file ur_controller.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains the class definition for UrController, the aspect responsible for managing and reporting the state of the game.
 * @version 0.3.2
 * @date 2025-09-13
 * 
 * 
 */

/**
 * @defgroup UrGameControlLayer Interface between visual/interactive layer and Ur game data model
 * @ingroup UrGame
 * 
 */

#ifndef ZOAPPURCONTROLLER_H
#define ZOAPPURCONTROLLER_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "toymaker/sim_system.hpp"

#include "game_of_ur_data/model.hpp"

class UrPlayerControls;

/**
 * @ingroup UrGameControlLayer
 * @brief The class responsible for managing and reporting the state of the game.  It owns its own instance of GameOfUrModel, and acts as the interface between it and engine-specific objects.
 * 
 */
class UrController: public ToyMaker::SimObjectAspect<UrController> {
public:
    /**
     * @brief Constructs a new UrController aspect.
     * 
     */
    UrController(): SimObjectAspect<UrController>{0} {}

    /**
     * @brief Gets the aspect type string associated with this object.
     * 
     * @return std::string The aspect type string associated with this class.
     */
    inline static std::string getSimObjectAspectTypeName() { return "UrController"; }

    /**
     * @brief Creates an UrController based on its JSON description.
     * 
     * @param jsonAspectProperties The JSON description of this instance of UrController.
     * @return std::shared_ptr<BaseSimObjectAspect> The newly constructed aspect.
     */
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);

    /**
     * @brief Constructs an UrController object using this one as its blueprint.
     * 
     * @return std::shared_ptr<BaseSimObjectAspect> The newly constructed UrController aspect.
     */
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    /**
     * @brief Gets a const reference to the underlying data model of this object.
     * 
     * @return const GameOfUrModel& A const reference to the underlying game data model.
     */
    inline const GameOfUrModel& getModel() const { return mModel; }

    /**
     * @brief Creates an instance of UrPlayerControls, which exposes methods which allow (controlled) interaction with this aspect, corresponding with an ID known by the underlying data model.
     * 
     * No more than two such player controls can exist for a single instance of this controller.
     * 
     * @return std::unique_ptr<UrPlayerControls> The interface between this controller and a player of the game.
     * 
     */
    std::unique_ptr<UrPlayerControls> createControls();

private:
    /**
     * @brief The game data model this aspect is an interface to.
     * 
     */
    GameOfUrModel mModel {};

    /**
     * @brief The number of controls connected with this object created over its lifetime.
     * 
     * More than two is an error.
     * 
     */
    uint8_t mControlsCreated {0};

    /**
     * @brief The path to the scene manager, responsible for navigating between the different game screens in this project.
     * 
     */
    std::string mSceneManagerPath {};

    /**
     * @brief Flags corresponding with various views subscribed with this object.
     * 
     * All the subscribed views must report that their update has taken place before the controller allows more game events to occur.
     * 
     */
    std::map<std::string, bool> mViewUpdated {};

    /**
     * @brief Returns whether all the views connected with this controller have reported completion of their updates.
     * 
     * @retval true All associated views have reported completion of their updates.
     * @retval false One or more views have not reported completion of their updates.
     */
    bool viewUpdatesComplete() const;


    /**
     * @brief Adds a view which is interested in receiving information about events regarding changes in the state of the game.
     * 
     * @param subscriber A view interested in game-state changes.
     */
    void onViewSubscribed(const std::string& subscriber);

    /**
     * @brief Attempts to launch a game piece to the board.
     * 
     * @param player The player requesting the launch.
     * @param piece The piece being launched.
     * @param launchLocation The game board grid location to which the player wants the piece launched.
     */
    void onLaunchPieceAttempted(PlayerID player, PieceIdentity piece, glm::u8vec2 launchLocation=glm::u8vec2{0,0});

    /**
     * @brief Attempts to move a piece present on the board according to the current roll.
     * 
     * @param player The player requesting the move.
     * @param piece The piece on the game board being moved.
     */
    void onMoveBoardPieceAttempted(PlayerID player, PieceIdentity piece);

    /**
     * @brief Attempts to end the current turn and move to the next one.
     * 
     * @param player The player requesting the end of the turn.
     */
    void onNextTurnAttempted(PlayerID player);

    /**
     * @brief Attempts to roll the dice.
     * 
     * @param player The player requesting the dice roll.
     */
    void onDiceRollAttempted(PlayerID player);

    /**
     * @brief The callback, invoked by a subscribed view, where the view reports that it has completed all view changes related to the latest change in game state.
     * 
     * @param viewName The name of the view which has completed its view updates.
     */
    void onViewUpdatesCompleted(const std::string& viewName);

    /**
     * @brief Initializes the game data model.
     * 
     */
    void onActivated() override;

public:
    /**
     * @brief Signal emitted by this object when it is ready to be interacted with by other objects.
     * 
     */
    ToyMaker::Signal<> mSigControllerReady { *this, "ControllerReady" };

    /**
     * @brief Observer to a signal emitted by views interested in events broadcasted by this object.
     * 
     */
    ToyMaker::SignalObserver<const std::string&> mObserveViewSubscribed { 
        *this, "ViewSubscribedObserved",
        [this](const std::string& viewName) {this->onViewSubscribed(viewName);}
    };

    /**
     * @brief Signal emitted when a the game data model reports a change to its phase.
     * 
     */
    ToyMaker::Signal<GamePhaseData> mSigPhaseUpdated { *this, "PhaseUpdated" };

    /**
     * @brief Signal emitted when the game data model reports a change in score.
     * 
     */
    ToyMaker::Signal<GameScoreData> mSigScoreUpdated { *this, "ScoreUpdated" };

    /**
     * @brief Signal emitted when information associated with a player changes.
     * 
     */
    ToyMaker::Signal<PlayerData> mSigPlayerUpdated { *this, "PlayerUpdated" };

    /**
     * @brief Signal emitted when the game data model reports a change in the state of its dice.
     * 
     */
    ToyMaker::Signal<DiceData> mSigDiceUpdated { *this, "DiceUpdated" };

    /**
     * @brief Signal emitted when the game data model reports a move successfully executed.
     * 
     */
    ToyMaker::Signal<MoveResultData> mSigMoveMade { *this, "MoveMade" };

    /**
     * @brief Signal emitted when this controller begins waiting for view updates related to the latest game state change to complete.
     * 
     */
    ToyMaker::Signal<> mSigViewUpdateStarted { *this, "ViewUpdateStarted" };

    /**
     * @brief Observer which receives signals from its subscribed views when they report their updates are complete.
     * 
     */
    ToyMaker::SignalObserver<const std::string&> mObserveViewUpdateCompleted {
        *this, "ViewUpdateCompletedObserved",
        [this](const std::string& viewName) { this->onViewUpdatesCompleted(viewName); }
    };

    /**
     * @brief Signal emitted when the controller expects a player to act, either via the game UI or some other source.
     * 
     */
    ToyMaker::Signal<GamePhaseData> mSigMovePrompted { *this, "MovePrompted" };
friend class UrPlayerControls;
};

/**
 * @ingroup UrGameControlLayer
 * @brief The definition for the object that acts as the interface between the game controller and any systems or objects interested in interacting with it.
 * 
 * Contains methods for prompting updates in the state of the game data model managed by the controller.  Each instance of this represents a single player playing the game.
 * 
 */
class UrPlayerControls {
public:
    /**
     * @brief Gets the ID of the player associated with this object.
     * 
     * @return PlayerID The player using this object.
     */
    inline PlayerID getPlayer() const { return mPlayer; }

    /**
     * @brief Attempts to launch a game piece to some location on the game board.
     * 
     * @param pieceType The type of piece being launched.
     * @param launchLocation The location on the board to which the piece is being launched.
     */
    void attemptLaunchPiece(PieceTypeID pieceType, glm::u8vec2 launchLocation=glm::u8vec2{0,0});

    /**
     * @brief Attempts to move a piece already on the board to a new location based on the current dice roll.
     * 
     * @param piece The type of piece being moved.
     */
    void attemptMoveBoardPiece(PieceIdentity piece);

    /**
     * @brief Attempts to end the current turn and begin the next one.
     * 
     */
    void attemptNextTurn();

    /**
     * @brief Attempts to roll the dice.
     * 
     */
    void attemptDiceRoll();

    /**
     * @brief Gets a constant reference to the underlying data model used by UrController.
     * 
     * @return const GameOfUrModel& A constant reference to the underlying game data model.
     */
    inline const GameOfUrModel& getModel() { return mUrController.getModel(); }
private:
    /**
     * @brief Creates an instance of UrPlayerControls.
     * 
     * @param player The player to whom these controls belong.
     * @param urController The reference to the game controller connected with these controls.
     * @return std::unique_ptr<UrPlayerControls> The newly created instance.
     */
    inline static std::unique_ptr<UrPlayerControls> create(PlayerID player, UrController& urController) {
        return std::unique_ptr<UrPlayerControls>(new UrPlayerControls{ player, urController });
    }
    /**
     * @brief Constructs a new UrPlayerControls object.
     * 
     * @param player The player to whom these controls belong.
     * @param urController The reference to the game controller connected with these controls.
     */
    UrPlayerControls(PlayerID player, UrController& urController): mPlayer{ player }, mUrController { urController } {}

    /**
     * @brief The ID of the player to whom these controls belong.
     * 
     */
    PlayerID mPlayer;

    /**
     * @brief The reference to the game controller connected with these controls.
     * 
     */
    UrController& mUrController;

friend class UrController;
};

#endif
