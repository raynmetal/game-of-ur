#include <iostream>

#include <toymaker/engine/sound/types.hpp>
#include <toymaker/engine/sound/system.hpp>

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

void UrUINavigation::onActivated() {
    mSoundChannel = getSimObject().getWorld().lock()->getSystem<ToyMaker::SoundSystem>()->createChannel();
    mSoundButtonClick = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Button_Click_Sound");
    mSoundButtonHover = ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::Sound>("Button_Hover_Sound");
}

void UrUINavigation::onButtonHoveredOver(const std::string& button) {
    std::cout << "Navigation button hovered over!\n";
}
void UrUINavigation::onButtonClicked(const std::string& sceneResourceName) {
    ToyMaker::ECSWorld::getSingletonSystem<ToyMaker::SceneSystem>()
        ->getByPath<UrSceneManager&>(mSceneManagerPath + "@" + UrSceneManager::getSimObjectAspectTypeName())
        .loadScene(sceneResourceName);
}
