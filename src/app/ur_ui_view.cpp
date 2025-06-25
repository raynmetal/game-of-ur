#include "ur_controller.hpp"

#include "ur_ui_view.hpp"

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
    std::cout << "Player A can roll dice: " << (getModel().canAdvanceOneTurn(PlayerID::PLAYER_A)? "yes": "no") << "\n";
}

const GameOfUrModel& UrUIView::getModel() const {
    return mGameOfUrController.lock()->getAspect<UrController>().getModel();
}
