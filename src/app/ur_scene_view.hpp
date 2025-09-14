/**
 * @file ur_scene_view.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains the definition of the class representing and controlling the 3D scene in which the game is played.
 * @version 0.3.2
 * @date 2025-09-13
 * 
 * 
 */

#ifndef ZOAPPURSCENEVIEW_H
#define ZOAPPURSCENEVIEW_H

#include "toymaker/sim_system.hpp"
#include "toymaker/signals.hpp"
#include "toymaker/text_render.hpp"

#include "board_locations.hpp"
#include "game_of_ur_data/model.hpp"

struct UrPieceAnimationKey;

bool operator<(const UrPieceAnimationKey& one, const UrPieceAnimationKey& two);

/**
 * @brief The class responsible for managing the 3D representation and interactions of a game in progress.
 * 
 * Displays the game board and adds game pieces to it per the current state of the game.  The state of the game is read from the game data model (or from changes broadcasted by UrController).
 * 
 * Converts board or game piece clicks into actions usable by the currently active player controller.  Also reports view updates (like animations that play after a game state change) to the game controller.
 * 
 */
class UrSceneView: public ToyMaker::SimObjectAspect<UrSceneView> {
public:
    UrSceneView(): ToyMaker::SimObjectAspect<UrSceneView>{0} {}

    inline static std::string getSimObjectAspectTypeName() { return "UrSceneView"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    const GameOfUrModel& getModel() const;
    const BoardLocations& getBoard() const;

private:
    enum class Mode {
        GENERAL,
        LAUNCH_POSITION_SELECTION,
        TRANSITION,
    };

    std::map<PieceIdentity, std::string> mPieceModelMap {};
    std::map<PieceIdentity, std::shared_ptr<ToyMaker::SceneNode>> mPieceNodeMap {};
    std::weak_ptr<ToyMaker::SimObject> mGameOfUrController {};
    std::weak_ptr<ToyMaker::SimObject> mGameOfUrBoard {};
    std::priority_queue<UrPieceAnimationKey> mAnimationKeys {};
    uint32_t mAnimationTimeMillis {};

    std::string mControllerPath {};
    Mode mMode { Mode::GENERAL };
    PlayerID mControlledBy {};

    void onControllerReady();

    void onBoardClicked(glm::u8vec2 boardLocation);
    void onLaunchPieceInitiated(PieceTypeID piece);
    void onLaunchPieceCanceled();
    void onMoveMade(const MoveResultData& moveResultData);

    void onViewUpdateStarted();
    void onControlInterface(PlayerID player);

    void onActivated() override;
    void variableUpdate(uint32_t variableStepMillis) override;

public:
    ToyMaker::SignalObserver<glm::u8vec2> mObserveBoardClicked { 
        *this, "BoardClickedObserved",
        [this](glm::u8vec2 boardLocation) { this->onBoardClicked(boardLocation); }
    };
    ToyMaker::SignalObserver<PieceTypeID> mObserveLaunchPieceInitiated {
        *this, "LaunchPieceInitiatedObserved",
        [this](PieceTypeID pieceType) { this->onLaunchPieceInitiated(pieceType); }
    };
    ToyMaker::SignalObserver<> mObserveLaunchPieceCancelled {
        *this, "LaunchPieceCanceledObserved",
        [this]() { this->onLaunchPieceCanceled(); }
    };
    ToyMaker::SignalObserver<MoveResultData> mObserveMoveMade {
        *this, "MoveMadeObserved",
        [this](const MoveResultData& moveResultData) { this->onMoveMade(moveResultData); }
    };
    ToyMaker::SignalObserver<PlayerID> mObserveControlInterface {
        *this, "ControlInterfaceObserved",
        [this](PlayerID playerID) { this->onControlInterface(playerID); }
    };

    ToyMaker::Signal<PieceTypeID, glm::u8vec2> mSigLaunchPieceAttempted {
        *this, "LaunchPieceAttempted"
    };
    ToyMaker::Signal<PieceIdentity> mSigMovePieceAttempted {
        *this, "MovePieceAttempted"
    };

    ToyMaker::SignalObserver<> mObserveControllerReady {
        *this, "ControllerReadyObserved",
        [this]() {this->onControllerReady();}
    };
    ToyMaker::Signal<std::string> mSigViewSubscribed {*this, "ViewSubscribed"};

    ToyMaker::SignalObserver<> mObserveViewUpdateStarted { 
        *this, "ViewUpdateStartedObserved",
        [this](){ this->onViewUpdateStarted(); }
    };
    ToyMaker::Signal<std::string> mSigViewUpdateCompleted {
        *this, "ViewUpdateCompleted"
    };
};

struct UrPieceAnimationKey {
    uint32_t mTime;
    PieceIdentity mPieceIdentity;
    ToyMaker::Placement mPlacement;
    bool mRemove { false };
};

#endif
