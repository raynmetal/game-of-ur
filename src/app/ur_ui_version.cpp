#include <string>

#include "ui_text.hpp"
#include "ur_ui_version.hpp"

#include "version.h"


std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrUIVersion::create(const nlohmann::json& jsonAspectProperties) {
    (void)jsonAspectProperties; // prevent unused parameter warnings
    return std::shared_ptr<UrUIVersion>{ new UrUIVersion{} };
}

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrUIVersion::clone() const {
    return std::shared_ptr<UrUIVersion>{ new UrUIVersion{} };
}

void UrUIVersion::onActivated() {
    getAspect<UIText>().updateText(
        "Game Of Ur v" 
        + std::to_string(Game_Of_Ur_VERSION_MAJOR) 
        + "." 
        + std::to_string(Game_Of_Ur_VERSION_MINOR)
        + "."
        + std::to_string(Game_Of_Ur_VERSION_PATCH)
    );
}
