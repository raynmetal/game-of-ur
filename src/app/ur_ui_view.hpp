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
    const GameOfUrModel& getModel() const;

private:
    enum Buttons {
        NEXT_TURN,
        DICE,
        SWALLOW,
        STORMBIRD,
        RAVEN,
        ROOSTER,
        EAGLE,
    };
    std::weak_ptr<ToyMakersEngine::SimObject> mGameOfUrController {};
    std::string mControllerPath {};

    static const std::map<std::string, Buttons> kButtonEnumMap;

    void onButtonClicked(const std::string& button);
    void onPhaseUpdated(GamePhaseData phase);
    void onScoreUpdated(GameScoreData score);
    void onPlayerUpdated(PlayerData player);
    void onDiceUpdated(DiceData dice);
    void onMoveMade(MoveResultData moveData);
    bool onCancel(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) const;

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

    ToyMakersEngine::Signal<PlayerID> mSigDiceRollAttempted { *this, "DiceRollAttempted" };
    ToyMakersEngine::Signal<PlayerID> mSigNextTurnAttempted { *this, "NextTurnAttempted" };
    ToyMakersEngine::Signal<PieceTypeID> mSigLaunchPieceInitiated { *this, "LaunchPieceInitiated" };
    ToyMakersEngine::Signal<PieceTypeID> mSigLaunchPieceCanceled { *this, "LaunchPieceCanceled" };
};

#endif
