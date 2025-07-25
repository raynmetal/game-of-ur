#include "ur_scene_manager.hpp"
#include "ur_main_menu.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrMainMenu::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrMainMenu> mainMenu { std::make_shared<UrMainMenu>() };
    mainMenu->mSceneManagerPath = jsonAspectProperties.at("scene_manager_path").get<std::string>();
    return mainMenu;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrMainMenu::clone() const {
    std::shared_ptr<UrMainMenu> mainMenu { std::make_shared<UrMainMenu>() };
    mainMenu->mSceneManagerPath = mSceneManagerPath;
    return mainMenu;
}

void UrMainMenu::onButtonClicked(const std::string& sceneResourceName) {
    ToyMakersEngine::ECSWorld::getSingletonSystem<ToyMakersEngine::SceneSystem>()
        ->getByPath<UrSceneManager&>(mSceneManagerPath + "@" + UrSceneManager::getSimObjectAspectTypeName())
        .loadScene(sceneResourceName);
}
