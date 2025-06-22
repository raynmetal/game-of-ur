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
