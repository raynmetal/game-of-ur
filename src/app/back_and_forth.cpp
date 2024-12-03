#include "back_and_forth.hpp"
#include "../engine/scene_system.hpp"

#include "glm/glm.hpp"

void BackAndForth::update(uint32_t deltaSimtimeMillis) {
    mElapsedTime += deltaSimtimeMillis;
    Placement placement  { getComponent<Placement>() };
    placement.mPosition.z = glm::sin(glm::radians(mElapsedTime/10.f + getEntityID()*45.f));
    updateComponent<Placement>(placement);
}

std::unique_ptr<SimObjectAspect> BackAndForth::makeCopy() const {
    return std::make_unique<BackAndForth>(nullptr);
}

