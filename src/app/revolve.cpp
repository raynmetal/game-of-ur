#include <iostream>

#include <glm/glm.hpp>

#include "../engine/scene_system.hpp"
#include "revolve.hpp"

void Revolve::variableUpdate(uint32_t variableStepMillis) {
    Placement placement { getComponent<Placement>() };
    placement.mOrientation = glm::rotate(placement.mOrientation, variableStepMillis/(1000.f*10.f), {0.f, 1.f, 0.f});
    updateComponent<Placement>(placement);
}

std::shared_ptr<BaseSimObjectAspect> Revolve::clone() const {
    return std::shared_ptr<Revolve>(new Revolve{});
}

std::shared_ptr<BaseSimObjectAspect> Revolve::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<Revolve>{new Revolve{}};
}
