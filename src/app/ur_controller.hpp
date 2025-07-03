#ifndef ZOAPPURCONTROLLER_H
#define ZOAPPURCONTROLLER_H

#include <glm/gtx/string_cast.hpp>

#include "../engine/sim_system.hpp"

#include "game_of_ur_data/model.hpp"

class UrPlayerControls;

class UrController: public ToyMakersEngine::SimObjectAspect<UrController> {
public:
    UrController(): SimObjectAspect<UrController>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrController"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    inline const GameOfUrModel& getModel() const { return mModel; }

    std::unique_ptr<UrPlayerControls> createControls();

private:
    GameOfUrModel mModel {};
    uint8_t mControlsCreated {0};

    void onLaunchPieceAttempted(PlayerID player, PieceIdentity piece, glm::u8vec2 launchLocation=glm::u8vec2{0,0});
    void onMoveBoardPieceAttempted(PlayerID player, PieceIdentity piece);
    void onNextTurnAttempted(PlayerID player);
    void onDiceRollAttempted(PlayerID player);

    void onActivated() override;

public:
    ToyMakersEngine::Signal<GamePhaseData> mSigPhaseUpdated { *this, "PhaseUpdated" };
    ToyMakersEngine::Signal<GameScoreData> mSigScoreUpdated { *this, "ScoreUpdated" };
    ToyMakersEngine::Signal<PlayerData> mSigPlayerUpdated { *this, "PlayerUpdated" };
    ToyMakersEngine::Signal<DiceData> mSigDiceUpdated { *this, "DiceUpdated" };
    ToyMakersEngine::Signal<MoveResultData> mSigMoveMade { *this, "MoveMade" };

friend class UrPlayerControls;
};

class UrPlayerControls {
public:
    inline PlayerID getPlayer() const { return mPlayer; }

    void attemptLaunchPiece(PieceTypeID pieceType, glm::u8vec2 launchLocation=glm::u8vec2{0,0});
    void attemptMoveBoardPiece(PieceIdentity piece);
    void attemptNextTurn();
    void attemptDiceRoll();

    inline const GameOfUrModel& getModel() { return mUrController.getModel(); }
private:
    inline static std::unique_ptr<UrPlayerControls> create(PlayerID player, UrController& urController) {
        return std::unique_ptr<UrPlayerControls>(new UrPlayerControls{ player, urController });
    }
    UrPlayerControls(PlayerID player, UrController& urController): mPlayer{ player }, mUrController { urController } {}

    PlayerID mPlayer;
    UrController& mUrController;

friend class UrController;
};

#endif
