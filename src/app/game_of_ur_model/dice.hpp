#ifndef ZOAPPDICE_H
#define ZOAPPDICE_H

#include <cstdint>
#include <random>

#include "game_phase.hpp"

class Dice {
public:
    enum State {
        UNROLLED,
        PRIMARY_ROLLED,
        SECONDARY_ROLLED,
    };

    void reset();
    void roll();

    inline bool canRoll() const { return mState != SECONDARY_ROLLED; }
    inline State getState() const { return mState; }
    inline uint8_t getPrimaryRoll() const { return mPrimaryRoll; }
    inline bool getSecondaryRoll() const { return mSecondaryRoll; }
    uint8_t getResult(GamePhase currentPhase) const;

private:
    uint8_t upgradedRoll() const;

    uint8_t mPrimaryRoll { 1 };
    bool mSecondaryRoll { false };
    State mState { UNROLLED };

    std::random_device mRandomDevice {};
    std::default_random_engine mRandomEngine { mRandomDevice() };
    std::uniform_int_distribution<int> mPrimaryDieDistribution { 1, 4 };
    std::uniform_int_distribution<int> mYesNoDieDistribution { 0, 1 };
};


#endif
