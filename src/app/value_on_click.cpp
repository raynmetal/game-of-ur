#include <iostream>
#include <random>
#include <sstream>

#include "test_text.hpp"
#include "value_on_click.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> ValueOnClick::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<ValueOnClick> valueOnClick { new ValueOnClick{} };
    valueOnClick->mReturnValue = jsonAspectProperties.at("return_value").get<std::string>();
    return valueOnClick;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> ValueOnClick::clone() const {
    std::shared_ptr<ValueOnClick> valueOnClick { new ValueOnClick{} };
    valueOnClick->mReturnValue = mReturnValue;
    return valueOnClick;
}

bool ValueOnClick::onPointerLeftClick(glm::vec4 clickLocation) {
    mSigClicked.emit(mReturnValue);
    return false;
}
