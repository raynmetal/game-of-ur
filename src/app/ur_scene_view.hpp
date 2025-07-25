#ifndef ZOAPPURSCENEVIEW_H
#define ZOAPPURSCENEVIEW_H

#include "../engine/sim_system.hpp"
#include "../engine/signals.hpp"
#include "../engine/text_render.hpp"

#include "board_locations.hpp"
#include "game_of_ur_data/model.hpp"

struct UrPieceAnimationKey;

bool operator<(const UrPieceAnimationKey& one, const UrPieceAnimationKey& two);

class UrSceneView: public ToyMakersEngine::SimObjectAspect<UrSceneView> {
public:
    UrSceneView(): ToyMakersEngine::SimObjectAspect<UrSceneView>{0} {}

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
    std::map<PieceIdentity, std::shared_ptr<ToyMakersEngine::SceneNode>> mPieceNodeMap {};
    std::weak_ptr<ToyMakersEngine::SimObject> mGameOfUrController {};
    std::weak_ptr<ToyMakersEngine::SimObject> mGameOfUrBoard {};
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
    ToyMakersEngine::SignalObserver<glm::u8vec2> mObserveBoardClicked { 
        *this, "BoardClickedObserved",
        [this](glm::u8vec2 boardLocation) { this->onBoardClicked(boardLocation); }
    };
    ToyMakersEngine::SignalObserver<PieceTypeID> mObserveLaunchPieceInitiated {
        *this, "LaunchPieceInitiatedObserved",
        [this](PieceTypeID pieceType) { this->onLaunchPieceInitiated(pieceType); }
    };
    ToyMakersEngine::SignalObserver<> mObserveLaunchPieceCancelled {
        *this, "LaunchPieceCanceledObserved",
        [this]() { this->onLaunchPieceCanceled(); }
    };
    ToyMakersEngine::SignalObserver<MoveResultData> mObserveMoveMade {
        *this, "MoveMadeObserved",
        [this](const MoveResultData& moveResultData) { this->onMoveMade(moveResultData); }
    };
    ToyMakersEngine::SignalObserver<PlayerID> mObserveControlInterface {
        *this, "ControlInterfaceObserved",
        [this](PlayerID playerID) { this->onControlInterface(playerID); }
    };

    ToyMakersEngine::Signal<PieceTypeID, glm::u8vec2> mSigLaunchPieceAttempted {
        *this, "LaunchPieceAttempted"
    };
    ToyMakersEngine::Signal<PieceIdentity> mSigMovePieceAttempted {
        *this, "MovePieceAttempted"
    };

    ToyMakersEngine::SignalObserver<> mObserveControllerReady {
        *this, "ControllerReadyObserved",
        [this]() {this->onControllerReady();}
    };
    ToyMakersEngine::Signal<std::string> mSigViewSubscribed {*this, "ViewSubscribed"};

    ToyMakersEngine::SignalObserver<> mObserveViewUpdateStarted { 
        *this, "ViewUpdateStartedObserved",
        [this](){ this->onViewUpdateStarted(); }
    };
    ToyMakersEngine::Signal<std::string> mSigViewUpdateCompleted {
        *this, "ViewUpdateCompleted"
    };
};

struct UrPieceAnimationKey {
    uint32_t mTime;
    PieceIdentity mPieceIdentity;
    ToyMakersEngine::Placement mPlacement;
    bool mRemove { false };
};

#endif
