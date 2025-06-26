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
