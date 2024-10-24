#include <glm/glm.hpp>

#include "../engine/scene_system.hpp"
#include "revolve.hpp"

void Revolve::update(uint32_t deltaSimtimeMillis) {
    Placement placement { getCoreComponent<Placement>() };
    placement.mOrientation = glm::rotate(placement.mOrientation, deltaSimtimeMillis/(1000.f*10.f), {0.f, 1.f, 0.f});
    updateCoreComponent<Placement>(placement);
}
