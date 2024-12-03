#ifndef ZOBACKANDFORTH_H
#define ZOBACKANDFORTH_H

#include "../engine/sim_system.hpp"

class BackAndForth: public SimObjectAspect {
public:
    BackAndForth(SimObject* simObject): SimObjectAspect{simObject} {}
    void update(uint32_t deltaSimtimeMillis) override;

    std::unique_ptr<SimObjectAspect> makeCopy() const override;
private:
    uint32_t mElapsedTime { 0 };
};

#endif
