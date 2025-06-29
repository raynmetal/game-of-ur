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

std::weak_ptr<Piece> Player::getPiece(PieceTypeID pieceType) {
    assert(pieceType <= PieceTypeID::TOTAL && "pieceType must be a valid member of the enum PieceTypeID");
    return mPieces[pieceType];
}

const Piece& Player::cGetPiece(PieceTypeID pieceType) const {
    assert(pieceType <= PieceTypeID::TOTAL && "pieceType must be a valid member of the enum PieceTypeID");
    assert(mRole != RoleID::NA && "This player has not been assigned a role");
    return *mPieces[pieceType];
}

void Player::initializeWithRole(RoleID role) {
    mRole = role;
    switch(role) {
        case RoleID::ONE:
        case RoleID::TWO:
            for(uint8_t type { 0 }; type < PieceTypeID::TOTAL; ++type) {
                mPieces[type] = std::make_shared<Piece>(static_cast<PieceTypeID>(type), mRole);
            }
            break;

        case RoleID::NA:
            assert(false && "Cannot initialize player with non-existent role");
    }
}

uint8_t Player::getNPieces(Piece::State inState) const {
    if(mRole == RoleID::NA) { return 0; }
    uint8_t count { 0 };
    for(uint8_t i{0}; i < mPieces.size(); ++i) {
        if(mPieces[i]->getState() == inState) {
            ++count;
        }
    }
    return count;
}
