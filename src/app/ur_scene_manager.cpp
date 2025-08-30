#include "ur_scene_manager.hpp"

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrSceneManager::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrSceneManager> sceneManager { std::make_shared<UrSceneManager>() };
    sceneManager->mNextScene = jsonAspectProperties.at("initial_scene").get<std::string>();
    sceneManager->mAutoloads = jsonAspectProperties.at("autoloads").get<std::vector<std::string>>();
    return sceneManager;
}

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrSceneManager::clone() const {
    std::shared_ptr<UrSceneManager> sceneManager { std::make_shared<UrSceneManager>() };
    sceneManager->mNextScene = mNextScene;
    sceneManager->mAutoloads = mAutoloads;
    return sceneManager;
}

void UrSceneManager::onActivated() {
    loadAutoloads();
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
    (void)timeStepMillis; // prevent unused parameter warnings
    if(!mAutoloadsActivated) { activateAutoloads(); return; }
    if(!mSwitchScenesThisFrame) { return; }

    loadScene_();
    mRemovedScene.clear();
    mSwitchScenesThisFrame = false;
}

void UrSceneManager::loadScene_() {
    // cache the children so that the scene system function that called us
    // is still valid
    mRemovedScene = getSimObject().getChildren();
    getSimObject().removeChildren();

    // as far as non-singleton scenes are concerned, this node is itself the root of the scene tree
    getSimObject().addNode(
        ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::SimObject>(
            mNextScene
        ),
        "/"
    );
    assert(getSimObject().getChildren().size() == 1 && "The scene manager must always have no more than one child");
}

void UrSceneManager::loadAutoloads() {
    /**
     * TODO: nodes attached directly to the *real* root node during a scene node's
     * onActivated don't get activated automatically.  It would be nice if they did,
     * or if autoloads became a kind of first class concept in the engine
     */
    for(auto& autoload: mAutoloads) {
        // add autoloaded components to the (actual) root of the scene tree
        std::shared_ptr<ToyMaker::SimObject> autoloadedNode {
            ToyMaker
                ::ResourceDatabase
                ::GetRegisteredResource<ToyMaker::SimObject>(autoload)
        };
        getSimObject()
            .getWorld()
            .lock()
            ->getSingletonSystem<ToyMaker::SceneSystem>()
            ->addNode(
                autoloadedNode,"/"
        );
    }
}

void UrSceneManager::activateAutoloads() {
    /**
     * NOTE: We activate the autoloaded nodes here, since they don't automatically
     * get activated when they're attached to the real root
     */
    for(auto& autoload: mAutoloads) {
        std::shared_ptr<ToyMaker::SimObject> autoloadedNode {
            ToyMaker
                ::ResourceDatabase
                ::GetRegisteredResource<ToyMaker::SimObject>(autoload)
        };
        autoloadedNode->setEnabled<ToyMaker::SceneSystem>(true);
    }
    mAutoloadsActivated = true;
}
