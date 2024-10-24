#ifndef ZOAPPREVOLVE_H
#define ZOAPPREVOLVE_H

#include "../engine/scene_system.hpp"
#include "../engine/sim_system.hpp"

class Revolve: public SimComponent {
public:
    Revolve (SimObject* simObject): SimComponent { simObject } {}
    void update(uint32_t deltaSimtimeMillis) override;
};

#endif
