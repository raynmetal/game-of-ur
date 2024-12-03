#ifndef ZOAPPREVOLVE_H
#define ZOAPPREVOLVE_H

#include "../engine/scene_system.hpp"
#include "../engine/sim_system.hpp"

class Revolve: public SimObjectAspect {
public:
    Revolve (SimObject* simObject): SimObjectAspect { simObject } {}
    void update(uint32_t deltaSimtimeMillis) override;

    std::unique_ptr<SimObjectAspect> makeCopy() const override;
};

#endif
