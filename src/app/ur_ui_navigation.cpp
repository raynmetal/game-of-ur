#include "ur_scene_manager.hpp"
#include "ur_ui_navigation.hpp"

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrUINavigation::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrUINavigation> mainMenu { std::make_shared<UrUINavigation>() };
    mainMenu->mSceneManagerPath = jsonAspectProperties.at("scene_manager_path").get<std::string>();
    return mainMenu;
}

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrUINavigation::clone() const {
    std::shared_ptr<UrUINavigation> mainMenu { std::make_shared<UrUINavigation>() };
    mainMenu->mSceneManagerPath = mSceneManagerPath;
    return mainMenu;
}

void UrUINavigation::onButtonClicked(const std::string& sceneResourceName) {
    ToyMaker::ECSWorld::getSingletonSystem<ToyMaker::SceneSystem>()
        ->getByPath<UrSceneManager&>(mSceneManagerPath + "@" + UrSceneManager::getSimObjectAspectTypeName())
        .loadScene(sceneResourceName);
}
