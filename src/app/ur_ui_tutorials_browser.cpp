#include <fstream>
#include <string>

#include "ui_button.hpp"
#include "ui_text.hpp"
#include "ui_image.hpp"
#include "ur_scene_manager.hpp"

#include "ur_ui_tutorials_browser.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrUITutorialsBrowser::create(const nlohmann::json& jsonAspectProperties) {
    (void)jsonAspectProperties; // prevent unused parameter warnings

    std::shared_ptr<UrUITutorialsBrowser> tutorialsBrowser { std::make_shared<UrUITutorialsBrowser>() };
    tutorialsBrowser->mTutorialsFilepath = jsonAspectProperties.at("tutorials_filepath").get<std::string>();
    tutorialsBrowser->mTutorialTextAspect = jsonAspectProperties.at("text_node_aspect").get<std::string>();
    tutorialsBrowser->mTutorialImageAspect = jsonAspectProperties.at("image_node_aspect").get<std::string>();

    return tutorialsBrowser;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrUITutorialsBrowser::clone() const {
    std::shared_ptr<UrUITutorialsBrowser> tutorialsBrowser { std::make_shared<UrUITutorialsBrowser>() };

    tutorialsBrowser->mTutorialsFilepath = mTutorialsFilepath;
    tutorialsBrowser->mTutorialTextAspect = mTutorialTextAspect;
    tutorialsBrowser->mTutorialImageAspect = mTutorialImageAspect;

    return tutorialsBrowser;
}

void UrUITutorialsBrowser::loadTutorials() {
    mTutorials.clear();

    std::ifstream jsonFileStream;
    jsonFileStream.open(mTutorialsFilepath);
    nlohmann::json tutorialsJSON = nlohmann::json::parse(jsonFileStream);
    jsonFileStream.close();

    for(
        auto tutorial {tutorialsJSON.cbegin()}, end {tutorialsJSON.cend()};
        tutorial != end;
        tutorial++
    ) {
        mTutorials.push_back(*tutorial);
    }

    std::cout << "Ur Tutorials Browser: tutorials loaded successfully!\n";
}

void UrUITutorialsBrowser::onActivated() {
    loadTutorials();
    openPage(0);
}

void UrUITutorialsBrowser::onButtonClicked(const std::string& button) {
    if(button == "next") {
        openPage(mPage + 1);
        return;
    }

    if(button == "previous") {
        openPage(mPage - 1);
        return;
    } 

    assert(false && "There shouldn't be any other buttons present on the page");
}

void UrUITutorialsBrowser::openPage(uint32_t page) {
    assert(hasPage(page) && "No such page exists");
    mPage = page;

    UIText& text {
        getSimObject().getByPath<UIText&>(mTutorialTextAspect)
    };
    UIImage& image {
        getSimObject().getByPath<UIImage&>(mTutorialImageAspect)
    };
    text.updateText(mTutorials[mPage].mText);
    image.updateImage(mTutorials[mPage].mImageFilepath);

    UIButton& next {
        getSimObject().getByPath<UIButton&>(
            "/viewport_UI/next/@UIButton"
        )
    };
    UIButton& prev {
        getSimObject().getByPath<UIButton&>(
            "/viewport_UI/previous/@UIButton"
        )
    };
    if(hasPage(page+1)) {
        next.enableButton();
    } else {
        next.disableButton();
    }
    if(hasPage(page-1)) {
        prev.enableButton();
    } else {
        prev.disableButton();
    }
}

bool UrUITutorialsBrowser::hasPage(uint32_t page) const {
    return page < mTutorials.size();
}

void from_json(const nlohmann::json& json, TutorialContent& tutorialContent) {
    tutorialContent.mText = json.at("text").get<std::string>();
    tutorialContent.mImageFilepath = json.at("image_filepath").get<std::string>();
}

void to_json(nlohmann::json& json, const TutorialContent& tutorialContent) {
    json = {
        { "text", tutorialContent.mText },
        { "image_filepath", tutorialContent.mImageFilepath },
    };
}
