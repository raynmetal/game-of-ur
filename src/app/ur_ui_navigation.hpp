/**
 * @file ur_ui_navigation.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains the class definition for the aspect which informs the UrSceneManager that a scene change is required upon receiving a corresponding UI event.
 * @version 0.3.2
 * @date 2025-09-13
 * 
 * 
 */

#ifndef ZOAPPUINAVIGATION_H
#define ZOAPPUINAVIGATION_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "toymaker/sim_system.hpp"

class UrUINavigation: public ToyMaker::SimObjectAspect<UrUINavigation> {
public:
    UrUINavigation(): ToyMaker::SimObjectAspect<UrUINavigation>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrUINavigation"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void loadScene(const std::string& sceneResourceName);

private:
    std::string mSceneManagerPath {};

    void onButtonClicked(const std::string& button);

public:
    ToyMaker::SignalObserver<const std::string&> mObserveButtonClicked {
        *this, "ButtonClickedObserved",
        [this](const std::string& button) { this->onButtonClicked(button); }
    };
};

#endif
