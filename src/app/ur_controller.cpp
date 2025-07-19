#include <iostream>

#include "ur_controller.hpp"


std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrController::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrController> controller { new UrController{} };
    return controller;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrController::clone() const {
    return std::shared_ptr<UrController>( new UrController {} );
}

void UrController::onActivated() {
    mSigScoreUpdated.emit(mModel.getScore());
    mSigPlayerUpdated.emit(mModel.getPlayerData(PlayerID::PLAYER_A));
    mSigPlayerUpdated.emit(mModel.getPlayerData(PlayerID::PLAYER_B));
    mSigPhaseUpdated.emit(mModel.getCurrentPhase());
    mSigDiceUpdated.emit(mModel.getDiceData());

    mSigMovePrompted.emit(mModel.getCurrentPhase());
}

std::unique_ptr<UrPlayerControls> UrController::createControls() {
    assert(mControlsCreated < 2 && "There can be no more than 2 player controls in existence created by this controller");
    return UrPlayerControls::create(static_cast<PlayerID>(mControlsCreated++), *this);
}

void UrController::onLaunchPieceAttempted(PlayerID player, PieceIdentity piece, glm::u8vec2 launchLocation) {
    if(!mModel.canLaunchPieceTo(piece, launchLocation, player)) return;

    const MoveResultData moveResults { mModel.getLaunchMoveData(piece, launchLocation) };
    mModel.movePiece(piece, launchLocation, player);

    if(moveResults.mDisplacedPiece.mIdentity.mOwner != RoleID::NA) {
        mSigPlayerUpdated.emit(mModel.getPlayerData(moveResults.mDisplacedPiece.mIdentity.mOwner));
    }

    mSigPlayerUpdated.emit(mModel.getPlayerData(moveResults.mMovedPiece.mIdentity.mOwner));
    mSigMoveMade.emit(moveResults);
    mSigPhaseUpdated.emit(mModel.getCurrentPhase());

    mSigMovePrompted.emit(mModel.getCurrentPhase());
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

    mSigMovePrompted.emit(mModel.getCurrentPhase());
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

    // We've started the play phase; scores and player data must be updated
    if(!canAdvanceOneTurn) {
        mSigPlayerUpdated.emit(mModel.getPlayerData(PlayerID::PLAYER_A));
        mSigPlayerUpdated.emit(mModel.getPlayerData(PlayerID::PLAYER_B));
        mSigScoreUpdated.emit(mModel.getScore());
    }

    mSigMovePrompted.emit(mModel.getCurrentPhase());
}

void UrController::onDiceRollAttempted(PlayerID player) {
    if(!mModel.canRollDice(player)) { return; }

    mModel.rollDice(player);
    mSigDiceUpdated.emit(mModel.getDiceData());
    mSigPhaseUpdated.emit(mModel.getCurrentPhase());
    mSigMovePrompted.emit(mModel.getCurrentPhase());
}

void UrPlayerControls::attemptDiceRoll() {
    mUrController.onDiceRollAttempted(mPlayer);
}
void UrPlayerControls::attemptLaunchPiece(PieceTypeID pieceType, glm::u8vec2 launchLocation) {
    const PieceIdentity piece { .mType { pieceType }, .mOwner { mUrController.getModel().getPlayerData(mPlayer).mRole } };
    mUrController.onLaunchPieceAttempted(mPlayer, piece, launchLocation);
}
void UrPlayerControls::attemptMoveBoardPiece(PieceIdentity piece) {
    mUrController.onMoveBoardPieceAttempted(mPlayer, piece);
}
void UrPlayerControls::attemptNextTurn() {
    mUrController.onNextTurnAttempted(mPlayer);
}
