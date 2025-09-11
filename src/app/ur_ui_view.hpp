#ifndef ZOAPPURUIVIEW_H
#define ZOAPPURUIVIEW_H

#include "toymaker/sim_system.hpp"
#include "toymaker/signals.hpp"
#include "toymaker/text_render.hpp"

#include "game_of_ur_data/model.hpp"

class UrUIView: public ToyMaker::SimObjectAspect<UrUIView> {
public:
    UrUIView(): ToyMaker::SimObjectAspect<UrUIView>{0} {}

    inline static std::string getSimObjectAspectTypeName() { return "UrUIView"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;
    void variableUpdate(uint32_t timeStep) override;
    const GameOfUrModel& getModel() const;

private:
    enum Buttons {
        SWALLOW=PieceTypeID::SWALLOW,
        STORMBIRD=PieceTypeID::STORMBIRD,
        RAVEN=PieceTypeID::RAVEN,
        ROOSTER=PieceTypeID::ROOSTER,
        EAGLE=PieceTypeID::EAGLE,

        NEXT_TURN,
        DICE,
    };

    enum class Mode {
        INTERACT,
        TRANSITION,
    };

    std::weak_ptr<ToyMaker::SimObject> mGameOfUrController {};
    std::string mControllerPath {};
    PlayerID mControlledBy {};
    Mode mMode { Mode::INTERACT };
    std::vector<std::reference_wrapper<UIText>> mUpdatedTextElements {};
    uint32_t mAnimationTimeMillis { 0 };
    uint32_t mBlinkLengthMillis { 2500 };
    uint32_t mBlinkPeriodMillis { 400 };

    static const std::map<std::string, Buttons> kButtonEnumMap;

    void onControllerReady();

    void updateText(const std::string& path, const std::string& text);
    void reactivateControls();

    void onButtonClicked(const std::string& button);
    void onPhaseUpdated(GamePhaseData phase);
    void onScoreUpdated(GameScoreData score);
    void onPlayerUpdated(PlayerData player);
    void onDiceUpdated(DiceData dice);
    void onViewUpdateStarted();
    void onMoveMade(MoveResultData moveData);
    void onControlInterface(PlayerID playerID);
    bool onCancel(const ToyMaker::ActionData& actionData, const ToyMaker::ActionDefinition& actionDefinition);

    std::shared_ptr<ToyMaker::SimObject> getLaunchButton(PieceTypeID pieceTypeID, PlayerID player);
    std::shared_ptr<ToyMaker::SceneNode> getPlayerPanel(PlayerID player);
    std::shared_ptr<ToyMaker::SimObject> getEndTurnButton();

    std::weak_ptr<ToyMaker::FixedActionBinding> handleCancel { declareFixedActionBinding(
        "General", "Cancel", [this](const ToyMaker::ActionData& actionData, const ToyMaker::ActionDefinition& actionDefinition) {
            return this->onCancel(actionData, actionDefinition);
        }
    )};
public:
    ToyMaker::SignalObserver<const std::string&> mObserveButtonClicked {
        *this, "ButtonClickedObserved",
        [this](const std::string& button) { this->onButtonClicked(button); }
    };

    ToyMaker::SignalObserver<GamePhaseData> mObservePhaseUpdated {
        *this, "PhaseUpdatedObserved",
        [this](GamePhaseData phaseData) { this->onPhaseUpdated(phaseData); }
    };
    ToyMaker::SignalObserver<GameScoreData> mObserveScoreUpdated {
        *this, "ScoreUpdatedObserved",
        [this](GameScoreData scoreData) { this->onScoreUpdated(scoreData); }
    };
    ToyMaker::SignalObserver<PlayerData> mObservePlayerUpdated {
        *this, "PlayerUpdatedObserved",
        [this](PlayerData playerData) { this->onPlayerUpdated(playerData); }
    };
    ToyMaker::SignalObserver<DiceData> mObserveDiceUpdated {
        *this, "DiceUpdatedObserved",
        [this](DiceData diceData) { this->onDiceUpdated(diceData); }
    };
    ToyMaker::SignalObserver<MoveResultData> mObserveMoveMade {
        *this, "MoveMadeObserved",
        [this](MoveResultData moveData) { this->onMoveMade(moveData); }
    };

    ToyMaker::SignalObserver<PlayerID> mObserveControlInterface {
        *this, "ControlInterfaceObserved",
        [this](PlayerID playerID) { this->onControlInterface(playerID); }
    };

    ToyMaker::Signal<> mSigDiceRollAttempted { *this, "DiceRollAttempted" };
    ToyMaker::Signal<> mSigNextTurnAttempted { *this, "NextTurnAttempted" };
    ToyMaker::Signal<PieceTypeID> mSigLaunchPieceInitiated { *this, "LaunchPieceInitiated" };
    ToyMaker::Signal<> mSigLaunchPieceCanceled { *this, "LaunchPieceCanceled" };

    ToyMaker::SignalObserver<> mObserveControllerReady { 
        *this, "ControllerReadyObserved",
        [this]() {this->onControllerReady();}
    };
    ToyMaker::Signal<std::string> mSigViewSubscribed {*this, "ViewSubscribed"};

    ToyMaker::SignalObserver<> mObserveViewUpdateStarted { 
        *this, "ViewUpdateStartedObserved",
        [this](){ this->onViewUpdateStarted(); }
    };
    ToyMaker::Signal<std::string> mSigViewUpdateCompleted {
        *this, "ViewUpdateCompleted"
    };
};

#endif
