#ifndef ZOBACKANDFORTH_H
#define ZOBACKANDFORTH_H

#include "../engine/sim_system.hpp"

class BackAndForth: public SimComponent {
public:
    BackAndForth(SimObject* simObject): SimComponent{simObject} {}
    void update(uint32_t deltaSimtimeMillis) override;
private:
    uint32_t mElapsedTime { 0 };
};

#endif
