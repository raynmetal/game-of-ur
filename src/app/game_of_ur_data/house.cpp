#include "house.hpp"

bool House::canMove(const Piece& gamePiece) const {
    // to be able to move to this cell, this cell ...
    return (
        // ... must be region accessible to this player, ...
        (
            mRegion == Region::BATTLEFIELD
            || static_cast<RoleID>(mRegion) == gamePiece.getOwner()
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

void House::move(std::shared_ptr<Piece> gamePiece) {
    if(!gamePiece) {
        mOccupant.reset();
    }

    assert(canMove(*gamePiece) && "This piece cannot move to this location");

    mOccupant = gamePiece;
}

PieceIdentity House::getOccupant() const {
    if(mOccupant.expired()) {
        return PieceIdentity {
            .mOwner { RoleID::NA },
        };
    }

    return mOccupant.lock()->getIdentity();
}
