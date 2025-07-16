#include <iostream>

#include "ur_player_cpu_random.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> PlayerCPURandom::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<PlayerCPURandom> player { new PlayerCPURandom{} };
    player->mControllerPath = jsonAspectProperties.at("controller_path").get<std::string>();
    return player;
}
std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> PlayerCPURandom::clone() const  {
    std::shared_ptr<PlayerCPURandom> player { new PlayerCPURandom{} };
    player->mControllerPath = mControllerPath;
    return player;
}

void PlayerCPURandom::onActivated() {
    assert(!mControls && "We shouldn't have controls assigned yet");
    mControls = (
        ToyMakersEngine::ECSWorld::getSingletonSystem<ToyMakersEngine::SceneSystem>()
            ->getByPath<UrController&>(mControllerPath + "@" + UrController::getSimObjectAspectTypeName()).createControls()
    );
    assert(mControls && "We should have controls assigned now");
}

void PlayerCPURandom::onMovePrompted(GamePhaseData phaseData) {
    // If it isn't our turn to take an action, do nothing
    if(
        phaseData.mGamePhase == GamePhase::END
        || phaseData.mTurn != mControls->getPlayer()
    ) {
        return;
    }

    // We've reached the end of our turn, but the game continues.  Advance 
    // to the next turn
    if(phaseData.mTurnPhase == TurnPhase::END) {
        std::cout << "CPU: ends turn\n" << std::endl;
        mControls->attemptNextTurn();
        return;
    }

    const GameOfUrModel& urModel { mControls->getModel() };

    // it's definitely our turn. Can we roll dice?
    const bool canRollDice {
        urModel.canRollDice(mControls->getPlayer())
    };
    // can we move any of our pieces
    const std::vector<std::pair<PieceIdentity, glm::u8vec2>> possibleMoves {
        urModel.getAllPossibleMoves()
    };

    // select an action among all the actions available to us
    const std::size_t nPossibleActions { (canRollDice?1:0) + possibleMoves.size() };
    const std::size_t selectedMoveIndex { std::uniform_int_distribution<std::size_t>{0, nPossibleActions - 1}(mRandomEngine) };

    // We've decided to roll the dice
    if(selectedMoveIndex == 0 && canRollDice) {
        std::cout << "CPU: rolls dice\n";
        mControls->attemptDiceRoll();
        return;
    }

    // We've opted to launch or move one of our pieces
    const std::pair<PieceIdentity, glm::u8vec2>& selectedMove {
        possibleMoves[selectedMoveIndex - (canRollDice? 1: 0)]
    };
    if(urModel.canLaunchPieceTo(selectedMove.first, selectedMove.second, mControls->getPlayer())) {
        std::cout << "CPU: launches piece\n";
        mControls->attemptLaunchPiece(selectedMove.first.mType, selectedMove.second);
        return;
    } 
    assert(urModel.canMoveBoardPiece(selectedMove.first, mControls->getPlayer()) && "We must be able to make a board move");
    std::cout << "CPU: moves piece on board\n";
    mControls->attemptMoveBoardPiece(selectedMove.first);
}

