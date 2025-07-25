#include "ur_scene_manager.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrSceneManager::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrSceneManager> sceneManager { std::make_shared<UrSceneManager>() };
    sceneManager->mNextScene = jsonAspectProperties.at("initial_scene").get<std::string>();
    return sceneManager;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrSceneManager::clone() const {
    std::shared_ptr<UrSceneManager> sceneManager { std::make_shared<UrSceneManager>() };
    sceneManager->mNextScene = mNextScene;
    return sceneManager;
}

void UrSceneManager::onActivated() {
    loadScene(mNextScene);
}

void UrSceneManager::loadScene(const std::string& sceneResourceName) {
    mNextScene = sceneResourceName;
    mSwitchScenesThisFrame = true;

    // NOTE: we won't load the scene right away; let the calls that led up
    // to here finish, and perform the actual switch during our next variable
    // update.  This way, we'll avoid creating invalid references.
}

void UrSceneManager::variableUpdate(uint32_t timeStepMillis) {
    if(!mSwitchScenesThisFrame) {
        mRemovedScene.clear();
        return;
    }

    loadScene_();
    mSwitchScenesThisFrame = false;
}

void UrSceneManager::loadScene_() {
    // cache the children so that the scene system function that called us
    // is still valid
    mRemovedScene = getSimObject().getChildren();
    getSimObject().removeChildren();

    getSimObject().addNode(
        ToyMakersEngine::ResourceDatabase::GetRegisteredResource<ToyMakersEngine::SimObject>(
            mNextScene
        ),
        "/"
    );
    assert(getSimObject().getChildren().size() == 1 && "The scene manager must always have no more than one child");
}
