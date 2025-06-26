#ifndef ZOAPPURSCENEVIEW_H
#define ZOAPPURSCENEVIEW_H

#include "../engine/sim_system.hpp"
#include "../engine/signals.hpp"
#include "../engine/text_render.hpp"

#include "game_of_ur_data/model.hpp"

class UrSceneView: public ToyMakersEngine::SimObjectAspect<UrSceneView> {
public:
    UrSceneView(): ToyMakersEngine::SimObjectAspect<UrSceneView>{0} {}

    inline static std::string getSimObjectAspectTypeName() { return "UrSceneView"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;
    const GameOfUrModel& getModel() const;

private:
    enum class Mode {
        GENERAL,
        LAUNCH_SWALLOW,
    };

    std::map<PieceIdentity, std::string> mPieceModelMap {};
    std::map<PieceIdentity, std::shared_ptr<ToyMakersEngine::SceneNode>> mPieceNodeMap {};
    std::weak_ptr<ToyMakersEngine::SimObject> mGameOfUrController {};
    std::string mControllerPath {};
    PlayerID mCurrentPlayer { PlayerID::PLAYER_A };
    Mode mMode { Mode::GENERAL };

    void onBoardClicked(glm::u8vec2 boardLocation);
    void onLaunchPieceInitiated(PieceTypeID piece);
    void onLaunchPieceCanceled();
    void onMoveMade(const MoveResultData& moveResultData);

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

    ToyMakersEngine::Signal<PlayerID, PieceIdentity, glm::u8vec2> mSigLaunchPieceAttempted {
        *this, "LaunchPieceAttempted"
    };
    ToyMakersEngine::Signal<PlayerID, PieceIdentity> mSigMovePieceAttempted {
        *this, "MovePieceAttempted"
    };
};

#endif
