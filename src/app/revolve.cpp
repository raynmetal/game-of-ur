#include <iostream>

#include <glm/glm.hpp>

#include "../engine/scene_system.hpp"
#include "revolve.hpp"

void Revolve::onSecondPassed() {
    std::cout << "Revolve: passage of a second observed\n";
}

void Revolve::update(uint32_t deltaSimtimeMillis) {
    Placement placement { getComponent<Placement>() };
    placement.mOrientation = glm::rotate(placement.mOrientation, deltaSimtimeMillis/(1000.f*10.f), {0.f, 1.f, 0.f});
    updateComponent<Placement>(placement);
}

std::unique_ptr<BaseSimObjectAspect> Revolve::makeCopy() const {
    return std::unique_ptr<Revolve>(new Revolve{});
}

std::unique_ptr<BaseSimObjectAspect> Revolve::create(const nlohmann::json& jsonAspectProperties) {
    return std::unique_ptr<Revolve>{new Revolve{}};
}
