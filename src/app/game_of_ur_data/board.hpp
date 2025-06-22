#ifndef ZOAPPBOARD_H
#define ZOAPPBOARD_H

#include <vector>
#include <array>

#include <glm/glm.hpp>

#include "house.hpp"

class Board {
public:
    // validates and applies the move, returning opponent piece that was knocked out, if any
    std::weak_ptr<Piece> move(RoleID role, std::weak_ptr<Piece> gamePiece, glm::u8vec2 toLocation, uint8_t roll);
    std::weak_ptr<Piece> getOccupantReference(glm::u8vec2 location);

    bool canMove(RoleID role, const Piece& gamePiece, glm::u8vec2 toLocation, uint8_t roll) const;
    bool movePassesRosette(const Piece& gamePiece, glm::u8vec2 toLocation) const;
    bool moveDisplacesOpponent(RoleID role, const Piece& gamePiece, glm::u8vec2 toLocation, uint8_t roll) const;

    House::Type getType(glm::u8vec2 location) const;
    House::Region getRegion(glm::u8vec2 location) const;
    PieceIdentity getOccupant(glm::u8vec2 location) const;
    glm::i8vec2 getNextCellDirection(glm::u8vec2 location) const;
    std::vector<glm::u8vec2> getValidLaunchPositions(PieceIdentity pieceIdentity) const;

    bool houseIsOccupied(glm::u8vec2 location) const;
    bool isValidHouse(glm::u8vec2 location) const;
    bool isValidLaunchHouse(glm::u8vec2 location, const Piece& gamePiece) const;
    bool isRosette(glm::u8vec2 location) const;
    bool isRouteEnd(glm::u8vec2 location) const;

    glm::u8vec2 computeMoveLocation(const Piece& gamePiece, uint8_t roll) const;
private:
    std::array<std::vector<House>, 3> mGrid {{
        {
            House{{1, 0}, House::ROSETTE, House::ONE},
            House{{0, -1}, House::REGULAR, House::ONE},
            House{{0, -1}, House::REGULAR, House::ONE},
            House{{0, -1}, House::REGULAR, House::ONE},
        },
        {
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::ROSETTE, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::ROSETTE, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::ROSETTE, House::BATTLEFIELD},
        },
        {
            House{{-1, 0}, House::ROSETTE, House::TWO},
            House{{0, -1}, House::REGULAR, House::TWO},
            House{{0, -1}, House::REGULAR, House::TWO},
            House{{0, -1}, House::REGULAR, House::TWO},
        },
    }};
};

#endif
