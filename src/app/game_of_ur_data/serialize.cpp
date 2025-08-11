#include "serialize.hpp"

void from_json(const nlohmann::json& json, PieceIdentity& pieceIdentity) {
    json.at("owner").get_to(pieceIdentity.mOwner);
    json.at("type").get_to(pieceIdentity.mType);
}
void to_json(nlohmann::json& json, const PieceIdentity& pieceIdentity) {
    json = {
        {"owner", pieceIdentity.mOwner},
        {"type", pieceIdentity.mType},
    };
}

void from_json(const nlohmann::json& json, GameScoreData& gameScoreData) {
    json.at("common_counters").get_to(gameScoreData.mCommonPoolCounters);
    json.at("player_one_counters").get_to(gameScoreData.mPlayerOneCounters);
    json.at("player_two_counters").get_to(gameScoreData.mPlayerTwoCounters);
    json.at("player_one_victory_pieces").get_to(gameScoreData.mPlayerOneVictoryPieces);
    json.at("player_two_victory_pieces").get_to(gameScoreData.mPlayerTwoVictoryPieces);
}
void to_json(nlohmann::json& json, const GameScoreData& gameScoreData) {
    json = {
        {"common_counters", static_cast<int>(gameScoreData.mCommonPoolCounters)},
        {"player_one_counters", static_cast<int>(gameScoreData.mPlayerOneCounters)},
        {"player_two_counters", static_cast<int>(gameScoreData.mPlayerTwoCounters)},
        {"player_one_victory_pieces", static_cast<int>(gameScoreData.mPlayerOneVictoryPieces)},
        {"player_two_victory_pieces", static_cast<int>(gameScoreData.mPlayerTwoVictoryPieces)},
    };
}

void from_json(const nlohmann::json& json, PlayerData& playerData) {
    json.at("counters").get_to(playerData.mCounters);
    json.at("is_winner").get_to(playerData.mIsWinner);
    json.at("on_board_pieces").get_to(playerData.mNBoardPieces);
    json.at("victory_pieces").get_to(playerData.mNVictoryPieces);
    json.at("unlaunched_pieces").get_to(playerData.mNUnlaunchedPieces);
    json.at("role").get_to(playerData.mRole);
    json.at("player").get_to(playerData.mPlayer);
}
void to_json(nlohmann::json& json, const PlayerData& playerData) {
    json = {
        { "counters", static_cast<int>(playerData.mCounters) },
        { "is_winner", playerData.mIsWinner},
        { "on_board_pieces", playerData.mNBoardPieces},
        { "victory_pieces", playerData.mNVictoryPieces },
        { "unlaunched_pieces", playerData.mNUnlaunchedPieces },
        { "role", playerData.mRole },
        { "player", playerData.mPlayer },
    };
}
