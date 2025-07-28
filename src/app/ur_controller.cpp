#include <iostream>

#include "ur_records.hpp"
#include "ur_controller.hpp"


std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrController::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrController> controller { new UrController{} };
    return controller;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrController::clone() const {
    return std::shared_ptr<UrController>( new UrController {} );
}

bool UrController::viewUpdatesComplete() const {
    for(auto viewUpdatePair: mViewUpdated) {
        if(!viewUpdatePair.second) { return false; }
    }
    return true;
}

void UrController::onViewSubscribed(const std::string& subscriber) {
    mViewUpdated[subscriber] = false;
}

void UrController::onViewUpdatesCompleted(const std::string& viewName) {
    mViewUpdated.at(viewName) = true;
    if(!viewUpdatesComplete()) { return; }

    mSigMovePrompted.emit(mModel.getCurrentPhase());
}

void UrController::onActivated() {
    mSigControllerReady.emit();

    mSigScoreUpdated.emit(mModel.getScore());
    mSigPlayerUpdated.emit(mModel.getPlayerData(PlayerID::PLAYER_A));
    mSigPlayerUpdated.emit(mModel.getPlayerData(PlayerID::PLAYER_B));
    mSigPhaseUpdated.emit(mModel.getCurrentPhase());
    mSigDiceUpdated.emit(mModel.getDiceData());

    for(const auto& view: mViewUpdated) {
        mViewUpdated[view.first] = false;
    }
    mSigViewUpdateStarted.emit();
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

    for(const auto& view: mViewUpdated) {
        mViewUpdated[view.first] = false;
    }
    mSigViewUpdateStarted.emit();
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

    if(mModel.getCurrentPhase().mGamePhase == GamePhase::END) {
        getSimObject().getWorld().lock()
            ->getSingletonSystem<ToyMakersEngine::SceneSystem>()
            ->getByPath<UrRecords&>(
                "/ur_records/@UrRecords"
            ).submitRecord(
                GameRecord{
                    .mSummary { mModel.getScore() },
                    .mPlayerA { mModel.getPlayerData(PlayerID::PLAYER_A) },
                    .mPlayerB { mModel.getPlayerData(PlayerID::PLAYER_B) },
                }
        );
    }

    for(const auto& view: mViewUpdated) {
        mViewUpdated[view.first] = false;
    }
    mSigViewUpdateStarted.emit();
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

    for(const auto& view: mViewUpdated) {
        mViewUpdated[view.first] = false;
    }
    mSigViewUpdateStarted.emit();
}

void UrController::onDiceRollAttempted(PlayerID player) {
    if(!mModel.canRollDice(player)) { return; }

    mModel.rollDice(player);
    mSigDiceUpdated.emit(mModel.getDiceData());
    mSigPhaseUpdated.emit(mModel.getCurrentPhase());

    for(const auto& view: mViewUpdated) {
        mViewUpdated[view.first] = false;
    }
    mSigViewUpdateStarted.emit();
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
