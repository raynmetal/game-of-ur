#include "ur_controller.hpp"

#include "game_of_ur_data/model.hpp"
#include "nlohmann/json.hpp"
#include "ur_ui_view.hpp"

const std::map<std::string, UrUIView::Buttons> UrUIView::kButtonEnumMap {
    {"dice_roll", UrUIView::Buttons::DICE},
    {"next_turn", UrUIView::Buttons::NEXT_TURN},
    {"swallow", UrUIView::Buttons::SWALLOW},
    {"storm_bird", UrUIView::Buttons::STORMBIRD},
    {"raven", UrUIView::Buttons::RAVEN},
    {"rooster", UrUIView::Buttons::ROOSTER},
    {"eagle", UrUIView::Buttons::EAGLE},
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
    switch(enumButton) {
        case SWALLOW:
        case STORMBIRD:
        case RAVEN:
        case ROOSTER:
        case EAGLE:
            std::cout << "launch piece attempted: " << button << " clicked\n";
            break;
        case DICE:
            std::cout << "dice roll attempted\n";
            break;
        case NEXT_TURN:
            std::cout << "next turn clicked\n";
            break;
    }
}

void UrUIView::onPhaseUpdated(GamePhaseData phase){
    std::cout << "on phase updated\n";
}
void UrUIView::onScoreUpdated(GameScoreData score) {
    std::cout << "on score updated\n";
}
void UrUIView::onPlayerUpdated(PlayerData player) {
    std::cout << "on player updated\n";
}
void UrUIView::onDiceUpdated(DiceData dice) {
    std::cout << "on dice updated\n";
}
void UrUIView::onMoveMade(MoveResultData moveData) {
    std::cout << "on move made\n";
}

bool UrUIView::onCancel(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) const {
    std::cout << "cancel attempted\n";
    return true;
}

