#include "game_piece.hpp"
#include "game_piece_type.hpp"

bool GamePiece::canMove(uint8_t roll, PlayerRoleID player) const {
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
