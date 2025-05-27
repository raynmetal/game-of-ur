#include <iostream>

#include <glm/glm.hpp>

#include "../engine/scene_system.hpp"
#include "revolve.hpp"

void Revolve::variableUpdate(uint32_t variableStepMillis) {
    ToyMakersEngine::Placement placement { getComponent<ToyMakersEngine::Placement>() };
    placement.mOrientation = glm::rotate(placement.mOrientation, variableStepMillis/(1000.f*10.f), {0.f, 1.f, 0.f});
    updateComponent<ToyMakersEngine::Placement>(placement);
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> Revolve::clone() const {
    return std::shared_ptr<Revolve>(new Revolve{});
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> Revolve::create(const nlohmann::json& jsonAspectProperties) {
    return std::shared_ptr<Revolve>{new Revolve{}};
}
