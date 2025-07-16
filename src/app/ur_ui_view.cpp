#include <algorithm>
#include <sstream>
#include <nlohmann/json.hpp>

#include "game_of_ur_data/model.hpp"

#include "ur_controller.hpp"
#include "ui_text.hpp"
#include "ui_button.hpp"

#include "ur_ui_view.hpp"

const std::map<std::string, UrUIView::Buttons> UrUIView::kButtonEnumMap {
    {"swallow", UrUIView::Buttons::SWALLOW},
    {"storm_bird", UrUIView::Buttons::STORMBIRD},
    {"raven", UrUIView::Buttons::RAVEN},
    {"rooster", UrUIView::Buttons::ROOSTER},
    {"eagle", UrUIView::Buttons::EAGLE},

    {"dice_roll", UrUIView::Buttons::DICE},
    {"next_turn", UrUIView::Buttons::NEXT_TURN},
};

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrUIView::clone() const {
    std::shared_ptr<UrUIView> uiView{ std::make_shared<UrUIView>() };
    uiView->mControllerPath = mControllerPath;
    return uiView;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrUIView::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrUIView> uiView { std::make_shared<UrUIView>() };
    uiView->mControllerPath = jsonAspectProperties.at("controller_path").get<std::string>();
    return uiView;
}

void UrUIView::onActivated() {
    mGameOfUrController = (
        ToyMakersEngine::ECSWorld::getSingletonSystem<ToyMakersEngine::SceneSystem>()
        ->getByPath<std::shared_ptr<ToyMakersEngine::SimObject>>(mControllerPath)
    );
}

const GameOfUrModel& UrUIView::getModel() const {
    return mGameOfUrController.lock()->getAspect<UrController>().getModel();
}

void UrUIView::onButtonClicked(const std::string& button) {
    UrUIView::Buttons enumButton { kButtonEnumMap.at(button) };
    std::cout << "UrUIView: ";
    switch(enumButton) {
        case SWALLOW:
        case STORMBIRD:
        case RAVEN:
        case ROOSTER:
        case EAGLE:
            std::cout << "launch piece attempted: " << button << " clicked\n";
            mSigLaunchPieceInitiated.emit(static_cast<PieceTypeID>(enumButton));
            break;
        case DICE:
            std::cout << "dice roll attempted\n";
            mSigDiceRollAttempted.emit();
            break;
        case NEXT_TURN:
            std::cout << "next turn clicked\n";
            mSigNextTurnAttempted.emit();
            break;
    }
}

void UrUIView::onPhaseUpdated(GamePhaseData phase){
    std::cout << "UrUIView: on phase updated\n";
    const std::string playerText { ((phase.mTurn == PlayerID::PLAYER_A)? "player a's turn": "player b's turn") };
    std::stringstream phaseText {};
    switch(phase.mGamePhase) {
        case GamePhase::INITIATIVE:
            phaseText << "initiative ";
            break;
        case GamePhase::PLAY:
            phaseText << "play ";
            break;
        case GamePhase::END:
            phaseText << "end ";
            break;
    }
    phaseText << "phase; turn ";
    switch(phase.mTurnPhase) {
        case TurnPhase::ROLL_DICE:
            phaseText << "roll dice";
            break;
        case TurnPhase::MOVE_PIECE:
            phaseText << "move piece";
            break;
        case TurnPhase::END:
            phaseText << "end";
            break;
    }
    getSimObject().getByPath<UIText&>("/viewport_UI/current_turn/@UIText").updateText(playerText);
    getSimObject().getByPath<UIText&>("/viewport_UI/phase/@UIText").updateText(phaseText.str());

    // enable piece launch buttons only for pieces that are available
    std::vector<PieceTypeID> unlaunchedPieceTypes { getModel().getUnlaunchedPieceTypes(phase.mTurn) };
    for(uint8_t pieceType{ PieceTypeID::SWALLOW }; pieceType < PieceTypeID::TOTAL; ++pieceType) {
        const PieceIdentity currentPlayerPieceIdentity {
            .mType { static_cast<PieceTypeID>(pieceType) },
            .mOwner { getModel().getPlayerData(phase.mTurn).mRole }
        };
        UIButton& button { getLaunchButton(static_cast<PieceTypeID>(pieceType))->getAspect<UIButton>() };

        if(getModel().canLaunchPiece(currentPlayerPieceIdentity, phase.mTurn)) {
            button.enableButton();
        } else {
            button.disableButton();
        }

        if(getModel().getCurrentPhase().mGamePhase != GamePhase::INITIATIVE) {
            const GamePieceData currentPlayerPiece { getModel().getPieceData(currentPlayerPieceIdentity) };
            switch(currentPlayerPiece.mState) {
                case Piece::State::UNLAUNCHED:
                    button.updateHighlightColor({1.f, 1.f, 0.f, 1.f});
                    break;
                case Piece::State::ON_BOARD:
                    button.updateHighlightColor({0.f, 0.f, 1.f, 1.f});
                    break;
                case Piece::State::FINISHED:
                    button.updateHighlightColor({0.f, 1.f, 0.f, 1.f});
                    break;
            }
        }
    }

    // enable the end turn button only at the end of a turn and
    // only during initiative and play phases
    UIButton& endTurnButton { getEndTurnButton()->getAspect<UIButton>() };
    if(phase.mTurnPhase == TurnPhase::END && phase.mGamePhase != GamePhase::END) {
        endTurnButton.enableButton();
    } else {
        endTurnButton.disableButton();
    }
}

