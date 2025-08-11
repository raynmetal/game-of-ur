#include "ur_scene_manager.hpp"
#include "ur_ui_navigation.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrUINavigation::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrUINavigation> mainMenu { std::make_shared<UrUINavigation>() };
    mainMenu->mSceneManagerPath = jsonAspectProperties.at("scene_manager_path").get<std::string>();
    return mainMenu;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrUINavigation::clone() const {
    std::shared_ptr<UrUINavigation> mainMenu { std::make_shared<UrUINavigation>() };
    mainMenu->mSceneManagerPath = mSceneManagerPath;
    return mainMenu;
}

void UrUINavigation::onButtonClicked(const std::string& sceneResourceName) {
    ToyMakersEngine::ECSWorld::getSingletonSystem<ToyMakersEngine::SceneSystem>()
        ->getByPath<UrSceneManager&>(mSceneManagerPath + "@" + UrSceneManager::getSimObjectAspectTypeName())
        .loadScene(sceneResourceName);
}
