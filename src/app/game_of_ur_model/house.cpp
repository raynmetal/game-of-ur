#include "house.hpp"

bool House::canMove(const GamePiece& gamePiece) const {
    // to be able to move to this cell, this cell ...
    return (
        // ... must be region accessible to this player, ...
        (
            mRegion == Region::BATTLEFIELD
            || mRegion == gamePiece.getOwner()
        )

        // ... and ... 
        && (
            // ... must be unoccupied ...
            mOccupant.expired()

            // ... or if occupied, not ...
            || !(
                // ... be a rosette ...
                mType == Type::ROSETTE
                // ... nor be occupied by another of our own pieces
                || (gamePiece.getOwner() == mOccupant.lock()->getOwner())
            )
        )
    );
}

void House::move(std::shared_ptr<GamePiece> gamePiece) {
    if(!gamePiece) {
        mOccupant.reset();
    }

    assert(canMove(*gamePiece) && "This piece cannot move to this location");

    mOccupant = gamePiece;
}

GamePieceIdentity House::getOccupant() const {
    if(mOccupant.expired()) {
        return GamePieceIdentity {
            .mOwner { PlayerRoleID::NA },
        };
    }

    mOccupant.lock()->getIdentity();
}
