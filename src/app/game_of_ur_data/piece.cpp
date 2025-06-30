#include "piece_type.hpp"
#include "piece.hpp"

bool Piece::canMove(uint8_t roll, RoleID player) const {
    return ( 
        !(mState == State::FINISHED) || (
            (
                (mState == State::ON_BOARD) || (
                    mState == State::UNLAUNCHED 
                    && roll == kGamePieceTypes[mIdentity.mType].mLaunchRoll
                )
            ) && (
                mIdentity.mOwner == player
            )
        )
    );
}

bool operator<(const PieceIdentity& one, const PieceIdentity& two) {
    assert(one.mOwner != RoleID::NA && two.mOwner != RoleID::NA && "The owner of a piece cannot be NA");
    return (
        static_cast<uint8_t>(one.mOwner) < static_cast<uint8_t>(two.mOwner)
        || (
            static_cast<uint8_t>(one.mOwner) == static_cast<uint8_t>(two.mOwner)
            && static_cast<uint8_t>(one.mType) < static_cast<uint8_t>(two.mType)
        )
    );
}
