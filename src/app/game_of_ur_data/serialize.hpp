/**
 * @ingroup UrGameDataModel
 * @file game_of_ur_data/serialize.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains methods to convert data from its-in game representation to its JSON representation, and vice versa.
 * @version 0.3.2
 * @date 2025-09-12
 * 
 * 
 */

#ifndef ZOAPPSERIALIZE_H
#define ZOAPPSERIALIZE_H

#include <nlohmann/json.hpp>

#include "model.hpp"

/** @ingroup UrGameDataModel */
NLOHMANN_JSON_SERIALIZE_ENUM(PieceTypeID, {
    {PieceTypeID::EAGLE, "eagle"},
    {PieceTypeID::ROOSTER, "rooster"},
    {PieceTypeID::RAVEN, "raven"},
    {PieceTypeID::STORMBIRD, "storm-bird"},
    {PieceTypeID::SWALLOW, "swallow"},
});

/** @ingroup UrGameDataModel */
NLOHMANN_JSON_SERIALIZE_ENUM(RoleID, {
    {RoleID::BLACK, "black"},
    {RoleID::WHITE, "white"},
});

/** @ingroup UrGameDataModel */
NLOHMANN_JSON_SERIALIZE_ENUM(PlayerID, {
    {PlayerID::PLAYER_A, "player-a"},
    {PlayerID::PLAYER_B, "player-b"},
});

/** @ingroup UrGameDataModel */
void from_json(const nlohmann::json& json, PieceIdentity& pieceIdentity);
/** @ingroup UrGameDataModel */
void to_json(nlohmann::json& json, const PieceIdentity& pieceIdentity);

/** @ingroup UrGameDataModel */
void from_json(const nlohmann::json& json, GameScoreData& gameScoreData);
/** @ingroup UrGameDataModel */
void to_json(nlohmann::json& json, const GameScoreData& gameScoreData);

/** @ingroup UrGameDataModel */
void from_json(const nlohmann::json& json, PlayerData& playerData);
/** @ingroup UrGameDataModel */
void to_json(nlohmann::json& json, const PlayerData& playerData);

#endif
