#ifndef ZOAPPPLAYERLOCAL_H
#define ZOAPPPLAYERLOCAL_H

#include "toymaker/sim_system.hpp"

#include "ur_controller.hpp"

class PlayerLocal: public ToyMaker::SimObjectAspect<PlayerLocal> {
public:
    PlayerLocal(): SimObjectAspect<PlayerLocal>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrPlayerLocal"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

private:
    std::string mControllerPath {};
    std::unique_ptr<UrPlayerControls> mControls {};

    void onActivated() override;

    void onLaunchPieceAttempted(PieceTypeID pieceType, glm::u8vec2 location);
    void onNextTurnAttempted();
    void onDiceRollAttempted();
    void onMoveBoardPieceAttempted(PieceIdentity piece);
    void onMovePrompted(GamePhaseData phaseData);

public:
    ToyMaker::SignalObserver<GamePhaseData> mObserveMovePrompted {
        *this, "MovePromptedObserved",
        [this](GamePhaseData phaseData) {this->onMovePrompted(phaseData);}
    };

    ToyMaker::SignalObserver<PieceTypeID, glm::u8vec2> mObservePieceLaunchAttempted {
        *this, "LaunchPieceAttemptedObserved",
        [this](PieceTypeID pieceType, glm::u8vec2 location) { this->onLaunchPieceAttempted(pieceType, location); }
    };
    ToyMaker::SignalObserver<PieceIdentity> mObserveMovePieceAttempted {
        *this, "MovePieceAttemptedObserved",
        [this](PieceIdentity piece) { this->onMoveBoardPieceAttempted(piece); }
    };
    ToyMaker::SignalObserver<> mObserveEndTurnAttempted {
        *this, "NextTurnAttemptedObserved",
        [this]() { this->onNextTurnAttempted(); }
    };
    ToyMaker::SignalObserver<> mObserveDiceRollAttempted {
        *this, "DiceRollAttemptedObserved",
        [this]() { this->onDiceRollAttempted(); }
    };

    ToyMaker::Signal<PlayerID> mSigControlInterface {*this, "ControlInterface"};
};

#endif
