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

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrUIView::clone() const {
    std::shared_ptr<UrUIView> uiView{ std::make_shared<UrUIView>() };
    uiView->mControllerPath = mControllerPath;
    return uiView;
}

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrUIView::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrUIView> uiView { std::make_shared<UrUIView>() };
    uiView->mControllerPath = jsonAspectProperties.at("controller_path").get<std::string>();
    return uiView;
}

void UrUIView::onActivated() {
    mGameOfUrController = (
        ToyMaker::ECSWorld::getSingletonSystem<ToyMaker::SceneSystem>()
        ->getByPath<std::shared_ptr<ToyMaker::SimObject>>(mControllerPath)
    );
}

const GameOfUrModel& UrUIView::getModel() const {
    return mGameOfUrController.lock()->getAspect<UrController>().getModel();
}

void UrUIView::onButtonClicked(const std::string& button) {
    if(mMode == Mode::TRANSITION) return;
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
    const std::string playerText { ((phase.mTurn == PlayerID::PLAYER_A)? "Player A's turn": "Player B's turn") };
    std::stringstream phaseText {};
    switch(phase.mGamePhase) {
        case GamePhase::INITIATIVE:
            phaseText << "Initiative";
            break;
        case GamePhase::PLAY:
            phaseText << "Play";
            break;
        case GamePhase::END:
            phaseText << "End";
            break;
    }
    phaseText << ": ";
    switch(phase.mTurnPhase) {
        case TurnPhase::ROLL_DICE:
            phaseText << "Roll the dice.";
            break;
        case TurnPhase::MOVE_PIECE:
            phaseText << "Move a piece.";
            break;
        case TurnPhase::END:
            phaseText << "End turn.";
            break;
    }
    updateText(
        "/viewport_UI/current_turn/@UIText",
        playerText
    );
    updateText(
        "/viewport_UI/phase/@UIText",
        phaseText.str()
    );

    // disable all piece launch buttons while changing their highlight color
    // to represent their current state (unlaunched, on board, finished)
    for(uint8_t playerIndex { PlayerID::PLAYER_A }; playerIndex <= PlayerID::PLAYER_B; ++playerIndex) {
        const PlayerID player { playerIndex };
        for(uint8_t pieceType{ PieceTypeID::SWALLOW }; pieceType < PieceTypeID::TOTAL; ++pieceType) {
            const PieceIdentity currentPlayerPieceIdentity {
                .mType { static_cast<PieceTypeID>(pieceType) },
                .mOwner { getModel().getPlayerData(player).mRole }
            };
            UIButton& button { getLaunchButton(static_cast<PieceTypeID>(pieceType), player)->getAspect<UIButton>() };
            button.disableButton();
            if(phase.mGamePhase != GamePhase::INITIATIVE) {
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
    }

    UIButton& endTurnButton { getEndTurnButton()->getAspect<UIButton>() };
    endTurnButton.disableButton();
}

void UrUIView::onControlInterface(PlayerID player) {
    std::cout << "UrUIView: on UI control requested\n";
    mControlledBy = player;
    reactivateControls();
}

void UrUIView::onScoreUpdated(GameScoreData score) {
    std::cout << "UrUIView: on score updated\n";
    updateText(
        "/viewport_UI/common_pile/@UIText",
        "Winning pile: " + std::to_string(static_cast<int>(score.mCommonPoolCounters))
    );
}

void UrUIView::onPlayerUpdated(PlayerData player) {
    std::cout << "UrUIView: on player updated\n";
    std::shared_ptr<ToyMaker::SceneNode> playerPanel { getPlayerPanel(player.mPlayer) };

    std::string playerText { (player.mPlayer == PlayerID::PLAYER_A)? "Player A": "Player B"};
    switch(player.mRole) {
        case RoleID::BLACK:
            playerText += " (Black)";
            break;
        case RoleID::WHITE:
            playerText += " (White)";
            break;

        case RoleID::NA:
            break;
    }
    const std::string countersText { "counters: " + std::to_string(static_cast<int>(player.mCounters)) };

    updateText(
        playerPanel->getPathFromAncestor(getSimObject().shared_from_this()) + "player/@UIText",
        playerText
    );
    updateText(
        playerPanel->getPathFromAncestor(getSimObject().shared_from_this()) + "counters/@UIText",
        countersText
    );
}

void UrUIView::onDiceUpdated(DiceData dice) {
    std::cout << "UrUIView: on dice updated\n";

    updateText(
        "/viewport_UI/primary_roll/@UIText",
        "Primary: " + ((dice.mState!=Dice::State::UNROLLED)? std::to_string(static_cast<int>(dice.mPrimaryRoll)): "NA")
    );
    updateText(
        "/viewport_UI/secondary_roll/@UIText",
        std::string{"Double/Quit: "} + (
            (dice.mState != Dice::State::SECONDARY_ROLLED)? 
                "NA":
                (dice.mSecondaryRoll? "D": "Q")
        )
    );
    updateText(
        "/viewport_UI/final_roll/@UIText",
        std::string("Final: ") 
        + ((dice.mState != Dice::State::UNROLLED)? std::to_string(static_cast<int>(dice.mResultScore)): "NA")
    );
    updateText(
        "/viewport_UI/previous_roll/@UIText",
        std::string("Previous: ") + std::to_string(static_cast<int>(dice.mPreviousResult))
    );

    getSimObject().getByPath<UIButton&>("/viewport_UI/dice_roll/@UIButton").disableButton();
}

void UrUIView::onMoveMade(MoveResultData moveData) {
    (void)moveData; // prevent unused parameter warnings
    std::cout << "UrUIView: on move made\n";
    getSimObject().getByPath<UIButton&>("/viewport_UI/dice_roll/@UIButton").disableButton();
}

bool UrUIView::onCancel(const ToyMaker::ActionData& actionData, const ToyMaker::ActionDefinition& actionDefinition) {
    (void)actionData; // prevent unused parameter warnings
    (void)actionDefinition; // prevent unused parameter warnings
    std::cout << "UrUIView: cancel attempted\n";
    if(mMode == Mode::TRANSITION) { return false; }

    mSigLaunchPieceCanceled.emit();
    return true;
}

std::shared_ptr<ToyMaker::SimObject> UrUIView::getLaunchButton(PieceTypeID pieceType, PlayerID player) {
    const std::string pieceString {
        "launch_" + std::find_if(kButtonEnumMap.begin(), kButtonEnumMap.end(),
            [pieceType](std::pair<std::string, UrUIView::Buttons> item){
                return static_cast<UrUIView::Buttons>(pieceType) == item.second;
            }
        )->first
    };
    return getPlayerPanel(player)->getByPath<std::shared_ptr<ToyMaker::SimObject>>(
            "/buttons/" + pieceString + "/"
    );
}

std::shared_ptr<ToyMaker::SceneNode> UrUIView::getPlayerPanel(PlayerID player) {
    const std::string playerPanelString {
        "player_panel_" + static_cast<std::string>((player == PlayerID::PLAYER_A)? "a": "b")
    };
    return getSimObject().getByPath<std::shared_ptr<ToyMaker::SceneNode>>(
        "/viewport_UI/" + playerPanelString + "/"
    );
}

std::shared_ptr<ToyMaker::SimObject> UrUIView::getEndTurnButton() {
    return getSimObject().getByPath<std::shared_ptr<ToyMaker::SimObject>>("/viewport_UI/next_turn/");
}

void UrUIView::onControllerReady() {
    const std::string viewPath { getSimObject().getPathFromAncestor(mGameOfUrController.lock()) };

    mSigViewSubscribed.emit(viewPath);
}

void UrUIView::onViewUpdateStarted() {
    mMode = Mode::TRANSITION;
    mAnimationTimeMillis = 0;
}

void UrUIView::variableUpdate(uint32_t timeStep) {
    if(mMode != Mode::TRANSITION) return;

    mAnimationTimeMillis += timeStep;

    // It's time for the animation to come to an end
    if(mAnimationTimeMillis >= mBlinkLengthMillis || mUpdatedTextElements.empty()) {
        for(UIText& text: mUpdatedTextElements) {
            text.updateColor(glm::u8vec4{255, 255, 255, 255});
        }
        mUpdatedTextElements.clear();
        mAnimationTimeMillis=0;
        mMode = Mode::INTERACT;

        const std::string viewPath { getSimObject().getPathFromAncestor(mGameOfUrController.lock()) };
        mSigViewUpdateCompleted.emit(viewPath);
        return;
    }

    // switch between black and white once every blink period
    const uint32_t nthBlink { mAnimationTimeMillis / mBlinkPeriodMillis };
    const glm::u8vec4 currentColor {
        (nthBlink%2)?
        glm::u8vec4{255, 255, 255, 255}:
        glm::u8vec4{0, 0, 0, 255}
    };
    for(UIText& text: mUpdatedTextElements) {
        text.updateColor(currentColor);
    }
}

void UrUIView::updateText(const std::string& path, const std::string& text) {
    UIText& textNode { getSimObject().getByPath<UIText&>(path) };
    if(text == textNode.getText()) { return; }

    textNode.updateText(text);
    mUpdatedTextElements.push_back(std::reference_wrapper{textNode});
}

void UrUIView::reactivateControls() {
    const GamePhaseData phase{ getModel().getCurrentPhase() };
    const DiceData dice { getModel().getDiceData() };

    // enable piece launch buttons only for pieces that are available
    for(uint8_t pieceType{ PieceTypeID::SWALLOW }; pieceType < PieceTypeID::TOTAL; ++pieceType) {
        const PieceIdentity currentPlayerPieceIdentity {
            .mType { static_cast<PieceTypeID>(pieceType) },
            .mOwner { getModel().getPlayerData(phase.mTurn).mRole }
        };

        if(getModel().canLaunchPiece(currentPlayerPieceIdentity, mControlledBy)) {
            UIButton& button {
                getLaunchButton(static_cast<PieceTypeID>(pieceType),
                mControlledBy)->getAspect<UIButton>()
            };
            button.enableButton();
        }
    }

    // enable the end turn button only at the end of a turn and
    // only during initiative and play phases
    UIButton& endTurnButton { getEndTurnButton()->getAspect<UIButton>() };
    if(
        phase.mTurnPhase == TurnPhase::END 
        && phase.mGamePhase != GamePhase::END
        && phase.mTurn == mControlledBy
    ) {
        endTurnButton.enableButton();
    }

    // enable the dice button if a die can be rolled
    UIButton& diceButton { getSimObject().getByPath<UIButton&>("/viewport_UI/dice_roll/@UIButton") };
    if(
        phase.mGamePhase != GamePhase::END 
        && phase.mTurnPhase != TurnPhase::END
        && phase.mTurn == mControlledBy
        && dice.mState != Dice::State::SECONDARY_ROLLED
    ) {
        diceButton.enableButton();
    }
}
