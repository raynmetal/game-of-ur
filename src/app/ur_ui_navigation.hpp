#ifndef ZOAPPUINAVIGATION_H
#define ZOAPPUINAVIGATION_H

#include <glm/gtx/string_cast.hpp>
#include "../engine/sim_system.hpp"

class UrUINavigation: public ToyMakersEngine::SimObjectAspect<UrUINavigation> {
public:
    UrUINavigation(): ToyMakersEngine::SimObjectAspect<UrUINavigation>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrUINavigation"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void loadScene(const std::string& sceneResourceName);

private:
    std::string mSceneManagerPath {};

    void onButtonClicked(const std::string& button);

public:
    ToyMakersEngine::SignalObserver<const std::string&> mObserveButtonClicked {
        *this, "ButtonClickedObserved",
        [this](const std::string& button) { this->onButtonClicked(button); }
    };
};

#endif