void UrUIView::onScoreUpdated(GameScoreData score) {
    std::cout << "UrUIView: on score updated\n";
    getSimObject().getByPath<UIText&>("/viewport_UI/common_pile/@UIText").updateText("common: " + std::to_string(static_cast<int>(score.mCommonPoolCounters)));
    getSimObject().getByPath<UIText&>("/viewport_UI/one_pile/@UIText").updateText("one: " + std::to_string(static_cast<int>(score.mPlayerOneCounters)));
    getSimObject().getByPath<UIText&>("/viewport_UI/two_pile/@UIText").updateText("two: " + std::to_string(static_cast<int>(score.mPlayerTwoCounters)));
}

void UrUIView::onPlayerUpdated(PlayerData player) {
    std::cout << "UrUIView: on player updated\n";
}

void UrUIView::onDiceUpdated(DiceData dice) {
    std::cout << "UrUIView: on dice updated\n";
    std::string displayString;
    switch (dice.mState) {
        case Dice::State::UNROLLED:
            displayString = "dice unrolled";
            break;
        case Dice::State::PRIMARY_ROLLED:
            displayString = "primary roll " + std::to_string(static_cast<int>(dice.mResultScore));
            break;
        case Dice::State::SECONDARY_ROLLED:
            displayString = "final roll " + std::to_string(static_cast<int>(dice.mResultScore));
            break;
    }

    UIButton& buttonAspect { getSimObject().getByPath<UIButton&>("/viewport_UI/dice_roll/@UIButton") };
    if(
        getModel().getCurrentPhase().mGamePhase == GamePhase::END 
        || getModel().getCurrentPhase().mTurnPhase == TurnPhase::END
        || dice.mState == Dice::State::SECONDARY_ROLLED
    ) {
        buttonAspect.disableButton();
    } else {
        buttonAspect.enableButton();
    }
    buttonAspect.updateText(displayString);
}

void UrUIView::onMoveMade(MoveResultData moveData) {
    std::cout << "UrUIView: on move made\n";
}

bool UrUIView::onCancel(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
    std::cout << "UrUIView: cancel attempted\n";
    mSigLaunchPieceCanceled.emit();
    return true;
}

std::shared_ptr<ToyMakersEngine::SimObject> UrUIView::getLaunchButton(PieceTypeID pieceType) {
    std::string pieceString { 
        std::find_if(kButtonEnumMap.begin(), kButtonEnumMap.end(),
            [pieceType](std::pair<std::string, UrUIView::Buttons> item){
                return static_cast<UrUIView::Buttons>(pieceType) == item.second;
            }
        )->first
    };
    return getSimObject().getByPath<std::shared_ptr<ToyMakersEngine::SimObject>>("/viewport_UI/player_panel_a/launch_" + pieceString + "/");
}

std::shared_ptr<ToyMakersEngine::SimObject> UrUIView::getEndTurnButton() {
    return getSimObject().getByPath<std::shared_ptr<ToyMakersEngine::SimObject>>("/viewport_UI/next_turn/");
}
