#include "back_and_forth.hpp"
#include "../engine/scene_system.hpp"

#include "glm/glm.hpp"

void BackAndForth::update(uint32_t deltaSimtimeMillis) {
    mElapsedTime += deltaSimtimeMillis;
    Placement placement  { getCoreComponent<Placement>() };
    placement.mPosition.z = glm::sin(glm::radians(mElapsedTime/10.f + getEntityID()*45.f));
    updateCoreComponent<Placement>(placement);
}
