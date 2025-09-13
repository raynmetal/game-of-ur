/**
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

NLOHMANN_JSON_SERIALIZE_ENUM(PieceTypeID, {
    {PieceTypeID::EAGLE, "eagle"},
    {PieceTypeID::ROOSTER, "rooster"},
    {PieceTypeID::RAVEN, "raven"},
    {PieceTypeID::STORMBIRD, "storm-bird"},
    {PieceTypeID::SWALLOW, "swallow"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(RoleID, {
    {RoleID::BLACK, "black"},
    {RoleID::WHITE, "white"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(PlayerID, {
    {PlayerID::PLAYER_A, "player-a"},
    {PlayerID::PLAYER_B, "player-b"},
});

void from_json(const nlohmann::json& json, PieceIdentity& pieceIdentity);
void to_json(nlohmann::json& json, const PieceIdentity& pieceIdentity);

void from_json(const nlohmann::json& json, GameScoreData& gameScoreData);
void to_json(nlohmann::json& json, const GameScoreData& gameScoreData);

void from_json(const nlohmann::json& json, PlayerData& playerData);
void to_json(nlohmann::json& json, const PlayerData& playerData);

#endif
