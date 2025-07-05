#ifndef ZOAPPPLAYERLOCAL_H
#define ZOAPPPLAYERLOCAL_H

#include "../engine/sim_system.hpp"

#include "ur_controller.hpp"

class PlayerLocal: public ToyMakersEngine::SimObjectAspect<PlayerLocal> {
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

public:
    ToyMakersEngine::SignalObserver<PieceTypeID, glm::u8vec2> mObservePieceLaunchAttempted {
        *this, "LaunchPieceAttemptedObserved",
        [this](PieceTypeID pieceType, glm::u8vec2 location) { this->onLaunchPieceAttempted(pieceType, location); }
    };
    ToyMakersEngine::SignalObserver<PieceIdentity> mObserveMovePieceAttempted {
        *this, "MovePieceAttemptedObserved",
        [this](PieceIdentity piece) { this->onMoveBoardPieceAttempted(piece); }
    };
    ToyMakersEngine::SignalObserver<> mObserveEndTurnAttempted {
        *this, "NextTurnAttemptedObserved",
        [this]() { this->onNextTurnAttempted(); }
    };
    ToyMakersEngine::SignalObserver<> mObserveDiceRollAttempted {
        *this, "DiceRollAttemptedObserved",
        [this]() { this->onDiceRollAttempted(); }
    };
};

#endif
