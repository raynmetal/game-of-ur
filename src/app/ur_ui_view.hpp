#ifndef ZOAPPURUIVIEW_H
#define ZOAPPURUIVIEW_H

#include "../engine/sim_system.hpp"
#include "../engine/signals.hpp"
#include "../engine/text_render.hpp"

#include "game_of_ur_data/model.hpp"

class UrUIView: public ToyMakersEngine::SimObjectAspect<UrUIView> {
public:
    UrUIView(): ToyMakersEngine::SimObjectAspect<UrUIView>{0} {}

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

    std::weak_ptr<ToyMakersEngine::SimObject> mGameOfUrController {};
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
    bool onCancel(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition);

    std::shared_ptr<ToyMakersEngine::SimObject> getLaunchButton(PieceTypeID pieceTypeID, PlayerID player);
    std::shared_ptr<ToyMakersEngine::SceneNode> getPlayerPanel(PlayerID player);
    std::shared_ptr<ToyMakersEngine::SimObject> getEndTurnButton();

    std::weak_ptr<ToyMakersEngine::FixedActionBinding> handleCancel { declareFixedActionBinding(
        "General", "Cancel", [this](const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
            return this->onCancel(actionData, actionDefinition);
        }
    )};
public:
    ToyMakersEngine::SignalObserver<const std::string&> mObserveButtonClicked {
        *this, "ButtonClickedObserved",
        [this](const std::string& button) { this->onButtonClicked(button); }
    };

    ToyMakersEngine::SignalObserver<GamePhaseData> mObservePhaseUpdated {
        *this, "PhaseUpdatedObserved",
        [this](GamePhaseData phaseData) { this->onPhaseUpdated(phaseData); }
    };
    ToyMakersEngine::SignalObserver<GameScoreData> mObserveScoreUpdated {
        *this, "ScoreUpdatedObserved",
        [this](GameScoreData scoreData) { this->onScoreUpdated(scoreData); }
    };
    ToyMakersEngine::SignalObserver<PlayerData> mObservePlayerUpdated {
        *this, "PlayerUpdatedObserved",
        [this](PlayerData playerData) { this->onPlayerUpdated(playerData); }
    };
    ToyMakersEngine::SignalObserver<DiceData> mObserveDiceUpdated {
        *this, "DiceUpdatedObserved",
        [this](DiceData diceData) { this->onDiceUpdated(diceData); }
    };
    ToyMakersEngine::SignalObserver<MoveResultData> mObserveMoveMade {
        *this, "MoveMadeObserved",
        [this](MoveResultData moveData) { this->onMoveMade(moveData); }
    };

    ToyMakersEngine::SignalObserver<PlayerID> mObserveControlInterface {
        *this, "ControlInterfaceObserved",
        [this](PlayerID playerID) { this->onControlInterface(playerID); }
    };

    ToyMakersEngine::Signal<> mSigDiceRollAttempted { *this, "DiceRollAttempted" };
    ToyMakersEngine::Signal<> mSigNextTurnAttempted { *this, "NextTurnAttempted" };
    ToyMakersEngine::Signal<PieceTypeID> mSigLaunchPieceInitiated { *this, "LaunchPieceInitiated" };
    ToyMakersEngine::Signal<> mSigLaunchPieceCanceled { *this, "LaunchPieceCanceled" };

    ToyMakersEngine::SignalObserver<> mObserveControllerReady { 
        *this, "ControllerReadyObserved",
        [this]() {this->onControllerReady();}
    };
    ToyMakersEngine::Signal<std::string> mSigViewSubscribed {*this, "ViewSubscribed"};

    ToyMakersEngine::SignalObserver<> mObserveViewUpdateStarted { 
        *this, "ViewUpdateStartedObserved",
        [this](){ this->onViewUpdateStarted(); }
    };
    ToyMakersEngine::Signal<std::string> mSigViewUpdateCompleted {
        *this, "ViewUpdateCompleted"
    };
};

#endif
