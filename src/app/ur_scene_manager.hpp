#ifndef ZOAPPSCENEMANAGER_H
#define ZOAPPSCENEMANAGER_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "toymaker/sim_system.hpp"

class UrSceneManager: public ToyMaker::SimObjectAspect<UrSceneManager> {
public:
    UrSceneManager(): ToyMaker::SimObjectAspect<UrSceneManager>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrSceneManager"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void loadScene(const std::string& sceneResourceName);

private:
    void onActivated() override;
    void variableUpdate(uint32_t timeStepMillis) override;

    void loadScene_();
    void loadAutoloads();
    void activateAutoloads();

    std::string mNextScene {};
    std::vector<std::string> mAutoloads {};
    bool mSwitchScenesThisFrame { false };
    bool mAutoloadsActivated { false };

    std::vector<std::shared_ptr<ToyMaker::SceneNodeCore>> mRemovedScene {};
};

#endif
