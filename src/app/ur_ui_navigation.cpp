#include <iostream>

#include "ur_sounds.hpp"

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

void UrUINavigation::onActivated() { }

void UrUINavigation::onButtonHoveredOver(const std::string& button) {
    std::cout << "Navigation button hovered over!\n";
    auto& soundPlayer {
        getSimObject().getWorld().lock()
            ->getSingletonSystem<ToyMaker::SceneSystem>()
            ->getByPath<UrSoundPlayer&>("/ur_sounds/@UrSoundPlayer")
    };
    soundPlayer.playEffect(UrSoundFX::BUTTON_HOVER);
}

void UrUINavigation::onButtonClicked(const std::string& sceneResourceName) {
    auto& soundPlayer {
        getSimObject().getWorld().lock()
            ->getSingletonSystem<ToyMaker::SceneSystem>()
            ->getByPath<UrSoundPlayer&>("/ur_sounds/@UrSoundPlayer")
    };
    soundPlayer.playEffect(UrSoundFX::BUTTON_CLICK, 1);

    ToyMaker::ECSWorld::getSingletonSystem<ToyMaker::SceneSystem>()
        ->getByPath<UrSceneManager&>(mSceneManagerPath + "@" + UrSceneManager::getSimObjectAspectTypeName())
        .loadScene(sceneResourceName);
}

