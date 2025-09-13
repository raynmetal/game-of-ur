/**
 * @file ur_ui_tutorials_browser.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains the class definition for the aspect responsible for loading and displaying tutorial content.
 * @version 0.3.2
 * @date 2025-09-13
 * 
 * 
 */

#ifndef ZOAPPUITUTORIALSBROWSER_H
#define ZOAPPUITUTORIALSBROWSER_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "toymaker/sim_system.hpp"

#include "ur_records.hpp"


struct TutorialContent {
    std::string mHeading {};
    std::string mText {};
    std::string mImageFilepath {};
};

class UrUITutorialsBrowser: public ToyMaker::SimObjectAspect<UrUITutorialsBrowser> {
public:

    UrUITutorialsBrowser(): ToyMaker::SimObjectAspect<UrUITutorialsBrowser>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "UrUITutorialsBrowser"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void loadScene(const std::string& sceneResourceName);

private:
    uint32_t mPage {0};
    std::string mTutorialsFilepath {};
    std::string mTutorialTextAspect {};
    std::string mTutorialHeadingAspect {};
    std::string mTutorialImageAspect {};
    std::vector<TutorialContent> mTutorials {};

    void onActivated() override;

    void onButtonClicked(const std::string& button);
    
    bool hasPage(uint32_t page) const;
    void openPage(uint32_t page);

    void loadTutorials();

public:
    ToyMaker::SignalObserver<const std::string&> mObserveButtonClicked {
        *this, "ButtonClickedObserved",
        [this](const std::string& button) { this->onButtonClicked(button); }
    };
};

void from_json(const nlohmann::json& json, TutorialContent& tutorialContent);
void to_json(nlohmann::json& json, const TutorialContent& tutorialContent);

#endif
