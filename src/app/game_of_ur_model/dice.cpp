#include <cassert>

#include "dice.hpp"

void Dice::reset() { mState = State::UNROLLED; }

uint8_t Dice::getResult(GamePhase currentPhase) const {
    switch(currentPhase) {
        case GamePhase::INITIATIVE:
            assert(mState == State::SECONDARY_ROLLED && "Both rolls should be made before attempting to retrive initiative result");
            return mSecondaryRoll? upgradedRoll(): mPrimaryRoll;

        case GamePhase::PLAY:
            assert(
                (mState != State::UNROLLED)
                && "At least one of the two dice should have been rolled during the play phase"
            );
            if(mState == State::SECONDARY_ROLLED) {
                return mSecondaryRoll? upgradedRoll(): 0;
            }
            return mPrimaryRoll;

        case GamePhase::END:
            assert(false && "Dice rolls are invalid for the last game phase");
    }

    return 0;
}

void Dice::roll() {
    switch(mState) {
        case State::UNROLLED:
            mPrimaryRoll = mPrimaryDieDistribution(mRandomEngine);
            break;
        case State::PRIMARY_ROLLED:
            mSecondaryRoll = mYesNoDieDistribution(mRandomEngine);
            break;
        case State::SECONDARY_ROLLED:
            assert(false && "No roll is possible after secondary");
    }
}

uint8_t Dice::upgradedRoll() const {
    if(mPrimaryRoll == 4) return 10;
    return mPrimaryRoll + 4;
}
