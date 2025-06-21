#include "player.hpp"

void Player::depositCounters(uint8_t counters) {
    mCounters += counters;
}

uint8_t Player::deductCounters(uint8_t counters) {
    if(counters > mCounters) {
        counters = mCounters;
    }
    mCounters -= counters;
    return counters;
}

std::weak_ptr<GamePiece> Player::getPiece(GamePieceTypeID pieceType) {
    assert(pieceType <= GamePieceTypeID::TOTAL && "pieceType must be a valid member of the enum GamePieceTypeID");
    return mPieces[pieceType];
}

const GamePiece& Player::cGetPiece(GamePieceTypeID pieceType) const {
    assert(pieceType <= GamePieceTypeID::TOTAL && "pieceType must be a valid member of the enum GamePieceTypeID");
    assert(mRole != PlayerRoleID::NA && "This player has not been assigned a role");
    return *mPieces[pieceType];
}

void Player::initializeRole(PlayerRoleID role) {
    mRole = role;
    switch(role) {
        case PlayerRoleID::ONE:
        case PlayerRoleID::TWO:
            for(uint8_t type { 0 }; type < GamePieceTypeID::TOTAL; ++type) {
                mPieces[type] = std::make_shared<GamePiece>(static_cast<GamePieceTypeID>(type), mRole);
            }
            break;

        case PlayerRoleID::NA:
            assert(false && "Cannot initialize player with non-existent role");
    }
}

uint8_t Player::getNPieces(GamePiece::State inState) const {
    if(mRole == PlayerRoleID::NA) { return 0; }
    uint8_t count { 0 };
    for(uint8_t i{0}; i < mPieces.size(); ++i) {
        count += mPieces[i]->getState() == inState? 1: 0;
    }
    return count;
}
