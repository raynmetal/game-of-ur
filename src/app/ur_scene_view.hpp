#ifndef ZOAPPURSCENEVIEW_H
#define ZOAPPURSCENEVIEW_H

#include "../engine/sim_system.hpp"
#include "../engine/signals.hpp"
#include "../engine/text_render.hpp"

#include "board_locations.hpp"
#include "game_of_ur_data/model.hpp"

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
    };

    std::map<PieceIdentity, std::string> mPieceModelMap {};
    std::map<PieceIdentity, std::shared_ptr<ToyMakersEngine::SceneNode>> mPieceNodeMap {};
    std::weak_ptr<ToyMakersEngine::SimObject> mGameOfUrController {};
    std::weak_ptr<ToyMakersEngine::SimObject> mGameOfUrBoard {};
    std::string mControllerPath {};
    Mode mMode { Mode::GENERAL };
    PlayerID mControlledBy {};

    void onBoardClicked(glm::u8vec2 boardLocation);
    void onLaunchPieceInitiated(PieceTypeID piece);
    void onLaunchPieceCanceled();
    void onMoveMade(const MoveResultData& moveResultData);

    void onControlInterface(PlayerID player);

    void onActivated() override;

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
};

#endif
