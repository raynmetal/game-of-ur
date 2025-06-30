#ifndef ZOAPPGAMEPIECETYPE_H
#define ZOAPPGAMEPIECETYPE_H

#include <string>
#include <cstdint>
#include <array>

#include "piece_type_id.hpp"

struct PieceType {
    enum LaunchType {
        ONE_BEFORE_ROSETTE,
        SAME_AS_LAUNCH_ROLL,
    };

    std::string mName;
    uint8_t mLaunchRoll;
    LaunchType mLaunchType;
    uint8_t mCost;
};

inline const std::array<const PieceType, 5> kGamePieceTypes {{
    {.mName="swallow", .mLaunchRoll=2, .mLaunchType=PieceType::LaunchType::ONE_BEFORE_ROSETTE, .mCost=3},
    {.mName="storm-bird", .mLaunchRoll=5,  .mLaunchType=PieceType::LaunchType::SAME_AS_LAUNCH_ROLL, .mCost=4},
    {.mName="raven", .mLaunchRoll=6, .mLaunchType=PieceType::LaunchType::SAME_AS_LAUNCH_ROLL, .mCost=4},
    {.mName="rooster", .mLaunchRoll=7, .mLaunchType=PieceType::LaunchType::SAME_AS_LAUNCH_ROLL, .mCost=4},
    {.mName="eagle", .mLaunchRoll=10, .mLaunchType=PieceType::LaunchType::SAME_AS_LAUNCH_ROLL, .mCost=5},
}};

#endif
