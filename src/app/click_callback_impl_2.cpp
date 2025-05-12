#include <iostream>

#include "click_callback_impl_2.hpp"

bool ClickCallbackImpl2::onPointerClick() {
    std::cout << "\t\"I believe I have been clicked by something!\"\n";
    return true;
}

std::shared_ptr<BaseSimObjectAspect> ClickCallbackImpl2::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<ClickCallbackImpl2>{ new ClickCallbackImpl2{} };
}

std::shared_ptr<BaseSimObjectAspect> ClickCallbackImpl2::clone() const {
    return std::shared_ptr<ClickCallbackImpl2>{ new ClickCallbackImpl2{} };
}
