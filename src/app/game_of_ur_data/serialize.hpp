#ifndef ZOAPPSERIALIZE_H
#define ZOAPPSERIALIZE_H

#include <nlohmann/json.hpp>

#include "piece.hpp"
#include "role_id.hpp"

NLOHMANN_JSON_SERIALIZE_ENUM(PieceTypeID, {
    {PieceTypeID::EAGLE, "eagle"},
    {PieceTypeID::ROOSTER, "rooster"},
    {PieceTypeID::RAVEN, "raven"},
    {PieceTypeID::STORMBIRD, "storm-bird"},
    {PieceTypeID::SWALLOW, "swallow"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(RoleID, {
    {RoleID::ONE, "player-one"},
    {RoleID::TWO, "player-two"},
});

void from_json(const nlohmann::json& json, PieceIdentity& pieceIdentity);
void to_json(nlohmann::json& json, const PieceIdentity& pieceIdentity);

#endif
