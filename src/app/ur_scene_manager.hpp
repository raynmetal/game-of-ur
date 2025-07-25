#ifndef ZOAPPSCENEMANAGER_H
#define ZOAPPSCENEMANAGER_H

#include <glm/gtx/string_cast.hpp>
#include "../engine/sim_system.hpp"

class UrSceneManager: public ToyMakersEngine::SimObjectAspect<UrSceneManager> {
public:
    UrSceneManager(): ToyMakersEngine::SimObjectAspect<UrSceneManager>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrSceneManager"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void loadScene(const std::string& sceneResourceName);

private:
    void onActivated() override;
    void variableUpdate(uint32_t timeStepMillis) override;

    void loadScene_();

    std::string mNextScene {};
    bool mSwitchScenesThisFrame { false };

    std::vector<std::shared_ptr<ToyMakersEngine::SceneNodeCore>> mRemovedScene {};
};

#endif
