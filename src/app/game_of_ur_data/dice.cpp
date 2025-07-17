#include <cassert>

#include "dice.hpp"

void Dice::reset() { mState = State::UNROLLED; }

uint8_t Dice::getResult(GamePhase currentPhase) const {
    switch(currentPhase) {
        case GamePhase::INITIATIVE:
            if(mState == State::SECONDARY_ROLLED) {
                return mSecondaryRoll? upgradedRoll(): mPrimaryRoll;
            }
            return mPrimaryRoll;

        case GamePhase::PLAY:
            if(mState == State::UNROLLED) return 0;
            if(mState == State::SECONDARY_ROLLED) {
                return mSecondaryRoll? upgradedRoll(): 0;
            }
            return mPrimaryRoll;

        case GamePhase::END:
            return 0;
    }

    return 0;
}

void Dice::roll() {
    switch(mState) {
        case State::UNROLLED:
            mPrimaryRoll = mPrimaryDieDistribution(mRandomEngine);
            mState = State::PRIMARY_ROLLED;
            break;
        case State::PRIMARY_ROLLED:
            mSecondaryRoll = mYesNoDieDistribution(mRandomEngine);
            mState = State::SECONDARY_ROLLED;
            break;
        case State::SECONDARY_ROLLED:
            assert(false && "No roll is possible after secondary");
    }
}

uint8_t Dice::upgradedRoll() const {
    if(mPrimaryRoll == 4) return 10;
    return mPrimaryRoll + 4;
}
