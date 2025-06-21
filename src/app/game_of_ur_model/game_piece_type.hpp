#ifndef ZOAPPGAMEPIECETYPE_H
#define ZOAPPGAMEPIECETYPE_H

#include <string>
#include <cstdint>
#include <array>

#include "game_piece_type_id.hpp"

struct GamePieceType {
    enum LaunchType {
        ONE_BEFORE_ROSETTE,
        FIVE=5,
        SIX=6,
        SEVEN=7,
        TEN=10,
    };

    std::string mName;
    uint8_t mLaunchRoll;
    LaunchType mLaunchType;
    uint8_t mCost;
};

inline const std::array<const GamePieceType, 5> kGamePieceTypes {{
    {.mName="swallow", .mLaunchRoll=2, .mLaunchType=GamePieceType::ONE_BEFORE_ROSETTE, .mCost=3},
    {.mName="storm-bird", .mLaunchRoll=5,  .mLaunchType=GamePieceType::FIVE, .mCost=4},
    {.mName="raven", .mLaunchRoll=6, .mLaunchType=GamePieceType::SIX, .mCost=4},
    {.mName="rooster", .mLaunchRoll=7, .mLaunchType=GamePieceType::SEVEN, .mCost=4},
    {.mName="eagle", .mLaunchRoll=10, .mLaunchType=GamePieceType::TEN, .mCost=5},
}};

#endif
