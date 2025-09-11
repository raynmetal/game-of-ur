#include "ur_player_local.hpp"

std::shared_ptr<ToyMaker::BaseSimObjectAspect> PlayerLocal::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<PlayerLocal> player { new PlayerLocal{} };
    player->mControllerPath = jsonAspectProperties.at("controller_path").get<std::string>();
    return player;
}
std::shared_ptr<ToyMaker::BaseSimObjectAspect> PlayerLocal::clone() const  {
    std::shared_ptr<PlayerLocal> player { new PlayerLocal{} };
    player->mControllerPath = mControllerPath;
    return player;
}

void PlayerLocal::onActivated() {
    assert(!mControls && "We shouldn't have controls assigned yet");
    mControls = (
        ToyMaker::ECSWorld::getSingletonSystem<ToyMaker::SceneSystem>()
            ->getByPath<UrController&>(mControllerPath + "@" + UrController::getSimObjectAspectTypeName()).createControls()
    );
    assert(mControls && "We should have controls assigned now");
}

void PlayerLocal::onLaunchPieceAttempted(PieceTypeID pieceType, glm::u8vec2 location) {
    mControls->attemptLaunchPiece(pieceType, location);
}

void PlayerLocal::onNextTurnAttempted() {
    mControls->attemptNextTurn();
}

void PlayerLocal::onDiceRollAttempted() {
    mControls->attemptDiceRoll();
}

void PlayerLocal::onMoveBoardPieceAttempted(PieceIdentity piece) {
    mControls->attemptMoveBoardPiece(piece);
}

void PlayerLocal::onMovePrompted(GamePhaseData phaseData) {
    if(phaseData.mTurn != mControls->getPlayer()) return;
    mSigControlInterface.emit(mControls->getPlayer());
}
