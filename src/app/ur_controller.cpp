#include <iostream>

#include "ur_controller.hpp"


std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrController::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrController> controller { std::make_shared<UrController>() };
    return controller;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrController::clone() const {
    return std::make_shared<UrController>();
}

void UrController::onActivated() {
    mSigScoreUpdated.emit(mModel.getScore());
    mSigPhaseUpdated.emit(mModel.getCurrentPhase());
    mSigDiceUpdated.emit(mModel.getDiceData());
}

void UrController::onLaunchPieceAttempted(PlayerID player, PieceIdentity piece, glm::u8vec2 launchLocation) {
    if(!mModel.canLaunchPiece(piece, launchLocation, player)) return;

    const MoveResultData moveResults { mModel.getLaunchMoveData(piece, launchLocation) };
    mModel.movePiece(piece, launchLocation, player);

    if(moveResults.mDisplacedPiece.mIdentity.mOwner != RoleID::NA) {
        mSigPlayerUpdated.emit(mModel.getPlayerData(moveResults.mDisplacedPiece.mIdentity.mOwner));
    }

    mSigPlayerUpdated.emit(mModel.getPlayerData(moveResults.mMovedPiece.mIdentity.mOwner));
    mSigMoveMade.emit(moveResults);
    mSigPhaseUpdated.emit(mModel.getCurrentPhase());
}

void UrController::onMoveBoardPieceAttempted(PlayerID player, PieceIdentity piece) {
    if(!mModel.canMoveBoardPiece(piece, player)) return;

    const MoveResultData moveResults { mModel.getBoardMoveData(piece) };
    mModel.movePiece(piece, moveResults.mMovedPiece.mLocation, player);

    if(moveResults.mDisplacedPiece.mIdentity.mOwner != RoleID::NA) {
        mSigPlayerUpdated.emit(mModel.getPlayerData(moveResults.mDisplacedPiece.mIdentity.mOwner));
    }
    if(moveResults.mCountersLost || moveResults.mCountersWon) {
        mSigScoreUpdated.emit(mModel.getScore());
        mSigPlayerUpdated.emit(mModel.getPlayerData(moveResults.mMovedPiece.mIdentity.mOwner));
    }
    mSigMoveMade.emit(moveResults);
    mSigPhaseUpdated.emit(mModel.getCurrentPhase());
}

void UrController::onNextTurnAttempted(PlayerID player) {
    if(!mModel.canAdvanceOneTurn(player) && !mModel.canStartPhasePlay()) {
        return;
    }

    const bool canAdvanceOneTurn { mModel.canAdvanceOneTurn(player) };
    if(canAdvanceOneTurn) { mModel.advanceOneTurn(player); }
    else { mModel.startPhasePlay(); }

    mSigPhaseUpdated.emit(mModel.getCurrentPhase());
    mSigDiceUpdated.emit(mModel.getDiceData());
    if(canAdvanceOneTurn) { return; }

    mSigPlayerUpdated.emit(mModel.getPlayerData(PlayerID::PLAYER_A));
    mSigPlayerUpdated.emit(mModel.getPlayerData(PlayerID::PLAYER_B));
    mSigScoreUpdated.emit(mModel.getScore());
}

void UrController::onDiceRollAttempted(PlayerID player) {
    if(!mModel.canRollDice(player)) { return; }

    mModel.rollDice(player);
    mSigDiceUpdated.emit(mModel.getDiceData());
    mSigPhaseUpdated.emit(mModel.getCurrentPhase());
}
