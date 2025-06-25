#ifndef ZOAPPURCONTROLLER_H
#define ZOAPPURCONTROLLER_H

#include "glm/gtx/string_cast.hpp"
#include "../engine/sim_system.hpp"

#include "game_of_ur_data/model.hpp"

class UrController: public ToyMakersEngine::SimObjectAspect<UrController> {
public:
    UrController(): SimObjectAspect<UrController>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrController"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    inline const GameOfUrModel& getModel() const { return mModel; }

private:
    GameOfUrModel mModel {};


    void onLaunchPieceAttempted(PlayerID player, PieceIdentity piece, glm::u8vec2 launchLocation=glm::u8vec2{0,0});
    void onMoveBoardPieceAttempted(PlayerID player, PieceIdentity piece);
    void onNextTurnAttempted(PlayerID player);
    void onDiceRollAttempted(PlayerID player);

public:

    ToyMakersEngine::SignalObserver<PlayerID, PieceIdentity, glm::u8vec2> mObservePieceLaunchAttempted {
        *this, "PieceLaunchAttemptedObserved",
        [this](PlayerID player, PieceIdentity piece, glm::u8vec2 location) { this->onLaunchPieceAttempted(player, piece, location); }
    };
    ToyMakersEngine::SignalObserver<PlayerID> mObserveEndTurnAttempted {
        *this, "NextTurnAttemptedObserved",
        [this](PlayerID player) { this->onNextTurnAttempted(player); }
    };
    ToyMakersEngine::SignalObserver<PlayerID> mObserveDiceRollAttempted {
        *this, "DiceRollAttemptedObserved",
        [this](PlayerID player) { this->onDiceRollAttempted(player); }
    };
    ToyMakersEngine::SignalObserver<PlayerID, PieceIdentity> mObserveMovePieceAttempted {
        *this, "MovePieceAttemptedObserved",
        [this](PlayerID player, PieceIdentity piece) { this->onMoveBoardPieceAttempted(player, piece); }
    };

    ToyMakersEngine::Signal<GamePhaseData> mSigPhaseUpdated { *this, "PhaseUpdated" };
    ToyMakersEngine::Signal<GameScoreData> mSigScoreUpdated { *this, "ScoreUpdated" };
    ToyMakersEngine::Signal<PlayerData> mSigPlayerUpdated { *this, "PlayerUpdated" };
    ToyMakersEngine::Signal<DiceData> mSigDiceUpdated { *this, "DiceUpdated" };
    ToyMakersEngine::Signal<MoveResultData> mSigMoveMade { *this, "MoveMade" };
};

#endif
