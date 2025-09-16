/**
 * @ingroup UrGameDataModel
 * @file game_of_ur_data/dice.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains the data model class representing the pair of dice used to play the Royal Game of Ur.
 * @version 0.3.2
 * @date 2025-09-12
 * 
 * 
 */

#ifndef ZOAPPDICE_H
#define ZOAPPDICE_H

#include <cstdint>
#include <random>

#include "phase.hpp"

/**
 * @ingroup UrGameDataModel
 * @brief The data model used to represent the pair of dice used to play the Royal Game of Ur.
 * 
 * Contains methods for querying the current state of the dice, and for performing dice rolls (when valid).
 * 
 */
class Dice {
public:
    /**
     * @brief Values representing all possible states for this pair of dice.
     * 
     */
    enum State {
        UNROLLED, //< The dice have not yet been rolled.
        PRIMARY_ROLLED, //< Only the first of the dice, the one producing its primary roll, has been rolled.
        SECONDARY_ROLLED, //< Both dice have been rolled (and no further rolls are permitted)
    };

    /**
     * @brief Resets the state of the dice to State::UNROLLED.
     * 
     */
    void reset();

    /**
     * @brief After validating that a roll is possible, rolls one of the dice.
     * 
     * If the current state is State::UNROLLED, the primary die is rolled.  If State::PRIMARY_ROLLED, the secondary die is rolled.
     * 
     */
    void roll();

    /**
     * @brief Tests whether rolling the dice is presently possible.
     * 
     * @retval true The dice can be rolled.
     * @retval false The dice can't be rolled.
     */
    inline bool canRoll() const { return mState != SECONDARY_ROLLED; }

    /**
     * @brief Gets the current state of the dice.
     * 
     * @return State The current state of the dice.
     */
    inline State getState() const { return mState; }

    /**
     * @brief Gets the value of the primary die, between 1 and 4.
     * 
     * @return uint8_t The value of the primary die, between 1 and 4.
     * 
     * @see mPrimaryRoll
     * 
     */
    inline uint8_t getPrimaryRoll() const { return mPrimaryRoll; }

    /**
     * @brief Gets the value of the secondary die, either Double or Quits.
     * 
     * @retval true The secondary die says Double.
     * @retval false The secondary die says Quits.
     * 
     * @see mSecondaryRoll
     * 
     */
    inline bool getSecondaryRoll() const { return mSecondaryRoll; }

    /**
     * @brief Gets the score represented by both dice combined for the current phase of the game.
     * 
     * The table below shows the results returned based on the state of the dice and that of the game:
     * 
     * | Primary Roll  | Secondary Roll | INITIATIVE | PLAY   |
     * | ------------: | :------------- | ---------: | -----: |
     * | -             | -              | 0          | 0      |
     * | 1             | -              | 0          | 1      |
     * | 2             | -              | 0          | 2      |
     * | 3             | -              | 0          | 3      |
     * | 4             | -              | 0          | 4      |
     * | 1             | Quits          | 1          | 0      |
     * | 2             | Quits          | 2          | 0      |
     * | 3             | Quits          | 3          | 0      |
     * | 4             | Quits          | 4          | 0      |
     * | 1             | Double         | 5          | 5      |
     * | 2             | Double         | 6          | 6      |
     * | 3             | Double         | 7          | 7      |
     * | 4             | Double         | 10         | 10     |
     * 
     * GamePhase::END always produces a score of 0.
     * 
     * @param currentPhase The current phase of the game.
     * @return uint8_t The dice score based on game and dice state.
     */
    uint8_t getResult(GamePhase currentPhase) const;

private:
    /**
     * @brief Maps the score of the primary die to its result score if one rolled Double with the secondary die.
     * 
     * @return uint8_t The upgraded value of the current primary die roll.
     * 
     * @see getResult()
     */
    uint8_t upgradedRoll() const;

    /**
     * @brief The score on the primary die, a value between 1 and 4.
     * 
     */
    uint8_t mPrimaryRoll { 1 };

    /**
     * @brief The score on the secondary die, either Double or Quits, where true -> Double, false -> Quits.
     * 
     */
    bool mSecondaryRoll { false };

    /**
     * @brief The current state of the dice.
     * 
     */
    State mState { UNROLLED };

    /**
     * @brief The device used as the source of random values.
     * 
     */
    std::random_device mRandomDevice {};

    /**
     * @brief The engine used to produce random values (I don't really know).
     * 
     */
    std::default_random_engine mRandomEngine { mRandomDevice() };

    /**
     * @brief A meaningful range of integer values used by the primary die, where each value has an even chance of being drawn.
     * 
     */
    std::uniform_int_distribution<int> mPrimaryDieDistribution { 1, 4 };

    /**
     * @brief The range of integer values possible for the secondary die, where each integer has an even chance of being drawn.
     * 
     */
    std::uniform_int_distribution<int> mYesNoDieDistribution { 0, 1 };
};


#endif
