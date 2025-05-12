#include <iostream>

#include "pointer_callback.hpp"

bool PointerCallback::onClick() {
    std::cout << "\t\"I was clicked!\"\n";
    return true;
}

std::shared_ptr<BaseSimObjectAspect> PointerCallback::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<PointerCallback>{ new PointerCallback{} };
}

std::shared_ptr<BaseSimObjectAspect> PointerCallback::clone() const {
    return std::shared_ptr<PointerCallback>{ new PointerCallback{} };
}
