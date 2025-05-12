#include <iostream>

#include "click_callback_impl_1.hpp"

bool ClickCallbackImpl1::onPointerClick() {
    std::cout << "\t\"I was clicked!\"\n";
    return true;
}

std::shared_ptr<BaseSimObjectAspect> ClickCallbackImpl1::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<ClickCallbackImpl1>{ new ClickCallbackImpl1{} };
}

std::shared_ptr<BaseSimObjectAspect> ClickCallbackImpl1::clone() const {
    return std::shared_ptr<ClickCallbackImpl1>{ new ClickCallbackImpl1{} };
}
