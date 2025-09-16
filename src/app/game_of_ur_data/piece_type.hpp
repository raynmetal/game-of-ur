/**
 * @ingroup UrGameDataModel
 * @file game_of_ur_data/piece_type.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains structs and consts that describe the pieces in the game and their capabilities.
 * @version 0.3.2
 * @date 2025-09-12
 * 
 * 
 */

#ifndef ZOAPPGAMEPIECETYPE_H
#define ZOAPPGAMEPIECETYPE_H

#include <string>
#include <cstdint>
#include <array>

#include "piece_type_id.hpp"

/**
 * @ingroup UrGameDataModel
 * @brief A struct containing information about a piece, including its name and information.
 * 
 */
struct PieceType {
    /**
     * @brief The way a piece of some type can be launched.
     * 
     */
    enum LaunchType {
        ONE_BEFORE_ROSETTE, //< Applicable only to the swallow piece; can be launched to a house one location before a rosette.
        SAME_AS_LAUNCH_ROLL, //< Applicable to most pieces; launch house must be (along the route) ordinally equal to the launch roll.
    };

    /**
     * @brief The name of this type of piece, for display.
     * 
     */
    std::string mName;

    /**
     * @brief The roll required in order to launch this type of piece.
     * 
     */
    uint8_t mLaunchRoll;

    /**
     * @brief The type of launch action applicable to this piece.
     * 
     */
    LaunchType mLaunchType;

    /**
     * @brief The number of counters won or lost depending on whether or not this piece succeeds in landing on a rosette house.
     * 
     */
    uint8_t mCost;
};

/**
 * @ingroup UrGameDataModel
 * @brief An array of PieceTypes, each element describing a single type of piece used in the game.
 * 
 */
inline const std::array<const PieceType, 5> kGamePieceTypes {{
    {.mName="swallow", .mLaunchRoll=2, .mLaunchType=PieceType::LaunchType::ONE_BEFORE_ROSETTE, .mCost=3},
    {.mName="storm-bird", .mLaunchRoll=5,  .mLaunchType=PieceType::LaunchType::SAME_AS_LAUNCH_ROLL, .mCost=4},
    {.mName="raven", .mLaunchRoll=6, .mLaunchType=PieceType::LaunchType::SAME_AS_LAUNCH_ROLL, .mCost=4},
    {.mName="rooster", .mLaunchRoll=7, .mLaunchType=PieceType::LaunchType::SAME_AS_LAUNCH_ROLL, .mCost=4},
    {.mName="eagle", .mLaunchRoll=10, .mLaunchType=PieceType::LaunchType::SAME_AS_LAUNCH_ROLL, .mCost=5},
}};

#endif
