#include "game_piece_type.hpp"
#include "board.hpp"

std::weak_ptr<GamePiece> Board::move(PlayerRoleID player, std::weak_ptr<GamePiece> gamePiece, glm::u8vec2 toLocation, uint8_t roll) {
    assert(!gamePiece.expired() && "game piece pointer must point to an existing object");
    assert(canMove(player, *gamePiece.lock(), toLocation, roll) && "this move is invalid and should not have been attempted");

    std::shared_ptr<GamePiece> lockedGamePiece { gamePiece.lock() };

    if(lockedGamePiece->getState() == GamePiece::State::ON_BOARD) {
        const glm::u8vec2 pieceLocation { lockedGamePiece->getLocation() };
        mGrid[pieceLocation.x][pieceLocation.y].move(nullptr);
    }

    lockedGamePiece->setLocation(toLocation);

    if(isValidHouse(toLocation)) {
        std::weak_ptr<GamePiece> knockedOut { mGrid[toLocation.x][toLocation.y].getOccupantReference() };
        if(std::shared_ptr opponentPiece = knockedOut.lock()) {
            opponentPiece->setLocation(glm::u8vec2{0,0});
        }
        mGrid[toLocation.x][toLocation.y].move(lockedGamePiece);
    }

    // otherwise, this piece has won
    return std::weak_ptr<GamePiece>{};
}

std::weak_ptr<GamePiece> Board::getOccupantReference(glm::u8vec2 location) {
    assert(isValidHouse(location) && "no house exists at the specified location");
    return mGrid[location.x][location.y].getOccupantReference();
}

bool Board::isValidHouse(glm::u8vec2 location) const {
    return (
        location.x < mGrid.size() 
        && location.y < mGrid[location.x].size()
    );
}
bool Board::isRosette(glm::u8vec2 location) const {
    return isValidHouse(location) && mGrid[location.x][location.y].isRosette();
}
bool Board::isRouteEnd(glm::u8vec2 location) const {
    return location.x == 1 && location.y == mGrid[1].size();
}

bool Board::canMove(PlayerRoleID role, const GamePiece& gamePiece, glm::u8vec2 toLocation, uint8_t roll) const {
    // no roll means that no piece is being moved
    if(roll == 0) return false;

    const bool gamePieceCanMove { gamePiece.canMove(roll, role) };
    if(!gamePieceCanMove || !(isValidHouse(toLocation) || isRouteEnd(toLocation))) return false;

    const bool destinationAvailable {
        (
            isValidHouse(toLocation) && mGrid[toLocation.x][toLocation.y].canMove(gamePiece)
        ) || (isRouteEnd(toLocation))
    };

    const bool moveCorrespondsToDice {
        (
            gamePiece.getState()==GamePiece::State::ON_BOARD 
            && computeMoveLocation(gamePiece, roll) == toLocation
        ) || (
            gamePiece.getState()==GamePiece::State::UNLAUNCHED
            && isValidLaunchHouse(toLocation, gamePiece)
        )
    };

    return destinationAvailable && moveCorrespondsToDice;
}

bool Board::movePassesRosette(const GamePiece& gamePiece, glm::u8vec2 toLocation) const {
    if(gamePiece.getState() != GamePiece::State::ON_BOARD) return false;
    glm::u8vec2 currentLocation { gamePiece.getLocation() };
    assert(isValidHouse(currentLocation));

    currentLocation += mGrid[currentLocation.x][currentLocation.y].getNextCellDirection();
    while(isValidHouse(currentLocation)) {
        if(
            currentLocation != toLocation &&
            mGrid[currentLocation.x][currentLocation.y].isRosette()
        ) return true;

        currentLocation += mGrid[currentLocation.x][currentLocation.y].getNextCellDirection();
    }
    return false;
}

bool Board::moveDisplacesOpponent(PlayerRoleID role, const GamePiece& gamePiece, glm::u8vec2 toLocation, uint8_t roll) const {
    return (
        canMove(role, gamePiece, toLocation, roll) 
        && isValidHouse(toLocation)
        && mGrid[toLocation.x][toLocation.y].isOccupied()
    );
}

glm::u8vec2 Board::computeMoveLocation(const GamePiece& gamePiece, uint8_t roll) const {
    assert(gamePiece.getState() == GamePiece::State::ON_BOARD && "moves may only be computed for pieces that are on the board");

    glm::u8vec2 currentLocation { gamePiece.getLocation() };
    for(
        // counter
        House house{ mGrid[currentLocation.x][currentLocation.y] }; 

        // condition
        roll > 0 && currentLocation.x < mGrid.size() && currentLocation.y < mGrid[currentLocation.x].size();

        // step
        roll--, currentLocation += house.getNextCellDirection()
    );

    // If it's a move that takes the piece off the board, but not exactly, the move
    // fails, and we return the piece's current location
    if(roll) return gamePiece.getLocation();

    // otherwise return the computed location
    return currentLocation;
}

bool Board::isValidLaunchHouse(glm::u8vec2 location, const GamePiece& gamePiece) const {
    const GamePieceType::LaunchType launchType { kGamePieceTypes[gamePiece.getType()].mLaunchType };
    return (
        (
            launchType == GamePieceType::ONE_BEFORE_ROSETTE
            && (
                location.x < mGrid.size()
                && (
                    (
                        location.x == 1 
                        && location.y < mGrid[1].size() 
                        && location.y % 4 == 3
                    ) || (
                        location.y == 1
                        && mGrid[location.x][0].getRegion() == static_cast<House::Region>(gamePiece.getOwner())
                    )
                )
            )
        ) || (
            location.x == 1 && location.y == (static_cast<uint8_t>(launchType) - 1)
        )
    );
}

bool Board::houseIsOccupied(glm::u8vec2 location) const {
    return (
        isValidHouse(location)
        && mGrid[location.x][location.y].isOccupied()
    );
}

House::Type Board::getType(glm::u8vec2 location) const {
    assert(isValidHouse(location) && "This location does not correspond to a valid house on this board");
    return mGrid[location.x][location.y].getType();
}

House::Region Board::getRegion(glm::u8vec2 location) const {
    assert(isValidHouse(location) && "This location does not correspond to a valid house on this board");
    return mGrid[location.x][location.y].getRegion();
}

GamePieceIdentity Board::getOccupant(glm::u8vec2 location) const {
    assert(isValidHouse(location) && "This location does not correspond to a valid house on this board");
    return mGrid[location.x][location.y].getOccupant();
}

std::vector<glm::u8vec2> Board::getValidLaunchPositions(GamePieceIdentity pieceIdentity) const {
    assert(pieceIdentity.mOwner != PlayerRoleID::NA && "Each piece must have a corresponding player role");

    std::vector<glm::u8vec2> results {};
    switch(kGamePieceTypes[pieceIdentity.mType].mLaunchType) {
        case GamePieceType::ONE_BEFORE_ROSETTE:
            results.push_back({
                mGrid[0][0].getRegion() == static_cast<House::Region>(pieceIdentity.mOwner)? 0: 2,
                1
            });
            for(uint8_t y { 3 }; y < mGrid[1].size(); y+=4) {
                results.push_back({1, y});
            }
            break;

        default:
            results.push_back({1, static_cast<uint8_t>(kGamePieceTypes[pieceIdentity.mType].mLaunchType) - 1});
            break;
    };

    return results;
}

glm::i8vec2 Board::getNextCellDirection(glm::u8vec2 location) const {
    assert(isValidHouse(location) && "This location does not correspond to a valid house on this board");
    return mGrid[location.x][location.y].getNextCellDirection();
}
